#include "sentencescrollarea.h"

#include "components/entryview/entryscrollareawidget.h"
#include "logic/entry/definitionsset.h"
#include "logic/entry/entry.h"
#include "logic/sentence/sentenceset.h"

#include <QScrollBar>

#define ENTIRE_WIDTH -1

SentenceScrollArea::SentenceScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);

    _scrollAreaWidget = new SentenceScrollAreaWidget{this};

    setWidget(_scrollAreaWidget);
    setWidgetResizable(true); // IMPORTANT! This makes the scrolling widget resize correctly.
#ifdef Q_OS_LINUX
    setMinimumWidth(250);
#else
    setMinimumWidth(350);
#endif

    if (!parent) {
        setMinimumHeight(400);
    }
}

SentenceScrollArea::~SentenceScrollArea()
{
}

void SentenceScrollArea::setSourceSentence(const SourceSentence &sentence)
{
    _scrollAreaWidget->setSourceSentence(sentence);
    _scrollAreaWidget->resize(width()
                                  - (verticalScrollBar()->isVisible()
                                         ? verticalScrollBar()->width()
                                         : 0),
                              _scrollAreaWidget->sizeHint().height());
}

void SentenceScrollArea::resizeEvent(QResizeEvent *event)
{
    _scrollAreaWidget->resize(width()
                                  - (verticalScrollBar()->isVisible()
                                         ? verticalScrollBar()->width()
                                         : 0),
                              event->size().height());
    event->accept();
}
