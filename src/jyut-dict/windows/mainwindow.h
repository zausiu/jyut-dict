#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "components/mainsplitter.h"
#include "components/maintoolbar.h"
#include "logic/update/githubreleasechecker.h"

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

// As its name suggests, is the main window of the application
// Contains a toolbar (for searching), and splitter (for results/detail)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createMenus();
    void createActions();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

    void toggleMinimized();
    void toggleMaximized();

private:
    GithubReleaseChecker *_checker;

    MainToolBar *_mainToolBar;
    MainSplitter *_mainSplitter;

    QMenu *_fileMenu;
    QMenu *_editMenu;
    QMenu *_windowMenu;
    QMenu *_helpMenu;

public slots:
    void notifyUpdateAvailable(bool updateAvailable,
                               std::string versionNumber,
                               std::string url, std::string description);
};

#endif // MAINWINDOW_H
