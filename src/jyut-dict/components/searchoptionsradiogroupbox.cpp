#include "searchoptionsradiogroupbox.h"

#include "logic/settings/settingsutils.h"

#include <QLocale>
#include <QStyle>

SearchOptionsRadioGroupBox::SearchOptionsRadioGroupBox(ISearchOptionsMediator *mediator,
                                                       QWidget *parent) :
    QGroupBox(parent)
{
    _mediator = mediator;
    _analytics = new Analytics{this};

    setupUI();
    translateUI();

    SearchParameters lastSelected
        = Settings::getSettings()
              ->value("SearchOptionsRadioGroupBox/lastSelected",
                      QVariant::fromValue(SearchParameters::ENGLISH))
              .value<SearchParameters>();
    QList<QRadioButton *> buttons = this->findChildren<QRadioButton *>();
    for (auto button : buttons) {
        if (button->property("data").value<SearchParameters>() == lastSelected) {
            button->click();
        #ifdef Q_OS_MAC
            // Makes the button selection show up correctly on macOS
            button->setDown(true);
        #endif
            break;
        }
    }
}

void SearchOptionsRadioGroupBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateUI();
    }
    QGroupBox::changeEvent(event);
}

void SearchOptionsRadioGroupBox::setOption(const Utils::ButtonOptionIndex index)
{
    if (index >= Utils::MAXIMUM_BUTTON_INDEX) {
        return;
    }

    QList<QRadioButton *> buttons = this->findChildren<QRadioButton *>();
    buttons.at(index)->click();
}

void SearchOptionsRadioGroupBox::setupUI()
{

    _layout = new QHBoxLayout{this};
    _layout->setContentsMargins(5, 5, 5, 5);

    _simplifiedButton = new QRadioButton{this};
    _traditionalButton = new QRadioButton{this};
    _jyutpingButton = new QRadioButton{this};
    _pinyinButton = new QRadioButton{this};
    _englishButton = new QRadioButton{this};

    _simplifiedButton->setProperty("data", QVariant::fromValue(SearchParameters::SIMPLIFIED));
    _traditionalButton->setProperty("data", QVariant::fromValue(SearchParameters::TRADITIONAL));
    _jyutpingButton->setProperty("data", QVariant::fromValue(SearchParameters::JYUTPING));
    _pinyinButton->setProperty("data", QVariant::fromValue(SearchParameters::PINYIN));
    _englishButton->setProperty("data", QVariant::fromValue(SearchParameters::ENGLISH));
#ifdef Q_OS_LINUX
    _simplifiedButton->setStyleSheet("QToolTip { padding: 1px; color: black }");
    _traditionalButton->setStyleSheet("QToolTip { padding: 1px; color: black }");
    _jyutpingButton->setStyleSheet("QToolTip { padding: 1px; color: black }");
    _pinyinButton->setStyleSheet("QToolTip { padding: 1px; color: black; }");
    _englishButton->setStyleSheet("QToolTip { padding: 1px; color: black; }");
#else
    _simplifiedButton->setStyleSheet("QToolTip { padding: 1px; }");
    _traditionalButton->setStyleSheet("QToolTip { padding: 1px; }");
    _jyutpingButton->setStyleSheet("QToolTip { padding: 1px; }");
    _pinyinButton->setStyleSheet("QToolTip { padding: 1px; }");
    _englishButton->setStyleSheet("QToolTip { padding: 1px; }");
#endif

    connect(_simplifiedButton, &QRadioButton::clicked, this, &SearchOptionsRadioGroupBox::notifyMediator);
    connect(_traditionalButton, &QRadioButton::clicked, this, &SearchOptionsRadioGroupBox::notifyMediator);
    connect(_jyutpingButton, &QRadioButton::clicked, this, &SearchOptionsRadioGroupBox::notifyMediator);
    connect(_pinyinButton, &QRadioButton::clicked, this, &SearchOptionsRadioGroupBox::notifyMediator);
    connect(_englishButton, &QRadioButton::clicked, this, &SearchOptionsRadioGroupBox::notifyMediator);

    _layout->addWidget(_simplifiedButton);
    _layout->addWidget(_traditionalButton);
    _layout->addWidget(_jyutpingButton);
    _layout->addWidget(_pinyinButton);
    _layout->addWidget(_englishButton);

    setLayout(_layout);
    setFlat(true);

#ifdef Q_OS_WIN
    setStyleSheet("QRadioButton[isHan=\"true\"] { font-size: 12px; }"
                  "QGroupBox { border: 0; }");
#else
    setStyleSheet("QGroupBox { border: 0; }");
#endif
}

void SearchOptionsRadioGroupBox::translateUI()
{
    // Set property so styling automatically changes
    setProperty("isHan", Settings::isCurrentLocaleHan());

    QList<QRadioButton *> buttons = this->findChildren<QRadioButton *>();
    for (auto button : buttons) {
        button->setProperty("isHan", Settings::isCurrentLocaleHan());
        button->style()->unpolish(button);
        button->style()->polish(button);
    }

    _simplifiedButton->setText(tr("SC"));
    _traditionalButton->setText(tr("TC"));
    _jyutpingButton->setText(tr("JP"));
    _pinyinButton->setText(tr("PY"));
    _englishButton->setText(tr("EN"));

    _simplifiedButton->setToolTip(tr("Search Simplified Chinese"));
    _traditionalButton->setToolTip(tr("Search Traditional Chinese"));
    _jyutpingButton->setToolTip(tr("Search Jyutping"));
    _pinyinButton->setToolTip(tr("Search Pinyin"));
    _englishButton->setToolTip(tr("Search English"));
}

void SearchOptionsRadioGroupBox::notifyMediator()
{
    QRadioButton *button = static_cast<QRadioButton *>(QObject::sender());

    std::string language = button->text().toStdString();
    QTimer::singleShot(100, this, [=]() {
        _analytics->sendEvent("search", "optionsChange", language);
    });

    Settings::getSettings()->setValue("SearchOptionsRadioGroupBox/lastSelected",
                                      button->property("data"));

    _mediator->setParameters(button->property("data").value<SearchParameters>());
}
