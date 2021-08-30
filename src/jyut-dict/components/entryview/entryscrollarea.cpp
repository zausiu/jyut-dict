#include "entryscrollarea.h"

#include "components/entryview/entryscrollareawidget.h"
#include "logic/entry/definitionsset.h"
#include "logic/entry/entry.h"
#include "logic/sentence/sentenceset.h"

#include <QScrollBar>

#define ENTIRE_WIDTH -1

EntryScrollArea::EntryScrollArea(std::shared_ptr<SQLUserDataUtils> sqlUserUtils,
                                 std::shared_ptr<SQLDatabaseManager> manager,
                                 QWidget *parent)
    : QScrollArea(parent)
{
    _enableUIUpdateTimer = new QTimer{this};
    _updateUITimer = new QTimer{this};

    setFrameShape(QFrame::NoFrame);

    _scrollAreaWidget = new EntryScrollAreaWidget{sqlUserUtils, manager, this};

    setWidget(_scrollAreaWidget);
    setWidgetResizable(true); // IMPORTANT! This makes the scrolling widget resize correctly.
    setMinimumWidth(350);

    connect(this,
            &EntryScrollArea::stallSentenceUIUpdate,
            _scrollAreaWidget,
            &EntryScrollAreaWidget::stallUISentenceUpdate);

    if (!parent) {
        setMinimumHeight(400);
    }
}

EntryScrollArea::~EntryScrollArea()
{
}

void EntryScrollArea::setEntry(const Entry &entry)
{
    _updateUITimer->stop();
    disconnect(_updateUITimer, nullptr, nullptr, nullptr);

    _updateUITimer->setInterval(25);
    QObject::connect(_updateUITimer, &QTimer::timeout, this, [=]() {
        if (_enableUIUpdate) {
            _updateUITimer->stop();
            disconnect(_updateUITimer, nullptr, nullptr, nullptr);
            _scrollAreaWidget->setEntry(entry);
            _scrollAreaWidget->setVisible(false);
            _scrollAreaWidget->resize(width()
                                          - (verticalScrollBar()->isVisible()
                                                 ? verticalScrollBar()->width()
                                                 : 0),
                                      _scrollAreaWidget->sizeHint().height());
            _scrollAreaWidget->setVisible(true);
        }
    });
    _updateUITimer->start();
}

void EntryScrollArea::resizeEvent(QResizeEvent *event)
{
    _scrollAreaWidget->resize(width()
                                  - (verticalScrollBar()->isVisible()
                                         ? verticalScrollBar()->width()
                                         : 0),
                              _scrollAreaWidget->sizeHint().height());
    event->accept();
}

// This slot is intended to be called after the user inputs text in the search
// box. The call to setEntry(entry) is very CPU intensive due to
// the time required to lay out many widgets, and it also must occur on the
// main GUI thread.
//
// We don't want to freeze the UI while the user types, so we disable the call
// to setEntry(entry) for 200ms. A pause of 200ms is good enough to assume that
// the user is done typing, so the expensive GUI operation can run.
//
// However, we don't want to stall the UI update every time a call to
// setEntry(entry) occurs! For example, we don't want to stall the UI update
// if the user is clicking on a result in the list view.
void EntryScrollArea::stallEntryUIUpdate(void)
{
    _enableUIUpdate = false;
    _enableUIUpdateTimer->stop();
    disconnect(_enableUIUpdateTimer, nullptr, nullptr, nullptr);
    _enableUIUpdateTimer->setInterval(200);
    _enableUIUpdateTimer->setSingleShot(true);
    QObject::connect(_enableUIUpdateTimer, &QTimer::timeout, this, [=]() {
        _enableUIUpdate = true;
    });
    _enableUIUpdateTimer->start();
}
