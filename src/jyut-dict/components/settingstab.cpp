#include "settingstab.h"

#include "logic/entry/entry.h"
#include "logic/entry/entrycharactersoptions.h"
#include "logic/entry/entryphoneticoptions.h"
#include "logic/settings/settings.h"
#include "logic/settings/settingsutils.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QVariant>

SettingsTab::SettingsTab(QWidget *parent)
    : QWidget(parent)
{
    _settings = Settings::getSettings(this);
    setupUI();
}

void SettingsTab::setupUI()
{
    _tabLayout = new QFormLayout{this};
#ifdef Q_OS_WIN
    _tabLayout->setVerticalSpacing(15);
    _tabLayout->setContentsMargins(20, 20, 20, 20);
#elif defined(Q_OS_LINUX)
    _tabLayout->setVerticalSpacing(15);
    _tabLayout->setContentsMargins(20, 20, 20, 20);
    _tabLayout->setLabelAlignment(Qt::AlignRight);
#endif

    _characterCombobox = new QComboBox{this};
    initializeCharacterComboBox(*_characterCombobox);
    _phoneticCombobox = new QComboBox{this};
    initializePhoneticComboBox(*_phoneticCombobox);
    _mandarinCombobox = new QComboBox{this};
    initializeMandarinComboBox(*_mandarinCombobox);
    //    _languageCombobox = new QComboBox{this};

    _divider = new QFrame{this};
    _divider->setFrameShape(QFrame::HLine);
    _divider->setFrameShadow(QFrame::Raised);
    _divider->setFixedHeight(1);

    _colourCombobox = new QComboBox{this};
    initializeColourComboBox(*_colourCombobox);
    QWidget *jyutpingColourWidget = new QWidget{this};
    initializeJyutpingColourWidget(*jyutpingColourWidget);
    QWidget *pinyinColourWidget = new QWidget{this};
    initializePinyinColourWidget(*pinyinColourWidget);

    _tabLayout->addRow(tr("Simplified/Traditional display options:"),
                       _characterCombobox);
    _tabLayout->addRow(tr("Jyutping/Pinyin display options:"),
                       _phoneticCombobox);
    _tabLayout->addRow(tr("Pinyin display options:"), _mandarinCombobox);

    _tabLayout->addRow(_divider);

    _tabLayout->addRow(tr("Colour words by tone in:"), _colourCombobox);
    _tabLayout->addRow(tr("Jyutping tone colours:"), jyutpingColourWidget);
    _tabLayout->addRow(tr("Pinyin tone colours:"), pinyinColourWidget);
}

void SettingsTab::initializeCharacterComboBox(QComboBox &characterCombobox)
{
    characterCombobox.addItem(tr("Only Simplified"),
                              QVariant::fromValue<EntryPhoneticOptions>(
                                  EntryPhoneticOptions::ONLY_JYUTPING));
    characterCombobox.addItem(tr("Only Traditional"),
                              QVariant::fromValue<EntryPhoneticOptions>(
                                  EntryPhoneticOptions::ONLY_PINYIN));
    characterCombobox.addItem(tr("Both, Prefer Simplified"),
                              QVariant::fromValue<EntryPhoneticOptions>(
                                  EntryPhoneticOptions::PREFER_JYUTPING));
    characterCombobox.addItem(tr("Both, Prefer Traditional"),
                              QVariant::fromValue<EntryPhoneticOptions>(
                                  EntryPhoneticOptions::PREFER_PINYIN));

    characterCombobox.setCurrentIndex(characterCombobox.findData(
        _settings->value("characterOptions",
                         QVariant::fromValue(
                             EntryCharactersOptions::PREFER_TRADITIONAL))));

    connect(&characterCombobox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            [&](int index) {
                _settings->setValue("characterOptions",
                                    characterCombobox.itemData(index));
                _settings->sync();
            });
}

void SettingsTab::initializePhoneticComboBox(QComboBox &phoneticCombobox)
{
    phoneticCombobox.addItem(tr("Only Jyutping"),
                             QVariant::fromValue<EntryPhoneticOptions>(
                                 EntryPhoneticOptions::ONLY_JYUTPING));
    phoneticCombobox.addItem(tr("Only Pinyin"),
                             QVariant::fromValue<EntryPhoneticOptions>(
                                 EntryPhoneticOptions::ONLY_PINYIN));
    phoneticCombobox.addItem(tr("Both, Prefer Jyutping"),
                             QVariant::fromValue<EntryPhoneticOptions>(
                                 EntryPhoneticOptions::PREFER_JYUTPING));
    phoneticCombobox.addItem(tr("Both, Prefer Pinyin"),
                             QVariant::fromValue<EntryPhoneticOptions>(
                                 EntryPhoneticOptions::PREFER_PINYIN));

    phoneticCombobox.setCurrentIndex(phoneticCombobox.findData(
        _settings->value("phoneticOptions",
                         QVariant::fromValue(
                             EntryPhoneticOptions::PREFER_JYUTPING))));

    connect(&phoneticCombobox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            [&](int index) {
                _settings->setValue("phoneticOptions",
                                    phoneticCombobox.itemData(index));
                _settings->sync();
            });
}

void SettingsTab::initializeMandarinComboBox(QComboBox &mandarinCombobox)
{
    mandarinCombobox.addItem(tr("Pinyin with diacritics"),
                             QVariant::fromValue<MandarinOptions>(
                                 MandarinOptions::PRETTY_PINYIN));
    mandarinCombobox.addItem(tr("Pinyin with numbers"),
                             QVariant::fromValue<MandarinOptions>(
                                 MandarinOptions::RAW_PINYIN));

    mandarinCombobox.setCurrentIndex(mandarinCombobox.findData(
        _settings->value("mandarinOptions",
                         QVariant::fromValue(MandarinOptions::PRETTY_PINYIN))));

    connect(&mandarinCombobox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            [&](int index) {
                _settings->setValue("mandarinOptions",
                                    mandarinCombobox.itemData(index));
                _settings->sync();
            });
}

void SettingsTab::initializeColourComboBox(QComboBox &colourCombobox)
{
    colourCombobox.addItem(tr("Jyutping"),
                           QVariant::fromValue<EntryColourPhoneticType>(
                               EntryColourPhoneticType::JYUTPING));
    colourCombobox.addItem(tr("Pinyin"),
                           QVariant::fromValue<EntryColourPhoneticType>(
                               EntryColourPhoneticType::PINYIN));

    colourCombobox.setCurrentIndex(colourCombobox.findData(
        _settings->value("entryColourPhoneticType",
                         QVariant::fromValue(
                             EntryColourPhoneticType::JYUTPING))));

    connect(&colourCombobox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            [&](int index) {
                _settings->setValue("entryColourPhoneticType",
                                    colourCombobox.itemData(index));
                _settings->sync();
            });
}

void SettingsTab::initializeJyutpingColourWidget(QWidget &jyutpingColourWidget)
{
    // Create buttons that show colours for jyutping tones
    QGridLayout *jyutpingLayout = new QGridLayout{&jyutpingColourWidget};
    jyutpingLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    jyutpingLayout->setContentsMargins(0, 0, 0, 0);
    jyutpingLayout->setVerticalSpacing(5);
    jyutpingColourWidget.setLayout(jyutpingLayout);

    // Create rounded rect colour buttons and label for tone
    for (std::vector<std::string>::size_type i = 0;
         i < Settings::jyutpingToneColours.size();
         i++) {
        // First, add rounded rect colour button
        QPushButton *button = new QPushButton{this};
        button->setStyleSheet(QString{COLOUR_BUTTON_STYLE}.arg(
            Settings::jyutpingToneColours[i].c_str()));
        button->setMinimumSize(40, 20);
        button->setFixedWidth(40);
        button->setProperty("tone", static_cast<int>(i));
        jyutpingLayout->addWidget(button,
                                  0,
                                  static_cast<int>(i),
                                  Qt::AlignCenter);

        connect(button, &QPushButton::clicked, this, [&]() {
            // Get new colour from dialog
            QPushButton *sender = static_cast<QPushButton *>(QObject::sender());
            QColor newColour = getNewColour(sender->palette().button().color());
            if (!newColour.isValid()) {
                return;
            }

            // Save colour to both settings file and global jyutping config
            std::vector<std::string>::size_type index
                = static_cast<unsigned long>(sender->property("tone").toInt());
            Settings::jyutpingToneColours[index] = newColour.name().toStdString();
            sender->setStyleSheet(QString{COLOUR_BUTTON_STYLE}.arg(
                Settings::jyutpingToneColours[index].c_str()));

            saveJyutpingColours();
        });

        // Then add tone label underneath it
        QLabel *label = new QLabel{&jyutpingColourWidget};
        label->setAlignment(Qt::AlignHCenter);
        label->setMinimumWidth(40);
        switch (i) {
            case 0: {
                label->setText(tr("No Tone"));
                break;
            }
            default: {
                label->setText(tr("Tone %1").arg(i));
                break;
            }
        }
        jyutpingLayout->addWidget(label, 1, static_cast<int>(i));
    }
}

void SettingsTab::initializePinyinColourWidget(QWidget &pinyinColourWidget)
{
    // Get colours from QSettings
    _settings->beginReadArray("pinyinColors");
    for (int i = 0; i < 5; ++i) {
        _settings->setArrayIndex(i);
        QColor color
            = _settings
                  ->value("colour",
                          QColor{
                              Settings::pinyinToneColours[static_cast<ulong>(i)]
                                  .c_str()})
                  .value<QColor>();
        Settings::pinyinToneColours[static_cast<unsigned long>(i)]
            = color.name().toStdString();
    }
    _settings->endArray();

    // Create widgets
    QGridLayout *pinyinLayout = new QGridLayout{&pinyinColourWidget};
    pinyinLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    pinyinLayout->setContentsMargins(0, 0, 0, 0);
    pinyinLayout->setVerticalSpacing(5);
    pinyinColourWidget.setLayout(pinyinLayout);
    for (std::vector<std::string>::size_type i = 0;
         i < Settings::pinyinToneColours.size();
         i++) {
        QPushButton *button = new QPushButton{this};
        button->setStyleSheet(QString{COLOUR_BUTTON_STYLE}.arg(
            Settings::pinyinToneColours[i].c_str()));
        button->setMinimumSize(40, 20);
        button->setFixedWidth(40);
        button->setProperty("tone", static_cast<int>(i));
        pinyinLayout->addWidget(button, 0, static_cast<int>(i), Qt::AlignCenter);

        connect(button, &QPushButton::clicked, this, [&]() {
            QPushButton *sender = static_cast<QPushButton *>(QObject::sender());
            QColor newColour = getNewColour(sender->palette().button().color());
            if (!newColour.isValid()) {
                return;
            }

            std::vector<std::string>::size_type index
                = static_cast<unsigned long>(sender->property("tone").toInt());
            Settings::pinyinToneColours[index] = newColour.name().toStdString();
            sender->setStyleSheet(QString{COLOUR_BUTTON_STYLE}.arg(
                Settings::pinyinToneColours[index].c_str()));

            savePinyinColours();
        });

        QLabel *label = new QLabel{&pinyinColourWidget};
        label->setAlignment(Qt::AlignHCenter);
        label->setMinimumWidth(40);
        switch (i) {
            case 0: {
                label->setText(tr("No Tone"));
                break;
            }
            case 5: {
                label->setText(tr("Neutral"));
                break;
            }
            default: {
                label->setText(tr("Tone %1").arg(i));
                break;
            }
        }
        pinyinLayout->addWidget(label, 1, static_cast<int>(i));
    }
}

QColor SettingsTab::getNewColour(QColor old_colour)
{
    return QColorDialog::getColor(old_colour, this);
}

void SettingsTab::saveJyutpingColours()
{
    _settings->beginWriteArray("jyutpingColours");
    for (std::vector<std::string>::size_type i = 0;
         i < Settings::jyutpingToneColours.size();
         i++) {
        _settings->setArrayIndex(static_cast<int>(i));
        _settings->setValue("colour",
                            QColor{Settings::jyutpingToneColours[i]
                                       .c_str()});
    }
    _settings->endArray();
    _settings->sync();
}

void SettingsTab::savePinyinColours()
{
    _settings->beginWriteArray("pinyinColours");
    for (std::vector<std::string>::size_type i = 0;
         i < Settings::pinyinToneColours.size();
         i++) {
        _settings->setArrayIndex(static_cast<int>(i));
        _settings->setValue("colour",
                            QColor{
                                Settings::pinyinToneColours[i].c_str()});
    }
    _settings->endArray();
    _settings->sync();
}
