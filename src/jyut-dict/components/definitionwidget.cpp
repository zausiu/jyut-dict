#include "definitionwidget.h"

DefinitionWidget::DefinitionWidget(QWidget *parent) : QWidget(parent)
{
    _definitionSectionsLayout = new QVBoxLayout{this};
    _definitionSectionsLayout->setContentsMargins(0, 0, 0, 0);
    _definitionSectionsLayout->setSpacing(15);
}

DefinitionWidget::DefinitionWidget(Entry &entry, QWidget *parent)
    : DefinitionWidget(parent)
{
    setEntry(entry);
}

DefinitionWidget::~DefinitionWidget()
{

}

// Must override this so that the layout is rendered properly with the
// appropriate sizes
#include <QDebug>
QSize DefinitionWidget::sizeHint() const
{
    qDebug() << "Height: " << height();
    if (_definitionSections.empty()) {
        return QSize(0, 0);
    } else if (_definitionSections.size() > 1) {
        int height = 0;
        for (auto section: _definitionSections) {
            height += section->sizeHint().height()
                      + _definitionSectionsLayout->spacing();
//            qDebug() << "Section height: " << section->sizeHint().height();
        }
//        qDebug() << "Calculated height: " << height;
        return QSize(width(), height);
    } else {
        int height = 0;
        for (auto section: _definitionSections) {
            height += section->sizeHint().height();
        }
        return QSize(width(), height);
    }
}

void DefinitionWidget::setEntry(const Entry &entry)
{
    cleanup();
    for (auto definitionsSet : entry.getDefinitionsSets()) {
        _definitionSections.push_back(new DefinitionSectionWidget{this});
        _definitionSections.back()->setEntry(definitionsSet);

        _definitionSectionsLayout->addWidget(_definitionSections.back());
    }

    // Force layout to update after adding widgets; fixes some layout issues.
    _definitionSectionsLayout->activate();
    updateGeometry();
}

void DefinitionWidget::cleanup()
{
    for (auto section : _definitionSections) {
        _definitionSectionsLayout->removeWidget(section);
        delete section;
    }
    _definitionSections.clear();
}
