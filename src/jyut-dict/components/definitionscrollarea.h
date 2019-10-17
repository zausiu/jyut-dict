#ifndef DEFINITIONSCROLLAREA_H
#define DEFINITIONSCROLLAREA_H

#include "components/definitionscrollareawidget.h"
#include "logic/entry/entry.h"

#include <QEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

// The DefinitionScrollArea is the "detail" view
// It displays an Entry object in the user interface

// The layout of the DefinitionScrollArea is:
// EntryHeader - shows word in chinese, pronunciation guides
// Definition Widget - contains all the definitions and headers for each source

class DefinitionScrollArea : public QScrollArea
{
public:
    explicit DefinitionScrollArea(QWidget *parent = nullptr);
    ~DefinitionScrollArea() override;

    void changeEvent(QEvent *event) override;

    void setEntry(const Entry &entry);
private:
    void resizeEvent(QResizeEvent *event) override;
    void setStyle(bool use_dark);

    bool _paletteRecentlyChanged = false;

    // Widget that contains elements to scroll
    DefinitionScrollAreaWidget *_scrollAreaWidget;

    void testEntry();
};

#endif // DEFINITIONSCROLLAREA_H
