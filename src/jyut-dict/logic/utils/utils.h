#ifndef UTILS_H
#define UTILS_H

#include <qglobal.h>

#include <string>
#include <vector>

// The Utils class provides a set of utilities and defines useful
// in several places in the application.

namespace Utils {
#ifdef APPIMAGE
constexpr auto VARIANT = "appimage";
#else
#ifdef PORTABLE
constexpr auto VARIANT = "portable";
#else
constexpr auto VARIANT = "install";
#endif
#endif

#ifdef Q_OS_WIN
    constexpr auto SAME_CHARACTER_STRING = "−";
#else
    constexpr auto SAME_CHARACTER_STRING = "－";
#endif

    // Index of buttons for Searching
    enum ButtonOptionIndex {
        SIMPLIFIED_BUTTON_INDEX,
        TRADITIONAL_BUTTON_INDEX,
        JYUTPING_BUTTON_INDEX,
        PINYIN_BUTTON_INDEX,
        ENGLISH_BUTTON_INDEX,
        MAXIMUM_BUTTON_INDEX
    };

    // Strings that are not language dependent should go here.
    // If they need to be translated, put them in strings.h
    constexpr auto CURRENT_VERSION = "2.23.0408";
    constexpr auto AUTHOR_EMAIL = "mailto: hi@aaronhktan.com";
    constexpr auto DONATE_LINK = "https://ko-fi.com/aaronhktan";
    constexpr auto AUTHOR_GITHUB_LINK = "https://github.com/aaronhktan/";
    constexpr auto GITHUB_LINK = "https://github.com/aaronhktan/jyut-dict";
    constexpr auto WEBSITE_LINK = "https://jyutdictionary.com";
    constexpr auto DICTIONARY_DOWNLOAD_LINK
        = "https://jyutdictionary.com/#download-addon";
    constexpr auto EASTER_EGG_LINK
        = "https://img02.tooopen.com/20150824/tooopen_sy_139151591238.jpg";

    // Strings used for adding languages
#ifdef Q_OS_MAC
    constexpr auto TTS_LINK = "x-apple.systempreferences:com.apple."
                              "preference.universalaccess?TextToSpeech";
    constexpr auto TTS_HELP_LINK
        = "https://support.apple.com/guide/mac-help/mchlp2290/mac";
#elif defined(Q_OS_LINUX)
    constexpr auto TTS_LINK
        = "https://packages.debian.org/stretch/libspeechd-dev";
    constexpr auto TTS_HELP_LINK
        = "https://packages.debian.org/stretch/libspeechd-dev";
#else
    constexpr auto TTS_LINK
        = "ms-settings:regionlanguage";
    constexpr auto TTS_HELP_LINK
        = "https://support.microsoft.com/en-ca/help/22805/"
          "windows-10-supported-narrator-languages-voices";
#endif

    void split(const std::string &string,
               const char delimiter,
               std::vector<std::string> &result);

    void split(const std::string &string,
               const std::string delimiter,
               std::vector<std::string> &result);

    void trim(const std::string &string, std::string &result);
    } // namespace Utils

#endif // UTILS_H
