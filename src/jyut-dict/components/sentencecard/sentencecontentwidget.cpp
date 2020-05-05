#include "sentencecontentwidget.h"

#include "logic/settings/settingsutils.h"
#ifdef Q_OS_MAC
#include "logic/utils/utils_mac.h"
#endif
#include "logic/utils/utils_qt.h"

#include <QTimer>

SentenceContentWidget::SentenceContentWidget(QWidget *parent) : QWidget(parent)
{
    _sentenceLayout = new QGridLayout{this};

    _sentenceNumberLabels = {};
    _simplifiedLabels = {};
    _traditionalLabels = {};
    _jyutpingLabels = {};
    _pinyinLabels = {};
    _sentenceLabels = {};
}

SentenceContentWidget::~SentenceContentWidget()
{
    cleanupLabels();
}

void SentenceContentWidget::changeEvent(QEvent *event)
{
#if defined(Q_OS_DARWIN)
    if (event->type() == QEvent::PaletteChange && !_paletteRecentlyChanged) {
        // QWidget emits a palette changed event when setting the stylesheet
        // So prevent it from going into an infinite loop with this timer
        _paletteRecentlyChanged = true;
        QTimer::singleShot(10, [=]() { _paletteRecentlyChanged = false; });

        // Set the style to match whether the user started dark mode
        setStyle(Utils::isDarkMode());
    }
#endif
    QWidget::changeEvent(event);
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
                                        .boundingRect("PY")
                                        .width();
        _sentenceNumberLabels.back()->setFixedWidth(definitionNumberWidth);
        int latinHeight = _sentenceNumberLabels.back()
                                         ->fontMetrics()
                                         .boundingRect("123PYingy")
                                         .height();
        _sentenceNumberLabels.back()->setFixedHeight(latinHeight);

        SourceSentence sourceSentence = sourceSentences.at(i);

        _simplifiedLabels.push_back(
            new QLabel{QString{sourceSentence.getSimplified().c_str()}.trimmed(),
                       this});
        _simplifiedLabels.back()->setWordWrap(true);
        _simplifiedLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        _traditionalLabels.push_back(
            new QLabel{QString{sourceSentence.getTraditional().c_str()}.trimmed(),
                       this});
        _traditionalLabels.back()->setWordWrap(true);
        _traditionalLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        _jyutpingLabels.push_back(new QLabel{
            QString{sourceSentence
                        .getCantonesePhonetic(
                            Settings::getSettings()
                                ->value("cantoneseOptions",
                                        QVariant::fromValue(
                                            CantoneseOptions::RAW_JYUTPING))
                                .value<CantoneseOptions>())
                        .c_str()}
                .trimmed(),
            this});
        _jyutpingLabels.back()->setWordWrap(true);
        _jyutpingLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        _pinyinLabels.push_back(new QLabel{
            QString{sourceSentence
                        .getMandarinPhonetic(
                            Settings::getSettings()
                                ->value("mandarinOptions",
                                        QVariant::fromValue(
                                            MandarinOptions::PRETTY_PINYIN))
                                .value<MandarinOptions>())
                        .c_str()}
                .trimmed(),
            this});
        _pinyinLabels.back()->setWordWrap(true);
        _pinyinLabels.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        SentenceSet targetSentenceSet = sourceSentence.getSentenceSets()[0];
        _sentenceLabels.push_back(
            new QLabel{QString{
                           targetSentenceSet.getSentences()[0].sentence.c_str()}
                               .trimmed()
                           + "\n",
                       this});
        _sentenceLabels.back()->setWordWrap(true);
        _sentenceLabels.back()->setTextInteractionFlags(
            Qt::TextSelectableByMouse);

        addLabelsToLayout(
            _sentenceLayout,
            static_cast<int>(i),
            _sentenceNumberLabels[i],
            _simplifiedLabels[i],
            _traditionalLabels[i],
            _jyutpingLabels[i],
            _pinyinLabels[i],
            _sentenceLabels[i],
            Settings::getSettings()
                ->value("phoneticOptions",
                        QVariant::fromValue(
                            EntryPhoneticOptions::PREFER_JYUTPING))
                .value<EntryPhoneticOptions>(),
            Settings::getSettings()
                ->value("characterOptions",
                        QVariant::fromValue(
                            EntryCharactersOptions::PREFER_TRADITIONAL))
                .value<EntryCharactersOptions>());
    }

    _sentenceLayout->activate();

#ifdef Q_OS_MAC
    setStyle(Utils::isDarkMode());
#else
    setStyle(/* use_dark = */ false);
#endif
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
                                        .boundingRect("PY")
                                        .width();
        _sentenceNumberLabels.back()->setFixedWidth(definitionNumberWidth);
        int definitionNumberHeight = _sentenceNumberLabels.back()
                                         ->fontMetrics()
                                         .boundingRect("123PYing")
                                         .height();
        _sentenceNumberLabels.back()->setFixedHeight(definitionNumberHeight);

        _sentenceLabels.push_back(new QLabel{sentences[i].sentence.c_str(), this});
        _sentenceLabels.back()->setWordWrap(true);
        _sentenceLabels.back()->setTextInteractionFlags(Qt::TextSelectableByMouse);

        _sentenceLayout->addWidget(_sentenceNumberLabels[i],
                                     static_cast<int>(i + 9), 0, Qt::AlignTop);
        _sentenceLayout->addWidget(_sentenceLabels[i],
                                     static_cast<int>(i + 9), 1, Qt::AlignTop);
    }

    _sentenceLayout->activate();

#ifdef Q_OS_MAC
    setStyle(Utils::isDarkMode());
#else
    setStyle(/* use_dark = */false);
#endif
}

void SentenceContentWidget::setStyle(bool use_dark)
{
    QString sentenceNumberStyleSheet = "QLabel { color: %1; }";
    QColor textColour = use_dark ? QColor{LABEL_TEXT_COLOUR_DARK_R,
                                          LABEL_TEXT_COLOUR_DARK_G,
                                          LABEL_TEXT_COLOUR_DARK_B}
                                 : QColor{LABEL_TEXT_COLOUR_LIGHT_R,
                                          LABEL_TEXT_COLOUR_LIGHT_R,
                                          LABEL_TEXT_COLOUR_LIGHT_R};
    for (auto label : _sentenceNumberLabels) {
        label->setStyleSheet(sentenceNumberStyleSheet.arg(textColour.name()));
    }

    QString chineseStyleSheet = "QLabel { font-size: 16px; }";
    for (const auto &label : _simplifiedLabels) {
        label->setStyleSheet(chineseStyleSheet);
    }
    for (const auto &label : _traditionalLabels) {
        label->setStyleSheet(chineseStyleSheet);
    }

    QString pronunciationStyleSheet = "QLabel { font-size: 13px; color: %1; }";
    for (const auto &label : _jyutpingLabels) {
        label->setStyleSheet(pronunciationStyleSheet.arg(textColour.name()));
    }
    for (const auto &label : _pinyinLabels) {
        label->setStyleSheet(pronunciationStyleSheet.arg(textColour.name()));
    }
}

void SentenceContentWidget::cleanupLabels()
{
    for (auto label : _sentenceNumberLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _sentenceNumberLabels.clear();

    for (auto label : _simplifiedLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _simplifiedLabels.clear();

    for (auto label : _traditionalLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _traditionalLabels.clear();

    for (auto label : _jyutpingLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _jyutpingLabels.clear();

    for (auto label : _pinyinLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _pinyinLabels.clear();

    for (auto label : _sentenceLabels) {
        _sentenceLayout->removeWidget(label);
        delete label;
    }
    _sentenceLabels.clear();
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
    EntryPhoneticOptions phoneticOptions,
    EntryCharactersOptions characterOptions)
{
    layout->addWidget(sentenceNumberLabel,
                      static_cast<int>(rowNumber * 6),
                      0,
                      6,
                      1,
                      Qt::AlignTop);

    // Add the first character option
    switch (characterOptions) {
    case EntryCharactersOptions::ONLY_SIMPLIFIED:
        traditionalLabel->setVisible(false);
        [[clang::fallthrough]];
    case EntryCharactersOptions::PREFER_SIMPLIFIED:
        layout->addWidget(simplifiedLabel,
                          static_cast<int>(rowNumber * 6),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryCharactersOptions::ONLY_TRADITIONAL:
        simplifiedLabel->setVisible(false);
        [[clang::fallthrough]];
    case EntryCharactersOptions::PREFER_TRADITIONAL:
        layout->addWidget(traditionalLabel,
                          static_cast<int>(rowNumber * 6),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    }

    // Add the second character label (if applicable)
    switch (characterOptions) {
    case EntryCharactersOptions::PREFER_SIMPLIFIED:
        layout->addWidget(traditionalLabel,
                          static_cast<int>(rowNumber * 6 + 1),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryCharactersOptions::PREFER_TRADITIONAL:
        layout->addWidget(simplifiedLabel,
                          static_cast<int>(rowNumber * 6 + 1),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    default:
        break;
    }

    // Add the first phonetic label
    switch (phoneticOptions) {
    case EntryPhoneticOptions::ONLY_JYUTPING:
        pinyinLabel->setVisible(false);
        [[clang::fallthrough]];
    case EntryPhoneticOptions::PREFER_JYUTPING:
        layout->addWidget(jyutpingLabel,
                          static_cast<int>(rowNumber * 6 + 2),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryPhoneticOptions::ONLY_PINYIN:
        jyutpingLabel->setVisible(false);
        [[clang::fallthrough]];
    case EntryPhoneticOptions::PREFER_PINYIN:
        layout->addWidget(pinyinLabel,
                          static_cast<int>(rowNumber * 6 + 2),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    }

    // Add the second phonetic label (if applicable)
    switch (phoneticOptions) {
    case EntryPhoneticOptions::PREFER_JYUTPING:
        layout->addWidget(pinyinLabel,
                          static_cast<int>(rowNumber * 6 + 3),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    case EntryPhoneticOptions::PREFER_PINYIN:
        layout->addWidget(jyutpingLabel,
                          static_cast<int>(rowNumber * 6 + 3),
                          1,
                          1,
                          -1,
                          Qt::AlignTop);
        break;
    default:
        break;
    }

    layout->addWidget(sentenceLabel,
                      static_cast<int>(rowNumber * 6 + 4),
                      1,
                      1,
                      -1,
                      Qt::AlignTop);
}
