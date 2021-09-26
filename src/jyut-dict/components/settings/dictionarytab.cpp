#include "dictionarytab.h"

#include "components/dictionarylist/dictionarylistview.h"
#include "logic/dictionary/dictionarysource.h"
#include "logic/utils/utils.h"
#include "logic/settings/settingsutils.h"
#ifdef Q_OS_MAC
#include "logic/utils/utils_mac.h"
#endif

#include <QtConcurrent/QtConcurrent>
#include "QCoreApplication"
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QtSql>

DictionaryTab::DictionaryTab(std::shared_ptr<SQLDatabaseManager> manager,
                             QWidget *parent)
    : QWidget{parent},
    _manager{manager}
{
    _utils = std::make_unique<SQLDatabaseUtils>(_manager);

    setupUI();
    translateUI();
    populateDictionaryList();
    _list->setCurrentIndex(_list->model()->index(0, 0));
}

void DictionaryTab::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateUI();
        _list->setCurrentIndex(_list->model()->index(0, 0));
    }
}

void DictionaryTab::setupUI()
{
#ifdef Q_OS_WIN
    if (QLocale::system().language() & QLocale::Chinese ||
        QLocale::system().language() & QLocale::Cantonese) {
        setStyleSheet("QLabel { font-size: 12px; }");
    }
#endif
    _tabLayout = new QGridLayout{this};
    _tabLayout->setAlignment(Qt::AlignTop);

    _explanatory = new QLabel{this};
    _explanatory->setWordWrap(true);
    _explanatory->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    _explanatory->setStyleSheet("QLabel { margin: 5px 0 5px 0; }");
#endif
    _list = new DictionaryListView{this};
    _list->setFixedWidth(200);
    _add = new QPushButton{this};
    _groupbox = new QGroupBox{this};
    _groupbox->setMinimumWidth(350);

    _tabLayout->addWidget(_explanatory, 1, 1, 1, -1);
    _tabLayout->addWidget(_list, 2, 1, 1, 1);
    _tabLayout->addWidget(_groupbox, 2, 2, 1, 1);
    _tabLayout->addWidget(_add, 3, 1, 1, 1);

    setLayout(_tabLayout);

    _description = new QLabel{this};
    _description->setWordWrap(true);
    _description->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _description->setAlignment(Qt::AlignTop);
    _legal = new QLabel{this};
    _legal->setWordWrap(true);
    _legal->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _legal->setAlignment(Qt::AlignTop);
    _version = new QLabel{this};
    _version->setWordWrap(true);
    _legal->setAlignment(Qt::AlignTop);
    _version->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _remove = new QPushButton{this};
    _link = new QPushButton{this};

    _groupboxLayout = new QGridLayout{_groupbox};
    _groupboxLayout->addWidget(_description, 1, 1, 4, 3);
    _groupboxLayout->addWidget(_legal, 5, 1, 2, 3);
    _groupboxLayout->addWidget(_version, 7, 1, 1, 3);
    _groupboxLayout->setRowStretch(8, INT_MAX);
    _groupboxLayout->addWidget(_remove, 9, 3, 1, 1);
    _groupboxLayout->addWidget(_link, 9, 2, 1, 1);

    _groupbox->setLayout(_groupboxLayout);

    connect(_list->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &DictionaryTab::setDictionaryMetadata);

    connect(_add, &QPushButton::clicked, this, [=] {
        QFileDialog *_fileDialog = new QFileDialog{this};
        _fileDialog->setFileMode(QFileDialog::ExistingFile);
        _fileDialog->setAcceptMode(QFileDialog::AcceptOpen);

        QString fileName = _fileDialog
                               ->getOpenFileName(this,
                                                 tr("Select dictionary file"),
                                                 QDir::homePath(),
                                                 "Dictionary Files (*.db)");
        if (!fileName.toStdString().empty()) {
            addDictionary(fileName);
        }
    });

#ifdef Q_OS_MAC
    // Set the style to match whether the user started dark mode
    setStyle(Utils::isDarkMode());
#else
    setStyle(false);
#endif
}

void DictionaryTab::translateUI()
{
    // Set property so styling automatically changes
    setProperty("isHan", Settings::isCurrentLocaleHan());

    QList<QPushButton *> buttons = this->findChildren<QPushButton *>();
    for (auto button : buttons) {
        button->setProperty("isHan", Settings::isCurrentLocaleHan());
        button->style()->unpolish(button);
        button->style()->polish(button);
    }

    _explanatory->setText(tr("The Dictionaries tab allows you to view "
                             "information about dictionaries, "
                             "and add or remove them."));
    _add->setText(tr("Add Dictionary..."));
    _remove->setText(tr("Delete Dictionary"));
    _link->setText(tr("Website"));
}

void DictionaryTab::setStyle(bool use_dark) {
    (void) (use_dark);
#ifdef Q_OS_MAC
    setStyleSheet("QPushButton[isHan=\"true\"] { font-size: 12px; height: 16px; }");
#elif defined(Q_OS_WIN)
    setStyleSheet("QPushButton[isHan=\"true\"] { font-size: 12px; height: 20px; }");
#endif
}

void DictionaryTab::setDictionaryMetadata(const QModelIndex &index)
{
    DictionaryMetadata metadata = qvariant_cast<DictionaryMetadata>(
        index.data());
    _description->setText(metadata.getDescription().c_str());
    _legal->setText(metadata.getLegal().c_str());
    _version->setText((tr("Version: %1")).arg(metadata.getVersion().c_str()));
#ifdef Q_OS_LINUX
    _groupbox->setTitle(tr("About %1").arg(metadata.getName().c_str()));
#endif

    disconnect(_link, nullptr, nullptr, nullptr);
    connect(_link, &QPushButton::clicked, this, [=] {
        QDesktopServices::openUrl(QUrl{metadata.getLink().c_str()});
    });

    _remove->setEnabled(_list->model()->rowCount() > 1);
    disconnect(_remove, nullptr, nullptr, nullptr);
    connect(_remove, &QPushButton::clicked, this, [=] {
        removeDictionary(metadata);
    });
}

void DictionaryTab::clearDictionaryList()
{
    _list->model()->removeRows(0, _list->model()->rowCount());
}

void DictionaryTab::populateDictionaryList()
{
    std::vector<DictionaryMetadata> sources;
    _utils->readSources(sources);

    for (std::vector<DictionaryMetadata>::size_type row = 0;
         row < sources.size();
         row++) {
        _list->model()->setData(_list->model()->index(static_cast<int>(row), 0),
                                QVariant::fromValue(sources.at(row)));
    }
}

void DictionaryTab::addDictionary(QString &dictionaryFile)
{
    QtConcurrent::run(_utils.get(),
                      &SQLDatabaseUtils::addSource,
                      dictionaryFile.toStdString(),
                      /* overwriteConflictingDictionaries */ false);

    _dialog = new QProgressDialog{"", QString(), 0, 0, this};
    _dialog->setWindowModality(Qt::ApplicationModal);
    _dialog->setMinimumSize(300, 75);
    Qt::WindowFlags flags = _dialog->windowFlags() | Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint
               | Qt::WindowFullscreenButtonHint | Qt::WindowContextHelpButtonHint);
    _dialog->setWindowFlags(flags);
    _dialog->setMinimumDuration(0);
#ifdef Q_OS_WIN
    _dialog->setWindowTitle(
        QCoreApplication::translate(Strings::STRINGS_CONTEXT, Strings::PRODUCT_NAME));
#elif defined(Q_OS_LINUX)
    _dialog->setWindowTitle(" ");
#endif
    _dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    _dialog->setLabelText(tr("Dropping search indexes..."));
    _dialog->setRange(0, 0);
    _dialog->setValue(0);

    disconnect(_utils.get(), nullptr, nullptr, nullptr);
    connect(_utils.get(), &SQLDatabaseUtils::insertingSource, this, [&] {
        _dialog->setLabelText(tr("Adding source..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::insertingEntries, this, [&] {
        _dialog->setLabelText(tr("Adding new entries..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::insertingDefinitions, this, [&] {
        _dialog->setLabelText(tr("Adding new definitions..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::rebuildingIndexes, this, [&] {
        _dialog->setLabelText(tr("Rebuilding search indexes..."));
    });

    connect(_utils.get(),
            &SQLDatabaseUtils::conflictingDictionaryNamesExist,
            this,
            [&](conflictingDictionaryMetadata dictionaries) {
                _dialog->reset();

                _overwriteDialog
                    = new OverwriteConflictingDictionaryDialog(dictionaries,
                                                               this);
                if (_overwriteDialog->exec() == QMessageBox::Yes) {
                    forceAddDictionary(dictionaryFile);
                }
            });

    connect(_utils.get(),
            &SQLDatabaseUtils::finishedAddition,
            this,
            [&](bool success, QString reason, QString description) {
                _dialog->reset();
                clearDictionaryList();
                populateDictionaryList();
                populateDictionarySourceUtils();
                _list->setCurrentIndex(_list->model()->index(0, 0));

                if (!success) {
                    failureMessage(reason, description);
                }
            });
}

void DictionaryTab::forceAddDictionary(QString &dictionaryFile)
{
    QtConcurrent::run(_utils.get(),
                      &SQLDatabaseUtils::addSource,
                      dictionaryFile.toStdString(),
                      /* overwriteConflictingDictionaries */ true);

    _dialog = new QProgressDialog{"", QString(), 0, 0, this};
    _dialog->setWindowModality(Qt::ApplicationModal);
    _dialog->setMinimumSize(300, 75);
    Qt::WindowFlags flags = _dialog->windowFlags() | Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint
               | Qt::WindowFullscreenButtonHint | Qt::WindowContextHelpButtonHint);
    _dialog->setWindowFlags(flags);
    _dialog->setMinimumDuration(0);
#ifdef Q_OS_WIN
    _dialog->setWindowTitle(
        QCoreApplication::translate(Strings::STRINGS_CONTEXT, Strings::PRODUCT_NAME));
#elif defined(Q_OS_LINUX)
    _dialog->setWindowTitle(" ");
#endif
    _dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    _dialog->setLabelText(tr("Dropping search indexes..."));
    _dialog->setRange(0, 0);
    _dialog->setValue(0);

    disconnect(_utils.get(), nullptr, nullptr, nullptr);

    connect(_utils.get(), &SQLDatabaseUtils::deletingDefinitions, this, [&] {
        _dialog->setLabelText(tr("Removing definitions..."));
    });

    connect(_utils.get(),
            &SQLDatabaseUtils::totalToDelete,
            this,
            [&](int numToDelete) {
                _dialog->setRange(0, numToDelete + 1);
                _dialog->setLabelText(
                    QString{tr("Deleted entry 0 of %1")}.arg(numToDelete));
            });

    connect(_utils.get(),
            &SQLDatabaseUtils::deletionProgress,
            this,
            [&](int deleted, int total) {
                _dialog->setLabelText(
                    QString{tr("Deleted entry %1 of %2")}.arg(deleted).arg(
                        total));
                _dialog->setValue(deleted);
            });

    connect(_utils.get(), &SQLDatabaseUtils::cleaningUp, this, [&] {
        _dialog->setLabelText(tr("Cleaning up..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::insertingSource, this, [&] {
        _dialog->setLabelText(tr("Adding source..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::insertingEntries, this, [&] {
        _dialog->setLabelText(tr("Adding new entries..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::insertingDefinitions, this, [&] {
        _dialog->setLabelText(tr("Adding new definitions..."));
    });

    connect(_utils.get(), &SQLDatabaseUtils::rebuildingIndexes, this, [&] {
        _dialog->setLabelText(tr("Rebuilding search indexes..."));
    });

    connect(_utils.get(),
            &SQLDatabaseUtils::finishedAddition,
            this,
            [&](bool success, QString reason, QString description) {
                _dialog->reset();
                clearDictionaryList();
                populateDictionaryList();
                populateDictionarySourceUtils();
                _list->setCurrentIndex(_list->model()->index(0, 0));

                if (!success) {
                    failureMessage(reason, description);
                }
            });
}

void DictionaryTab::removeDictionary(DictionaryMetadata metadata)
{
    QtConcurrent::run(_utils.get(),
                      &SQLDatabaseUtils::removeSource,
                      metadata.getName());

    _dialog = new QProgressDialog{"", QString(), 0, 0, this};
    _dialog->setWindowModality(Qt::ApplicationModal);
    _dialog->setMinimumSize(300, 75);
    Qt::WindowFlags flags = _dialog->windowFlags() | Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint
               | Qt::WindowFullscreenButtonHint | Qt::WindowContextHelpButtonHint);
    _dialog->setWindowFlags(flags);
    _dialog->setMinimumDuration(0);
#ifdef Q_OS_WIN
    _dialog->setWindowTitle(
        QCoreApplication::translate(Strings::STRINGS_CONTEXT, Strings::PRODUCT_NAME));
#elif defined(Q_OS_LINUX)
    _dialog->setWindowTitle(" ");
#endif
    _dialog->setAttribute(Qt::WA_DeleteOnClose, true);

    _dialog->setLabelText(tr("Removing source..."));
    _dialog->setRange(0, 0);
    _dialog->setValue(0);

    disconnect(_utils.get(), nullptr, nullptr, nullptr);
    connect(_utils.get(), &SQLDatabaseUtils::deletingDefinitions, this, [&] {
        _dialog->setLabelText(tr("Removing definitions..."));
    });

    connect(_utils.get(),
            &SQLDatabaseUtils::totalToDelete,
            this,
            [&](int numToDelete) {
                _dialog->setRange(0, numToDelete + 1);
                _dialog->setLabelText(
                    QString{tr("Deleted entry 0 of %1")}.arg(numToDelete));
            });

    connect(_utils.get(),
            &SQLDatabaseUtils::deletionProgress,
            this,
            [&](int deleted, int total) {
                _dialog->setLabelText(
                    QString{tr("Deleted entry %1 of %2")}.arg(deleted).arg(
                        total));
                _dialog->setValue(deleted);
            });

    connect(_utils.get(), &SQLDatabaseUtils::rebuildingIndexes, this, [&] {
        _dialog->setLabelText(tr("Rebuilding search indexes..."));
        _dialog->setRange(0, 0);
    });

    connect(_utils.get(), &SQLDatabaseUtils::cleaningUp, this, [&] {
        _dialog->setLabelText(tr("Cleaning up..."));
    });

    connect(_utils.get(),
            &SQLDatabaseUtils::finishedDeletion,
            this,
            [&](bool success) {
                _dialog->setLabelText(success ? tr("Done!") : tr("Failed!"));
                if (success) {
                    std::vector<std::pair<std::string, std::string>> sources;
                    _utils->readSources(sources);
                    for (auto source : sources) {
                        DictionarySourceUtils::addSource(source.first, source.second);
                    }
                }

                QTimer::singleShot(500, [&] {
                    _dialog->reset();
                    clearDictionaryList();
                    populateDictionaryList();
                    _list->setCurrentIndex(_list->model()->index(0, 0));
                });
            });
}

void DictionaryTab::populateDictionarySourceUtils()
{
    std::vector<std::pair<std::string, std::string>> sources;
    _utils->readSources(sources);
    for (auto source : sources) {
        DictionarySourceUtils::addSource(source.first,
                                         source.second);
    }
}

void DictionaryTab::failureMessage(QString reason, QString description)
{
    _message = new DictionaryTabFailureDialog{reason, description, this};
    _message->exec();
}
