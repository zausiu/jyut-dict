#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include "components/searchlineedit.h"
#include "components/searchoptionsradiogroupbox.h"
#include "logic/search/isearchoptionsmediator.h"
#include "logic/search/sqldatabasemanager.h"

#include <QHBoxLayout>
#include <QToolBar>
#include <QWidget>

#include <memory>

// The MainToolBar is an object that contains all widgets in the toolbar
// This includes the searchbar,
// and any other radio buttons that change search parameters

class MainToolBar : public QToolBar
{
public:
    explicit MainToolBar(std::shared_ptr<SQLDatabaseManager> manager,
                         QWidget *parent = nullptr);
    ~MainToolBar();

private:
    QHBoxLayout *_toolBarLayout;
    QWidget *_toolBarWidget;

    SearchLineEdit *_searchBar;
    SearchOptionsRadioGroupBox *_optionsBox;
    ISearchOptionsMediator *_searchOptions;
};

#endif // MAINTOOLBAR_H
