#ifndef MAINSPLITTER_H
#define MAINSPLITTER_H

#include "components/definitionscrollarea.h"
#include "logic/analytics/analytics.h"
#include "logic/search/sqlsearch.h"

#include <QModelIndex>
#include <QSplitter>
#include <QWidget>

// The MainSplitter contains a "master" listview and a "detail" scrollarea
//
// It handles the model changed signal that the master listview emits,
// and passes the data to the detail scrollarea.
//
// TODO: Perhaps move that logic out into its own class, to further decouple
// UI and functionality?

class MainSplitter : public QSplitter
{
Q_OBJECT

public:
    explicit MainSplitter(QWidget *parent = nullptr);
    ~MainSplitter() override;

private:
    Analytics *_analytics;

    DefinitionScrollArea *_definitionScrollArea;
    QListView *_resultListView;

private slots:
    void handleSelectionChanged(const QModelIndex &selection);
};

#endif // MAINSPLITTER_H
