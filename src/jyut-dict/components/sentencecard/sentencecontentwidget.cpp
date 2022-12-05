#include "sentencecontentwidget.h"

#include "logic/settings/settingsutils.h"
#ifdef Q_OS_MAC
#include "logic/utils/utils_mac.h"
#elif defined (Q_OS_LINUX)
#include "logic/utils/utils_linux.h"
#elif defined(Q_OS_WIN)
#include "logic/utils/utils_windows.h"
#endif
#include "logic/utils/utils_qt.h"

#include <QTimer>

SentenceContentWidget::SentenceContentWidget(QWidget *parent) : QWidget(parent)
{
    _sentenceLayout = new QGridLayout{this};

    _sentenceNumberLabels = {};
    _simplifiedLabels = {};
    _traditionalLabels = {};
    _cantoneseLabels = {};
    _mandarinLabels = {};
    _sentenceLabels = {};
}

SentenceContentWidget::~SentenceContentWidget()
{
    cleanupLabels();
}

void SentenceContentWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange && !_paletteRecentlyChanged) {
        // QWidget emits a palette changed event when setting the stylesheet
        // So prevent it from going into an infinite loop with this timer
        _paletteRecentlyChanged = true;
        QTimer::singleShot(10, this, [=]() { _paletteRecentlyChanged = false; });

        // Set the style to match whether the user started dark mode
        setStyle(Utils::isDarkMode());
    }
    if (event->type() == QEvent::LanguageChange) {
        translateUI();
        setStyle(Utils::isDarkMode());
    }
    QWidget::changeEvent(event);
}

void SentenceContentWidget::setSentenceSet(const SentenceSet &set)
{
    cleanupLabels();

    if (set.isEmpty()) {
        return;
    }

    _sentenceLayout->setContentsMargins(10, 0, 10, 10);

    std::vector<Sentence::TargetSentence> sentences = set.getSentences();
    for (size_t i = 0; i < sentences.size(); i++) {
        std::string number = std::to_string(i + 1);
        _sentenceNumberLabels.push_back(new QLabel{number.c_str(), this});
        int definitionNumberWidth = _sentenceNumberLabels.back()
                                        ->fontMetrics()
                                        .boundingRect("999")
                                        .width();
        _sentenceNumberLabels.back()->setFixedWidth(definitionNumberWidth);
        int definitionNumberHeight = _sentenceNumberLabels.back()
                                         ->fontMetrics()
                                         .boundingRect("123PYingy")
                                         .height();
        _sentenceNumberLabels.back()->setFixedHeight(definitionNumberHeight);

        _sentenceLabels.push_back(new QLabel{QString{
                                                 sentences[i].sentence.c_str()}
                                                     .trimmed(), this});
        _sentenceLabels.back()->setProperty("language", sentences[i].language.c_str());
        _sentenceLabels.back()->setWordWrap(true);
        _sentenceLabels.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        _sentenceLayout->addWidget(_sentenceNumberLabels[i],
                                   static_cast<int>(i + 9), 0, Qt::AlignTop);
        _sentenceLayout->addWidget(_sentenceLabels[i],
                                   static_cast<int>(i + 9), 1, Qt::AlignTop);
    }

    setStyle(Utils::isDarkMode());
}

void SentenceContentWidget::setSourceSentenceVector(
    const std::vector<SourceSentence> &sourceSentences)
{
    if (sourceSentences.empty()) {
        return;
    }

    _sentenceLayout->setVerticalSpacing(0);
    _sentenceLayout->setContentsMargins(10, 0, 10, 0);

    // Iterate through all the source sentences
    for (size_t i = 0; i < sourceSentences.size(); i++) {
        std::string number = std::to_string(i + 1);
        _sentenceNumberLabels.push_back(new QLabel{number.c_str(), this});
        int definitionNumberWidth = _sentenceNumberLabels.back()
                                        ->fontMetrics()
                                        .boundingRect("999")
                                        .width();
        _sentenceNumberLabels.back()->setFixedWidth(definitionNumberWidth);
        int latinHeight = _sentenceNumberLabels.back()
                                         ->fontMetrics()
                                         .boundingRect("123PYingy")
                                         .height();
        _sentenceNumberLabels.back()->setFixedHeight(latinHeight);

        SourceSentence sourceSentence = sourceSentences.at(i);

        QString simplified = QString{sourceSentence.getSimplified().c_str()}
                                 .trimmed();
        _simplifiedLabels.push_back(new QLabel{simplified, this});
        _simplifiedLabels.back()->setWordWrap(true);
        _simplifiedLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        QString traditional = QString{sourceSentence.getTraditional().c_str()}
                                  .trimmed();
        _traditionalLabels.push_back(new QLabel{traditional, this});
        _traditionalLabels.back()->setWordWrap(true);
        _traditionalLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        // Get first enabled Cantonese pronunciation option by looping through
        // enabled bits of cantoneseOptions
        CantoneseOptions cantoneseOptions
            = Settings::getSettings()
                  ->value("Entry/cantonesePronunciationOptions",
                          QVariant::fromValue(CantoneseOptions::RAW_JYUTPING))
                  .value<CantoneseOptions>();
        for (uint32_t i = 1;
             i < static_cast<uint32_t>(CantoneseOptions::SENTRY);
             i <<= 1) {
            if (i & static_cast<uint32_t>(cantoneseOptions)) {
                cantoneseOptions = static_cast<CantoneseOptions>(i);
            }
        }

        // Do the same thing for mandarin
        MandarinOptions mandarinOptions
            = Settings::getSettings()
                  ->value("Entry/mandarinPronunciationOptions",
                          QVariant::fromValue(MandarinOptions::PRETTY_PINYIN))
                  .value<MandarinOptions>();
        for (uint32_t i = 1;
             i < static_cast<uint32_t>(MandarinOptions::SENTRY);
             i <<= 1) {
            if (i & static_cast<uint32_t>(mandarinOptions)) {
                mandarinOptions = static_cast<MandarinOptions>(i);
            }
        }

        sourceSentence.generatePhonetic(cantoneseOptions, mandarinOptions);

        QString cantonesePronunciation = QString{sourceSentence
                                                     .getCantonesePhonetic(
                                                         cantoneseOptions)
                                                     .c_str()}
                                             .trimmed();
        _cantoneseLabels.push_back(new QLabel{cantonesePronunciation, this});
        _cantoneseLabels.back()->setWordWrap(true);
        _cantoneseLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        QString mandarinPronunciation
            = QString{sourceSentence.getMandarinPhonetic(mandarinOptions).c_str()}
                  .trimmed();
        _mandarinLabels.push_back(new QLabel{mandarinPronunciation, this});
        _mandarinLabels.back()->setWordWrap(true);
        _mandarinLabels.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        SentenceSet targetSentenceSet = sourceSentence.getSentenceSets()[0];
        QString sentence
            = QString{targetSentenceSet.getSentences()[0].sentence.c_str()}
                  .trimmed();
        _sentenceLabels.push_back(new QLabel{sentence, this});
        _sentenceLabels.back()->setProperty("language",
                                            targetSentenceSet.getSentences()[0].language.c_str());
        _sentenceLabels.back()->setWordWrap(true);
        _sentenceLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        _sourceSentenceLanguage.push_back(new QLabel{this});
        _sourceSentenceLanguage.back()
            ->setProperty("language",
                          sourceSentence.getSourceLanguage().c_str());
        _sourceSentenceLanguage.back()->setText(
            Utils::getLanguageFromISO639(_sourceSentenceLanguage.back()
                                             ->property("language")
                                             .toString()
                                             .toStdString())
                .trimmed());
        _sourceSentenceLanguage.back()->setObjectName("sourceSentenceLanguage");

        addLabelsToLayout(
            _sentenceLayout,
            static_cast<int>(i),
            _sentenceNumberLabels[i],
            _simplifiedLabels[i],
            _traditionalLabels[i],
            _cantoneseLabels[i],
            _mandarinLabels[i],
            _sentenceLabels[i],
            _sourceSentenceLanguage[i],
            Settings::getSettings()
                ->value("SearchResults/phoneticOptions",
                        QVariant::fromValue(
                            EntryPhoneticOptions::PREFER_CANTONESE))
                .value<EntryPhoneticOptions>(),
            Settings::getSettings()
                ->value("characterOptions",
                        QVariant::fromValue(
                            EntryCharactersOptions::PREFER_TRADITIONAL))
                .value<EntryCharactersOptions>());
    }

    setStyle(Utils::isDarkMode());
}

void SentenceContentWidget::translateUI(void)
{
    for (const auto &label : _sourceSentenceLanguage) {
        label->setText(Utils::getLanguageFromISO639(
                           label->property("language").toString().toStdString())
                           .trimmed());
        label->resize(label->sizeHint());
    }
}

void SentenceContentWidget::setStyle(bool use_dark)
{
    QString sentenceNumberStyleSheet = "QLabel { color: %1; "
                                       "margin-top: 2px; }";
    QColor textColour = use_dark ? QColor{LABEL_TEXT_COLOUR_DARK_R,
                                          LABEL_TEXT_COLOUR_DARK_G,
                                          LABEL_TEXT_COLOUR_DARK_B}
                                 : QColor{LABEL_TEXT_COLOUR_LIGHT_R,
                                          LABEL_TEXT_COLOUR_LIGHT_R,
                                          LABEL_TEXT_COLOUR_LIGHT_R};
    for (auto label : _sentenceNumberLabels) {
        label->setStyleSheet(sentenceNumberStyleSheet.arg(textColour.name()));
    }

    int borderRadius = 10;
    QString radiusString = QString::number(borderRadius);
    for (const auto &label : _sourceSentenceLanguage) {
        QString sourceStyleSheet = "QLabel {"
                                   "background: %1; "
                                   "border-radius: %2px; "
                                   "color: %3; "
                                   "margin-top: 2px; "
                                   "padding: 2px; }";
        QColor languageColour = Utils::getLanguageColour(
            Utils::getISO639FromLanguage(label->text().trimmed()));
        QColor languageTextColour = Utils::getContrastingColour(languageColour);
        label->setStyleSheet(
            sourceStyleSheet.arg(languageColour.name(),
                                 radiusString,
                                 languageTextColour.name()));
        label->setText(Utils::getLanguageFromISO639(
                           label->property("language").toString().toStdString())
                           .trimmed());
        label->setMinimumHeight(borderRadius * 2);
        label->resize(label->sizeHint());
    }

    QString chineseStyleSheet = "QLabel { font-size: 16px; padding-left: 2px; }";
    for (const auto &label : _simplifiedLabels) {
#ifdef Q_OS_WIN
        QFont font = QFont{"Microsoft YaHei", 16};
        font.setStyleHint(QFont::System, QFont::PreferAntialias);
        label->setFont(font);
#endif
        label->setStyleSheet(chineseStyleSheet);
    }
    for (const auto &label : _traditionalLabels) {
#ifdef Q_OS_WIN
        QFont font = QFont{"Microsoft YaHei", 16};
        font.setStyleHint(QFont::System, QFont::PreferAntialias);
        label->setFont(font);
#endif
        label->setStyleSheet(chineseStyleSheet);
    }

    QString pronunciationStyleSheet = "QLabel { font-size: 13px; "
                                      "color: %1; "
                                      "padding-left: 2px; }";
    for (const auto &label : _cantoneseLabels) {
        label->setStyleSheet(pronunciationStyleSheet.arg(textColour.name()));
    }
    for (const auto &label : _mandarinLabels) {
        label->setStyleSheet(pronunciationStyleSheet.arg(textColour.name()));
    }

    QString sentenceStyleSheet = "QLabel { padding-left: 2px; }";
    for (const auto &label : _sentenceLabels) {
#ifdef Q_OS_WIN
        if (label->property("language").toString().trimmed() == "cmn" ||
                label->property("language").toString().trimmed() == "yue") {
            QFont font = QFont{"Microsoft YaHei"};
            font.setStyleHint(QFont::System, QFont::PreferAntialias);
            font.setPixelSize(14);
            label->setFont(font);
        } else {
            QFont font = QFont{"Microsoft YaHei", 10};
            font.setStyleHint(QFont::System, QFont::PreferAntialias);
            label->setFont(font);
        }
#endif
        label->setStyleSheet(sentenceStyleSheet);
    }
}

void SentenceContentWidget::addLabelsToLayout(
    QGridLayout *layout,
    int rowNumber,
    QLabel *sentenceNumberLabel,
    QLabel *simplifiedLabel,
    QLabel *traditionalLabel,
    QLabel *jyutpingLabel,
    QLabel *pinyinLabel,
    QLabel *sentenceLabel,
    QLabel *sourceSentenceLanguage,
    EntryPhoneticOptions phoneticOptions,
    EntryCharactersOptions characterOptions)
{
    layout->addWidget(sentenceNumberLabel,
                      rowNumber * 10,
                      0,
                      1,
                      1);

    layout->addWidget(sourceSentenceLanguage,
                      rowNumber * 10,
                      1,
                      1,
                      1,
                      Qt::AlignLeft);

    // Add the first character option
    switch (characterOptions) {
    case EntryCharactersOptions::ONLY_SIMPLIFIED:
        traditionalLabel->setVisible(false);
    case EntryCharactersOptions::PREFER_SIMPLIFIED:
        if (simplifiedLabel->text().isEmpty()) {
            simplifiedLabel->setVisible(false);
            break;
        }
        layout->addWidget(simplifiedLabel,
                          rowNumber * 10 + 1,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryCharactersOptions::ONLY_TRADITIONAL:
        simplifiedLabel->setVisible(false);
    case EntryCharactersOptions::PREFER_TRADITIONAL:
        if (traditionalLabel->text().isEmpty()) {
            traditionalLabel->setVisible(false);
            break;
        }
        layout->addWidget(traditionalLabel,
                          rowNumber * 10 + 1,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    }

    // Add the second character label (if applicable)
    switch (characterOptions) {
    case EntryCharactersOptions::PREFER_SIMPLIFIED:
        if (traditionalLabel->text().isEmpty()) {
            traditionalLabel->setVisible(false);
            break;
        }
        layout->addWidget(traditionalLabel,
                          rowNumber * 10 + 2,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryCharactersOptions::PREFER_TRADITIONAL:
        if (simplifiedLabel->text().isEmpty()) {
            simplifiedLabel->setVisible(false);
            break;
        }
        layout->addWidget(simplifiedLabel,
                          rowNumber * 10 + 2,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    default:
        break;
    }

    // TODO: Fix this so you can select Yale
    // Add the first phonetic label
    switch (phoneticOptions) {
    case EntryPhoneticOptions::ONLY_CANTONESE:
        pinyinLabel->setVisible(false);
    case EntryPhoneticOptions::PREFER_CANTONESE:
        if (jyutpingLabel->text().isEmpty()) {
            jyutpingLabel->setVisible(false);
            break;
        }
        layout->addWidget(jyutpingLabel,
                          rowNumber * 10 + 3,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryPhoneticOptions::ONLY_MANDARIN:
        jyutpingLabel->setVisible(false);
    case EntryPhoneticOptions::PREFER_MANDARIN:
        if (pinyinLabel->text().isEmpty()) {
            pinyinLabel->setVisible(false);
            break;
        }
        layout->addWidget(pinyinLabel,
                          rowNumber * 10 + 3,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    }

    // Add the second phonetic label (if applicable)
    switch (phoneticOptions) {
    case EntryPhoneticOptions::PREFER_CANTONESE:
        if (pinyinLabel->text().isEmpty()) {
            pinyinLabel->setVisible(false);
            break;
        }
        layout->addWidget(pinyinLabel,
                          rowNumber * 10 + 4,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryPhoneticOptions::PREFER_MANDARIN:
        if (jyutpingLabel->text().isEmpty()) {
            jyutpingLabel->setVisible(false);
            break;
        }
        layout->addWidget(jyutpingLabel,
                          rowNumber * 10 + 4,
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    default:
        break;
    }

    layout->addWidget(sentenceLabel,
                      rowNumber * 10 + 5,
                      1,
                      1,
                      -1,
                      Qt::AlignTop);
    if (sentenceLabel->text().isEmpty()) {
        sentenceLabel->setVisible(false);
    }

    _spaceLabels.push_back(new QLabel{" "});
    layout->addWidget(_spaceLabels.back(),
                      rowNumber * 10 + 6,
                      1,
                      1,
                      -1);
}

void SentenceContentWidget::cleanupLabels()
{
    clearLabelVector(_sentenceNumberLabels);
    clearLabelVector(_simplifiedLabels);
    clearLabelVector(_traditionalLabels);
    clearLabelVector(_cantoneseLabels);
    clearLabelVector(_mandarinLabels);
    clearLabelVector(_sentenceLabels);
    clearLabelVector(_sourceSentenceLanguage);
    clearLabelVector(_spaceLabels);
}

void SentenceContentWidget::clearLabelVector(std::vector<QLabel *> &vector)
{
    for (const auto &label : vector) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    vector.clear();
}
