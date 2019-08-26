#include "settingsutils.h"

#include <QCoreApplication>

namespace Settings
{

QTranslator systemTranslator;
QTranslator applicationTranslator;

QLocale currentLocale;

std::unique_ptr<QSettings> getSettings(QObject *parent)
{
#ifdef Q_OS_MAC
    QString settingsPath = QCoreApplication::applicationDirPath() + "/../Resources/settings.ini";
#elif defined(Q_OS_WIN)
    QString settingsPath = QCoreApplication::applicationDirPath() + "./settings.ini";
#elif defined(Q_OS_LINUX)
    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
#endif

    std::unique_ptr<QSettings> settings;

#ifdef PORTABLE
    settings = std::make_unique<QSettings>(settingsPath,
                                           QSettings::IniFormat,
                                           parent);
#else
    settings = std::make_unique<QSettings>(parent);
#endif

    updateSettings(*settings);

    return settings;
}

// Currently not used
bool updateSettings(QSettings &settings)
{
    if (settings.value("Metadata/version", QVariant{SETTINGS_VERSION}).toInt()
        != SETTINGS_VERSION) {
        // Convert to new version here
        settings.setValue("Metadata/version", QVariant{SETTINGS_VERSION});
        settings.sync();
    }
    return true;
}

bool clearSettings(QSettings &settings)
{
    settings.clear();
    settings.setValue("Metadata/version", QVariant{SETTINGS_VERSION});
    settings.sync();
    return true;
}

QLocale getCurrentLocale()
{
    return currentLocale;
}

bool setCurrentLocale(QLocale &locale)
{
    currentLocale = locale;
    return true;
}

bool isCurrentLocaleHan()
{
    return currentLocale.script() == QLocale::HanScript
           || currentLocale.script() == QLocale::SimplifiedHanScript
           || currentLocale.script() == QLocale::TraditionalHanScript;
}

}
