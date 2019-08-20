#ifndef ADVANCEDTAB_H
#define ADVANCEDTAB_H

#include "logic/settings/settingsutils.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QSettings>
#include <QWidget>

class AdvancedTab : public QWidget
{
public:
    explicit AdvancedTab(QWidget *parent = nullptr);

private:
    void setupUI();
    void initializeUpdateCheckbox(QCheckBox &checkbox);

    QCheckBox *_updateCheckbox;

    QFormLayout *_tabLayout;

    std::unique_ptr<QSettings> _settings;
};

#endif // ADVANCEDTAB_H
