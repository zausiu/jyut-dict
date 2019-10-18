#include "definitionheaderwidget.h"

#ifdef Q_OS_MAC
#include "logic/utils/utils_mac.h"
#endif

#include <QTimer>

DefinitionHeaderWidget::DefinitionHeaderWidget(QWidget *parent)
    : QWidget(parent)
{
    _layout = new QVBoxLayout{this};
    _layout->setContentsMargins(10, 10, 10, 10);
    _layout->setSpacing(10);

    _titleLabel = new QLabel{this};

    _layout->addWidget(_titleLabel);

#ifdef Q_OS_MAC
    setStyle(Utils::isDarkMode());
#else
    setStyle(/* use_dark = */false);
#endif
}

DefinitionHeaderWidget::DefinitionHeaderWidget(std::string title, QWidget *parent)
    : DefinitionHeaderWidget(parent)
{
    setSectionTitle(title);
}

void DefinitionHeaderWidget::changeEvent(QEvent *event)
{
#ifdef Q_OS_MAC
    if (event->type() == QEvent::PaletteChange && !_paletteRecentlyChanged) {
        // QWidget emits a palette changed event when setting the stylesheet
        // So prevent it from going into an infinite loop with this timer
        _paletteRecentlyChanged = true;
        QTimer::singleShot(100, [=]() { _paletteRecentlyChanged = false; });

        // Set the style to match whether the user started dark mode
        setStyle(Utils::isDarkMode());
    }
#endif
    QWidget::changeEvent(event);
}

DefinitionHeaderWidget::~DefinitionHeaderWidget()
{

}

void DefinitionHeaderWidget::setSectionTitle(std::string title)
{
    _titleLabel->setText(title.c_str());
    _titleLabel->setFixedHeight(_titleLabel->fontMetrics().boundingRect(title.c_str()).height());
}

void DefinitionHeaderWidget::setStyle(bool use_dark)
{
    // Style the main background
    QString widgetStyleSheet = "QWidget { "
                               " background-color: %1; "
                               " border-top-left-radius: 10px; "
                               " border-top-right-radius: 10px; "
                               " border-bottom-left-radius: 0px; "
                               " border-bottom-right-radius: 0px; "
                               "}";
    QColor backgroundColour = use_dark ? QColor{60, 60, 60}
                                       : QColor{235, 235, 235};
    setStyleSheet(widgetStyleSheet.arg(backgroundColour.name()));

    // Style the label text
    QString textStyleSheet = "QLabel { color: %1; }";
    QColor textColour = use_dark ? QColor{168, 168, 168}
                                 : QColor{111, 111, 111};
    _titleLabel->setStyleSheet(textStyleSheet.arg(textColour.name()));
}
