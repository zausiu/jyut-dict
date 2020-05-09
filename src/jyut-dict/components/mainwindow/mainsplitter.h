#ifndef MAINSPLITTER_H
#define MAINSPLITTER_H

#include "components/entryview/entryscrollarea.h"
#include "logic/analytics/analytics.h"
#include "logic/database/sqldatabasemanager.h"
#include "logic/entry/entry.h"
#include "logic/search/sqlsearch.h"

#include <QAbstractListModel>
#include <QModelIndex>
#include <QSplitter>
#include <QWidget>

// The MainSplitter contains a "master" listview and a "detail" scrollarea
//
// It handles the model changed signal that the master listview emits,
// and passes the data to the detail scrollarea.

class MainSplitter : public QSplitter
{
Q_OBJECT

public:
    explicit MainSplitter(std::shared_ptr<SQLDatabaseManager> manager,
                          std::shared_ptr<SQLSearch> sqlSearch,
                          QWidget *parent = nullptr);
    ~MainSplitter() override;

    void setFocusToResults(void);
    void openCurrentSelectionInNewWindow(void);

private:
    std::shared_ptr<SQLDatabaseManager> _manager;
    Analytics *_analytics;

    EntryScrollArea *_entryScrollArea;
    QAbstractListModel *_model;
    QListView *_resultListView;

private slots:
    void prepareEntry(Entry &entry);

    void handleClick(const QModelIndex &selection);
    void handleDoubleClick(const QModelIndex &selection);
};

#endif // MAINSPLITTER_H
