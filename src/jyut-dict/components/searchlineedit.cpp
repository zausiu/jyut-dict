#include "searchlineedit.h"

#include "logic/search/sqlsearch.h"

#include <QIcon>

#include <iostream>
#include <vector>

SearchLineEdit::SearchLineEdit(ISearchOptionsMediator *mediator, QWidget *parent)
    : QLineEdit(parent)
{
    _mediator = mediator;

    setPlaceholderText(tr("Search"));

    QIcon search = QIcon(":/images/search.png");
    QIcon clear = QIcon(":/images/x.png");
    QIcon search_inverted = QIcon(":/images/search_inverted.png");
    QIcon clear_inverted = QIcon(":/images/x_inverted.png");

    _clearLineEdit = new QAction("", this);
    addAction(_clearLineEdit, QLineEdit::TrailingPosition);
    connect(_clearLineEdit, &QAction::triggered,
            [this](){
                this->clear();
                removeAction(_clearLineEdit);
                _search->searchEnglish(text());});

    // Customize the look of the searchbar to fit in better with platform styles
#ifdef Q_OS_WIN
    setStyleSheet("QLineEdit { \
                     border-color: black; \
                     border-width: 2px; \
                     font-size: 12px; \
                     background-color: #ffffff; }");
    setMinimumHeight(25);
    addAction(search, QLineEdit::LeadingPosition);
    _clearLineEdit->setIcon(clear);
#elif defined(Q_OS_DARWIN)
    if (!system("defaults read -g AppleInterfaceStyle")) {
        setStyleSheet("QLineEdit { \
                         border-radius: 3px; \
                         border-color: black; \
                         border-width: 1px; \
                         font-size: 12px; \
                         padding-top: 4px; \
                         padding-bottom: 4px; \
                         selection-background-color: darkgray; \
                         background-color: #586365; }");
        addAction(search_inverted, QLineEdit::LeadingPosition);
        _clearLineEdit->setIcon(clear_inverted);
    } else {
        setStyleSheet("QLineEdit { \
                         border-radius: 3px; \
                         border-color: black; \
                         border-width: 1px; \
                         font-size: 12px; \
                         padding-top: 4px; \
                         padding-bottom: 4px; \
                         selection-background-color: darkgray; \
                         background-color: #ffffff; }");
         addAction(search, QLineEdit::LeadingPosition);
         _clearLineEdit->setIcon(clear);
    }
#else
        setStyleSheet("QLineEdit { \
                         border-radius: 3px; \
                         border-color: black; \
                         border-width: 1px; \
                         font-size: 12px; \
                         padding-top: 4px; \
                         padding-bottom: 4px; \
                         selection-background-color: darkgray; \
                         background-color: #ffffff; }");
        addAction(search, QLineEdit::LeadingPosition);
        _clearLineEdit->setIcon(clear);
#endif

    setMinimumWidth(parent->width() / 2);

    _search = new SQLSearch();
}

void SearchLineEdit::checkClearVisibility()
{
    if (text().isEmpty()) {
        removeAction(_clearLineEdit);
    } else {
        addAction(_clearLineEdit, QLineEdit::TrailingPosition);
    }
}

void SearchLineEdit::keyReleaseEvent(QKeyEvent *event)
{
    checkClearVisibility();
    QLineEdit::keyReleaseEvent(event);
    search();
}

// When in focus and text present, the clear button should be visible
void SearchLineEdit::focusInEvent(QFocusEvent *event)
{
    checkClearVisibility();
    QLineEdit::focusInEvent(event);
}

// When out of focus, the clear button should never be visible.
void SearchLineEdit::focusOutEvent(QFocusEvent *event)
{
    removeAction(_clearLineEdit);
    QLineEdit::focusOutEvent(event);
}

// Since the textChanged event happens before letters are painted,
// it feels slow.
// Thus, only use the signals and slots mechanism on Windows, and only
// when inputting Chinese.
void SearchLineEdit::updateParameters(SearchParameters parameters)
{
    _parameters = parameters;

#ifdef Q_OS_WIN
    if (_parameters == SearchParameters::SIMPLIFIED ||
            _parameters == SearchParameters::TRADITIONAL) {
        connect(this, &QLineEdit::textChanged,
                [this](){
                    this->checkClearVisibility();
                    this->search();});
    } else {
        disconnect(this, &QLineEdit::textChanged, nullptr, nullptr);
    }
#endif
}

void SearchLineEdit::search()
{
    switch (_parameters) {
        case SearchParameters::SIMPLIFIED: {
            _search->searchSimplified(text());
            break;
        }
        case SearchParameters::TRADITIONAL: {
            _search->searchTraditional(text());
            break;
        }
        case SearchParameters::PINYIN: {
            _search->searchPinyin(text());
            break;
        }
        case SearchParameters::JYUTPING: {
            _search->searchJyutping(text());
            break;
        }
        case SearchParameters::ENGLISH: {
            _search->searchEnglish(text());
            break;
        }
        default: {
            break;
        }
    }
}

SearchLineEdit::~SearchLineEdit()
{
    delete _clearLineEdit;
}
