#include "sourcesentence.h"

#include "logic/utils/chineseutils.h"

SourceSentence::SourceSentence()
{
}

SourceSentence::SourceSentence(const std::string &sourceLanguage,
                               const std::string &simplified,
                               const std::string &traditional,
                               const std::string &jyutping,
                               const std::string &pinyin,
                               const std::vector<SentenceSet> &sentences)
    : _sourceLanguage{sourceLanguage}
    , _simplified{simplified}
    , _traditional{traditional}
    , _jyutping{jyutping}
    , _pinyin{pinyin}
    , _sentences{sentences}
{
    // Create pretty pinyin
    _prettyPinyin = ChineseUtils::createPrettyPinyin(_pinyin);
}

std::ostream &operator<<(std::ostream &out, const SourceSentence &sourceSentence)
{
    out << "Simplified: " << sourceSentence.getSimplified() << "\n";
    out << "Traditional: " << sourceSentence.getTraditional() << "\n";
    out << "Jyutping: " << sourceSentence.getJyutping() << "\n";
    out << "Pinyin: " << sourceSentence.getPinyin() << "\n";
    for (size_t i = 0; i < sourceSentence.getSentenceSets().size(); i++) {
        out << sourceSentence.getSentenceSets()[i] << "\n";
    }
    return out;
}

std::string SourceSentence::getSourceLanguage(void) const
{
    return _sourceLanguage;
}

void SourceSentence::setSourceLanguage(std::string sourceLanguage)
{
    _sourceLanguage = sourceLanguage;
}

std::string SourceSentence::getCharacters(EntryCharactersOptions options) const
{
    switch (options) {
    case EntryCharactersOptions::ONLY_SIMPLIFIED:
    case EntryCharactersOptions::PREFER_SIMPLIFIED:
        return _simplified;
    case EntryCharactersOptions::ONLY_TRADITIONAL:
    case EntryCharactersOptions::PREFER_TRADITIONAL:
        return _traditional;
    }
    return _traditional;
}

std::string SourceSentence::getSimplified(void) const
{
    return _simplified;
}

void SourceSentence::setSimplified(std::string simplified)
{
    _simplified = simplified;
}

std::string SourceSentence::getTraditional(void) const
{
    return _traditional;
}

void SourceSentence::setTraditional(std::string traditional)
{
    _traditional = traditional;
}

std::string SourceSentence::getPhonetic(EntryPhoneticOptions options,
                                        CantoneseOptions cantoneseOptions,
                                        MandarinOptions mandarinOptions) const
{
    switch (options) {
    case EntryPhoneticOptions::ONLY_CANTONESE:
    case EntryPhoneticOptions::PREFER_CANTONESE:
        return getCantonesePhonetic(cantoneseOptions);
    case EntryPhoneticOptions::ONLY_MANDARIN:
    case EntryPhoneticOptions::PREFER_MANDARIN:
        return getMandarinPhonetic(mandarinOptions);
    }
    return getCantonesePhonetic(cantoneseOptions);
}

std::string SourceSentence::getCantonesePhonetic(
    CantoneseOptions cantoneseOptions) const
{
    switch (cantoneseOptions) {
    case CantoneseOptions::RAW_JYUTPING:
    default:
        return _jyutping;
    }
}

std::string SourceSentence::getMandarinPhonetic(
    MandarinOptions mandarinOptions) const
{
    switch (mandarinOptions) {
    case MandarinOptions::PRETTY_PINYIN:
        return _prettyPinyin;
    case MandarinOptions::RAW_PINYIN:
    default:
        return _pinyin;
    }
}

std::string SourceSentence::getJyutping(void) const
{
    return _jyutping;
}

void SourceSentence::setJyutping(const std::string &jyutping)
{
    _jyutping = jyutping;
}

std::string SourceSentence::getPinyin(void) const
{
    return _pinyin;
}

std::string SourceSentence::getPrettyPinyin(void) const
{
    return _prettyPinyin;
}

void SourceSentence::setPinyin(const std::string &pinyin)
{
    _pinyin = pinyin;
}

std::vector<SentenceSet> SourceSentence::getSentenceSets(void) const
{
    return _sentences;
}

std::string SourceSentence::getSentenceSnippet(void) const
{
    if (_sentences.empty()) {
        return "";
    }

    SentenceSet sentenceSet = _sentences.at(0);

    if (sentenceSet.getSentenceSnippet().empty()) {
        return "";
    }

    std::vector<Sentence::TargetSentence> snippets = sentenceSet
                                                         .getSentenceSnippet();

    if (snippets.empty()) {
        return "";
    }

    return snippets.at(0).sentence;
}

std::string SourceSentence::getSentenceSnippetLanguage(void) const
{
    if (_sentences.empty()) {
        return "";
    }

    SentenceSet sentenceSet = _sentences.at(0);

    if (sentenceSet.getSentenceSnippet().empty()) {
        return "";
    }

    std::vector<Sentence::TargetSentence> snippets = sentenceSet
                                                         .getSentenceSnippet();

    if (snippets.empty()) {
        return "";
    }

    return snippets.at(0).language;
}

void SourceSentence::setIsWelcome(const bool isWelcome)
{
    _isWelcome = isWelcome;
}

bool SourceSentence::isWelcome(void) const
{
    return _isWelcome;
}

void SourceSentence::setIsEmpty(const bool isEmpty)
{
    _isEmpty = isEmpty;
}

bool SourceSentence::isEmpty(void) const
{
    return _isEmpty;
}
