#ifndef STRINGS_H
#define STRINGS_H

#include <QTranslator>

// Strings should contain all the strings that need to be translated
// and are used in several different locations.

// To use these, #include <QCoreApplication>, then
// QCoreApplication::translate(Strings::STRINGS_CONTEXT, Strings::<define here>)

namespace Strings {

constexpr auto STRINGS_CONTEXT = "strings";

constexpr auto JYUTPING_SHORT = QT_TRANSLATE_NOOP("strings", "JP");
constexpr auto PINYIN_SHORT = QT_TRANSLATE_NOOP("strings", "PY");
constexpr auto DEFINITIONS_ALL_CAPS = QT_TRANSLATE_NOOP("strings", "DEFINITIONS");

constexpr auto PRODUCT_NAME = QT_TRANSLATE_NOOP("strings", "Jyut Dictionary");
constexpr auto PRODUCT_DESCRIPTION
    = QT_TRANSLATE_NOOP("strings",
                        "A free, open-source offline Cantonese Dictionary.");
constexpr auto CREDITS_TEXT = QT_TRANSLATE_NOOP(
    "strings",
    "Icon based on <a "
    "href=https://commons.wikimedia.org/wiki/"
    "Commons:Ancient_Chinese_characters_project "
    "style=\"color: %1; text-decoration: none\">Wikimedia Ancient "
    "Chinese characters project</a> files."
    "<br>UI icons from <a href=https://feathericons.com "
    "style=\"color: %1; text-decoration: none\">Feather Icons</a>."
    "<br>Made with <a href=\"#\" style=\"color: %1; text-decoration: none\">"
    "Qt</a>.");
constexpr auto CONTACT_TITLE
    = QT_TRANSLATE_NOOP("strings",
                        "%1 is made with ❤️ by Aaron Tan.");
constexpr auto CONTACT_BODY
    = QT_TRANSLATE_NOOP("strings",
                        "Donate 💵, report a bug 🐛, or just say hi 👋!");
constexpr auto CONTACT_BODY_NO_EMOJI
    = QT_TRANSLATE_NOOP("strings", "Donate, report a bug, or just say hi!");
constexpr auto OTHER_SOURCES = QT_TRANSLATE_NOOP(
    "strings",
    "<a href=https://www.pleco.com style=\"color: %1; "
    "text-decoration: none\">Pleco Dictionary</a>, "
    "<a href=http://www.cantonese.sheik.co.uk/dictionary/ "
    "style=\"color: %1; text-decoration: none\">CantoDict</a>, "
    "<a href=https://words.hk style=\"color: %1; text-decoration: "
    "none\">words.hk</a>");
constexpr auto YUE_ERROR_STRING = QT_TRANSLATE_NOOP(
    "strings",
    "A voice for \"Chinese - Hong Kong\" could not be found.");
constexpr auto YUE_DESCRIPTION_STRING
    = QT_TRANSLATE_NOOP("strings",
                        "Unable to use text to speech.\n\n"
                        "Debug: Locale: %1, Language: Chinese - Hong Kong");

constexpr auto ZH_TW_ERROR_STRING = QT_TRANSLATE_NOOP(
    "strings",
    "A voice for \"Chinese - Taiwan\" could not be found.");
constexpr auto ZH_TW_DESCRIPTION_STRING
    = QT_TRANSLATE_NOOP("strings",
                        "Unable to use text to speech.\n\n"
                        "Debug: Locale: %1, Language: Chinese - Taiwan");

constexpr auto ZH_CN_ERROR_STRING = QT_TRANSLATE_NOOP(
    "strings",
    "A voice for \"Chinese - Mainland China\" could not be found.");
constexpr auto ZH_CN_DESCRIPTION_STRING = QT_TRANSLATE_NOOP(
    "strings",
    "Unable to use text to speech.\n\n"
    "Debug: Locale: %1, Language: Chinese - Mainland");

}

#endif // STRINGS_H
