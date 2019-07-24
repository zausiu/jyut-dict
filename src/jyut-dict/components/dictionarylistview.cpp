#include "dictionarylistview.h"

#include "components/dictionarylistdelegate.h"
#include "components/dictionarylistmodel.h"

DictionaryListView::DictionaryListView(QWidget *parent)
    : QListView(parent)
{
    setFrameShape(QFrame::NoFrame);
    setMinimumWidth(250);

    _model = new DictionaryListModel{this};
    setModel(_model);

    _delegate = new DictionaryListDelegate{this};
    setItemDelegate(_delegate);

    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

// On Windows, because of a bug in Qt (see QTBUG-7232), every time mouse
// is scrolled, listview advances by by three items. Override the wheelEvent to
// modify this undesired behaviour until fixed by Qt.
#ifdef Q_OS_WIN
void SearchListView::wheelEvent(QWheelEvent *event)
{
    int singleStep = verticalScrollBar()->singleStep();
    singleStep = qMin(singleStep, 10);
    verticalScrollBar()->setSingleStep(singleStep);
    QAbstractItemView::wheelEvent(event);
}
#endif
