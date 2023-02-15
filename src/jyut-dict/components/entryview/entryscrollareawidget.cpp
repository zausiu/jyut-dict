#include "entryscrollareawidget.h"

#include "components/entryview/entryscrollarea.h"
#include "components/magnifywindow/magnifyscrollarea.h"
#ifdef Q_OS_MAC
#include "logic/utils/utils_mac.h"
#elif defined (Q_OS_LINUX)
#include "logic/utils/utils_linux.h"
#elif defined(Q_OS_WIN)
#include "logic/utils/utils_windows.h"
#endif

#include <QTimer>

EntryScrollAreaWidget::EntryScrollAreaWidget(
    std::shared_ptr<SQLUserDataUtils> sqlUserUtils,
    std::shared_ptr<SQLDatabaseManager> manager,
    QWidget *parent)
    : QWidget(parent)
    , _sqlUserUtils{sqlUserUtils}
    , _manager{manager}
{
    setObjectName("EntryScrollAreaWidget");

    // Entire Scroll Area
    _scrollAreaLayout = new QVBoxLayout{this};
    _scrollAreaLayout->setSpacing(0);
    _scrollAreaLayout->setContentsMargins(11, 11, 11, 11);

    _entryHeaderWidget = new EntryHeaderWidget{this};
    _entryActionWidget = new EntryActionWidget{sqlUserUtils, this};
    _entryContentWidget = new EntryContentWidget{manager, this};

    // Add all widgets to main layout
    _scrollAreaLayout->addWidget(_entryHeaderWidget);
    _scrollAreaLayout->addWidget(_entryActionWidget);
    _scrollAreaLayout->addWidget(_entryContentWidget);
    _scrollAreaLayout->addStretch(1);

    setStyle(Utils::isDarkMode());

    connect(this,
            &EntryScrollAreaWidget::stallUISentenceUpdate,
            _entryContentWidget,
            &EntryContentWidget::stallSentenceUIUpdate);

    connect(_entryActionWidget,
            &EntryActionWidget::openInNewWindowAction,
            this,
            &EntryScrollAreaWidget::openInNewWindowAction);

    connect(_entryActionWidget,
            &EntryActionWidget::openMagnifyWindowAction,
            this,
            &EntryScrollAreaWidget::openMagnifyWindowAction);
}

void EntryScrollAreaWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange && !_paletteRecentlyChanged) {
        // QWidget emits a palette changed event when setting the stylesheet
        // So prevent it from going into an infinite loop with this timer
        _paletteRecentlyChanged = true;
        QTimer::singleShot(10, this, [=]() { _paletteRecentlyChanged = false; });

        // Set the style to match whether the user started dark mode
        setStyle(Utils::isDarkMode());
    }
    QWidget::changeEvent(event);
}

void EntryScrollAreaWidget::setEntry(const Entry &entry)
{
    _entry = entry;
    _entryHeaderWidget->setEntry(entry);
    _entryActionWidget->setEntry(entry);
    _entryContentWidget->setEntry(entry);
}

void EntryScrollAreaWidget::setStyle(bool use_dark)
{
    (void) (use_dark);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("QWidget#EntryScrollAreaWidget { "
                  "   background-color: palette(base); "
                  "} ");
}

void EntryScrollAreaWidget::updateStyleRequested(void)
{
    QEvent event{QEvent::PaletteChange};
    QCoreApplication::sendEvent(_entryHeaderWidget, &event);
    QCoreApplication::sendEvent(_entryActionWidget, &event);

    _entryContentWidget->updateStyleRequested();

    QList<MagnifyScrollArea *> areas = findChildren<MagnifyScrollArea *>();
    for (auto &area : areas) {
        area->updateStyleRequested();
    }
}

void EntryScrollAreaWidget::openInNewWindowAction(void)
{
    EntryScrollArea *area = new EntryScrollArea{_sqlUserUtils,
                                                _manager,
                                                nullptr};
    area->setParent(this, Qt::Window);
    area->setAttribute(Qt::WA_DeleteOnClose);
    area->setEntry(_entry);
#ifndef Q_OS_MAC
    area->setWindowTitle(" ");
#endif
    emit area->stallSentenceUIUpdate();
    area->show();
}

void EntryScrollAreaWidget::openMagnifyWindowAction(void)
{
    MagnifyScrollArea *area = new MagnifyScrollArea{nullptr};
    area->setParent(this, Qt::Window);
    area->setAttribute(Qt::WA_DeleteOnClose);
    area->setEntry(_entry);
#ifndef Q_OS_MAC
    area->setWindowTitle(" ");
#endif
    area->show();
}
