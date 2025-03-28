#ifndef UTILS_QT_H
#define UTILS_QT_H

#include "logic/search/searchparameters.h"

#include <QColor>

// Whereas utils.h should contain only STL functions, utils_qt can contain
// stuff specific to Qt.

namespace Utils {

#define TITLE_BAR_BACKGROUND_COLOR_DARK_R 42
#define TITLE_BAR_BACKGROUND_COLOR_DARK_G 42
#define TITLE_BAR_BACKGROUND_COLOR_DARK_B 42

#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_DARK_R 45
#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_DARK_G 45
#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_DARK_B 45

#define TITLE_BAR_BORDER_COLOR_DARK_R 0
#define TITLE_BAR_BORDER_COLOR_DARK_G 0
#define TITLE_BAR_BORDER_COLOR_DARK_B 0

#define LABEL_TEXT_COLOUR_DARK_R 168
#define LABEL_TEXT_COLOUR_DARK_G 168
#define LABEL_TEXT_COLOUR_DARK_B 168

#define HEADER_BACKGROUND_COLOUR_DARK_R 60
#define HEADER_BACKGROUND_COLOUR_DARK_G 60
#define HEADER_BACKGROUND_COLOUR_DARK_B 60

#define CONTENT_BACKGROUND_COLOUR_DARK_R 50
#define CONTENT_BACKGROUND_COLOUR_DARK_G 50
#define CONTENT_BACKGROUND_COLOUR_DARK_B 50

#define BACKGROUND_COLOUR_DARK_R 30
#define BACKGROUND_COLOUR_DARK_G 30
#define BACKGROUND_COLOUR_DARK_B 30

#define LIST_ITEM_ACTIVE_COLOUR_DARK_R 204
#define LIST_ITEM_ACTIVE_COLOUR_DARK_G 0
#define LIST_ITEM_ACTIVE_COLOUR_DARK_B 1

#ifdef Q_OS_MAC
#define LIST_ITEM_INACTIVE_COLOUR_DARK_R 220
#define LIST_ITEM_INACTIVE_COLOUR_DARK_G 220
#define LIST_ITEM_INACTIVE_COLOUR_DARK_B 220
#else
#define LIST_ITEM_INACTIVE_COLOUR_DARK_R 45
#define LIST_ITEM_INACTIVE_COLOUR_DARK_G 45
#define LIST_ITEM_INACTIVE_COLOUR_DARK_B 45
#endif

#define TOOLBAR_TEXT_INACTIVE_COLOUR_DARK_R 128
#define TOOLBAR_TEXT_INACTIVE_COLOUR_DARK_G 128
#define TOOLBAR_TEXT_INACTIVE_COLOUR_DARK_B 128

#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_DARK_R 255
#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_DARK_G 255
#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_DARK_B 255

#define LABEL_TEXT_COLOUR_LIGHT_R 111
#define LABEL_TEXT_COLOUR_LIGHT_G 111
#define LABEL_TEXT_COLOUR_LIGHT_B 111

#define TITLE_BAR_BACKGROUND_COLOR_R 212
#define TITLE_BAR_BACKGROUND_COLOR_G 212
#define TITLE_BAR_BACKGROUND_COLOR_B 212

#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_R 246
#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_G 246
#define TITLE_BAR_INACTIVE_BACKGROUND_COLOR_B 246

#define TITLE_BAR_BORDER_COLOR_R 191
#define TITLE_BAR_BORDER_COLOR_G 191
#define TITLE_BAR_BORDER_COLOR_B 191

#define HEADER_BACKGROUND_COLOUR_LIGHT_R 215
#define HEADER_BACKGROUND_COLOUR_LIGHT_G 215
#define HEADER_BACKGROUND_COLOUR_LIGHT_B 215

#define CONTENT_BACKGROUND_COLOUR_LIGHT_R 235
#define CONTENT_BACKGROUND_COLOUR_LIGHT_G 235
#define CONTENT_BACKGROUND_COLOUR_LIGHT_B 235

#define BACKGROUND_COLOUR_LIGHT_R 255
#define BACKGROUND_COLOUR_LIGHT_G 255
#define BACKGROUND_COLOUR_LIGHT_B 255

#define LIST_ITEM_ACTIVE_COLOUR_LIGHT_R 204
#define LIST_ITEM_ACTIVE_COLOUR_LIGHT_G 0
#define LIST_ITEM_ACTIVE_COLOUR_LIGHT_B 1

#define LIST_ITEM_INACTIVE_COLOUR_LIGHT_R 220
#define LIST_ITEM_INACTIVE_COLOUR_LIGHT_G 220
#define LIST_ITEM_INACTIVE_COLOUR_LIGHT_B 220

#define TOOLBAR_TEXT_INACTIVE_COLOUR_LIGHT_R 128
#define TOOLBAR_TEXT_INACTIVE_COLOUR_LIGHT_G 128
#define TOOLBAR_TEXT_INACTIVE_COLOUR_LIGHT_B 128

#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_LIGHT_R 0
#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_LIGHT_G 0
#define TOOLBAR_TEXT_NOT_FOCUSED_COLOUR_LIGHT_B 0

// The following is used to calculate a contrasting colour
// https://stackoverflow.com/questions/946544/good-text-foreground-color-for-a-given-background-color
QColor getContrastingColour(const QColor backgroundColour);

QColor getLanguageColour(std::string language);

QString getLanguageFromISO639(std::string language);
std::string getISO639FromLanguage(QString language);

std::string getStringFromSearchParameter(SearchParameters parameter);

void refreshLanguageMap();

QString getLicenseFolderPath();
} // namespace Utils

#endif // UTILS_QT_H
