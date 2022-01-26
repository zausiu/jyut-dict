#include "entry.h"

#include "logic/settings/settings.h"
#include "logic/utils/chineseutils.h"


Entry::Entry()
    : QObject()
{
    _simplified = "";
    _traditional = "";
    _jyutping = "";
    _pinyin = "";
    _prettyPinyin = "";
    _definitions = {};
    _isWelcome = false;
    _isEmpty = false;
}

Entry::Entry(const std::string &simplified, const std::string &traditional,
             const std::string &jyutping, const std::string &pinyin,
             const std::vector<DefinitionsSet> &definitions)
    : _simplified{simplified},
      _traditional{traditional},
      _jyutping{jyutping},
      _pinyin{pinyin},
      _definitions{definitions}
{
    _isWelcome = false;
    _isEmpty = false;

    // Normalize pinyin and jyutping to lowercase >:(
    std::transform(_jyutping.begin(), _jyutping.end(), _jyutping.begin(), ::tolower);
    std::transform(_pinyin.begin(), _pinyin.end(), _pinyin.begin(), ::tolower);

    // Create comparison versions of traditional and simplified entries
    _traditionalDifference = ChineseUtils::compareStrings(_simplified, _traditional);
    _simplifiedDifference = ChineseUtils::compareStrings(_traditional, _simplified);

    // Create pretty pinyin
    _prettyPinyin = ChineseUtils::createPrettyPinyin(_pinyin);
}

Entry::Entry(const Entry &entry)
    : QObject(),
      _simplified{entry._simplified},
      _simplifiedDifference{entry._simplifiedDifference},
      _traditional{entry._traditional},
      _traditionalDifference{entry._traditionalDifference},
      _colouredSimplified{entry._colouredSimplified},
      _colouredSimplifiedDifference{entry._colouredSimplifiedDifference},
      _colouredTraditional{entry._colouredTraditional},
      _colouredTraditionalDifference{entry._colouredTraditionalDifference},
      _jyutping{entry._jyutping},
      _pinyin{entry._pinyin},
      _prettyPinyin{entry._prettyPinyin},
      _definitions{entry.getDefinitionsSets()},
      _isWelcome{entry.isWelcome()},
      _isEmpty{entry.isEmpty()}
{

}

Entry::Entry(const Entry &&entry)
    : _simplified{std::move(entry._simplified)},
      _simplifiedDifference{std::move(entry._simplifiedDifference)},
      _traditional{std::move(entry._traditional)},
      _traditionalDifference{std::move(entry._traditionalDifference)},
      _colouredSimplified{std::move(entry._colouredSimplified)},
      _colouredSimplifiedDifference{std::move(entry._colouredSimplifiedDifference)},
      _colouredTraditional{std::move(entry._colouredTraditional)},
      _colouredTraditionalDifference{std::move(entry._colouredTraditionalDifference)},
      _jyutping{std::move(entry._jyutping)},
      _pinyin{std::move(entry._pinyin)},
      _prettyPinyin{entry._prettyPinyin},
      _definitions{std::move(entry._definitions)},
      _isWelcome{entry.isWelcome()},
      _isEmpty{entry.isEmpty()}
{

}

Entry &Entry::operator=(const Entry &entry)
{
    if (&entry == this) {
        return *this;
    }

    _simplified = entry._simplified;
    _simplifiedDifference = entry._simplifiedDifference;
    _traditional = entry._traditional;
    _traditionalDifference = entry._traditionalDifference;
    _colouredSimplified = entry._colouredSimplified;
    _colouredSimplifiedDifference = entry._colouredSimplifiedDifference;
    _colouredTraditional = entry._colouredTraditional;
    _colouredTraditionalDifference = entry._colouredTraditionalDifference;
    _jyutping = entry._jyutping;
    _pinyin = entry._pinyin;
    _prettyPinyin = entry._prettyPinyin;
    _definitions = entry.getDefinitionsSets();
    _isWelcome = entry.isWelcome();
    _isEmpty = entry.isEmpty();

    return *this;
}

Entry &Entry::operator=(const Entry &&entry)
{
    if (&entry == this) {
        return *this;
    }

    _simplified = entry._simplified;
    _simplifiedDifference = entry._simplifiedDifference;
    _traditional = entry._traditional;
    _traditionalDifference = entry._traditionalDifference;
    _colouredSimplified = entry._colouredSimplified;
    _colouredSimplifiedDifference = entry._colouredSimplifiedDifference;
    _colouredTraditional = entry._colouredTraditional;
    _colouredTraditionalDifference = entry._colouredTraditionalDifference;
    _jyutping = entry._jyutping;
    _pinyin = entry._pinyin;
    _prettyPinyin = entry._prettyPinyin;
    _definitions = entry.getDefinitionsSets();
    _isWelcome = entry.isWelcome();
    _isEmpty = entry.isEmpty();

    return *this;
}

bool Entry::operator==(const Entry &other) const
{
    if (&other == this) {
        return true;
    }

    if (other._simplified == _simplified && other._traditional == _traditional
        && other._jyutping == _jyutping && other._pinyin == _pinyin) {
        return true;
    }

    return false;
}

bool Entry::operator!=(const Entry &other) const
{
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &out, const Entry &entry)
{
    out << "Simplified: " << entry.getSimplified() << "\n";
    out << "Traditional: " << entry.getTraditional() << "\n";
    out << "Jyutping: " << entry.getJyutping() << "\n";
    out << "Pinyin: " << entry.getPinyin() << "\n";
    for (size_t i = 0; i < entry.getDefinitionsSets().size(); i++) {
        out << entry.getDefinitionsSets()[i] << "\n";
    }
    return out;
}

std::string Entry::getCharacters(EntryCharactersOptions options, bool use_colours) const
{
    switch (options) {
        case (EntryCharactersOptions::ONLY_SIMPLIFIED): {
            if (use_colours) {
                return _colouredSimplified;
            }
            return _simplified;
        }
        case (EntryCharactersOptions::ONLY_TRADITIONAL): {
            if (use_colours) {
                return _colouredTraditional;
            }
            return _traditional;
        }
        case (EntryCharactersOptions::PREFER_SIMPLIFIED): {
            if (use_colours) {
                return _colouredSimplified + " [" + _colouredTraditionalDifference + "]";
            }
            return _simplified + " [" + _traditionalDifference + "]";
        }
        case (EntryCharactersOptions::PREFER_TRADITIONAL): {
            if (use_colours) {
                return _colouredTraditional + " [" + _colouredSimplifiedDifference + "]";
            }
            return _traditional + " [" + _simplifiedDifference + "]";
        }
    }

    return _simplified;
}

std::string Entry::getCharactersNoSecondary(EntryCharactersOptions options, bool use_colours) const
{
    switch (options) {
        case (EntryCharactersOptions::PREFER_SIMPLIFIED):
        case (EntryCharactersOptions::ONLY_SIMPLIFIED):
            if (use_colours) {
                return _colouredSimplified;
            }
            return _simplified;
        case (EntryCharactersOptions::PREFER_TRADITIONAL):
        case (EntryCharactersOptions::ONLY_TRADITIONAL):
            if (use_colours) {
                return _colouredTraditional;
            }
            return _traditional;
    }

    return _simplified;
}

std::string Entry::getSimplified(void) const
{
    return _simplified;
}

void Entry::setSimplified(std::string simplified)
{
    _simplified = simplified;
}

std::string Entry::getTraditional(void) const
{
    return _traditional;
}

void Entry::setTraditional(std::string traditional)
{
    _traditional = traditional;
}

std::string Entry::getPhonetic(EntryPhoneticOptions options) const
{
    return getPhonetic(options, CantoneseOptions::RAW_JYUTPING, MandarinOptions::RAW_PINYIN);
}

std::string Entry::getPhonetic(EntryPhoneticOptions options, MandarinOptions mandarinOptions) const
{
    return getPhonetic(options, CantoneseOptions::RAW_JYUTPING, mandarinOptions);
}

std::string Entry::getPhonetic(EntryPhoneticOptions options, CantoneseOptions cantoneseOptions) const
{
    return getPhonetic(options, cantoneseOptions, MandarinOptions::RAW_PINYIN);
}

std::string Entry::getPhonetic(EntryPhoneticOptions options, CantoneseOptions cantoneseOptions, MandarinOptions mandarinOptions) const
{
    switch (options) {
        case EntryPhoneticOptions::ONLY_JYUTPING: {
            return getCantonesePhonetic(cantoneseOptions);
        }
        case EntryPhoneticOptions::ONLY_PINYIN: {
            return getMandarinPhonetic(mandarinOptions);
        }
        case EntryPhoneticOptions::PREFER_JYUTPING: {
            std::string jyutping = getCantonesePhonetic(cantoneseOptions);
            std::string pinyin = getMandarinPhonetic(mandarinOptions);
            return jyutping + " (" + pinyin + ")";
        }
        case EntryPhoneticOptions::PREFER_PINYIN: {
            std::string jyutping = getCantonesePhonetic(cantoneseOptions);
            std::string pinyin = getMandarinPhonetic(mandarinOptions);
            return pinyin + " (" + jyutping + ")";
        }
    }
    return _jyutping;
}

std::string Entry::getCantonesePhonetic(CantoneseOptions cantoneseOptions) const
{
    switch (cantoneseOptions) {
        case CantoneseOptions::RAW_JYUTPING:
        default:
        return _jyutping;
    }
}

std::string Entry::getMandarinPhonetic(MandarinOptions mandarinOptions) const
{
    switch (mandarinOptions) {
        case MandarinOptions::PRETTY_PINYIN:
            return _prettyPinyin;
        case MandarinOptions::RAW_PINYIN:
        default:
            return _pinyin;
    }
}

std::string Entry::getJyutping(void) const
{
    return _jyutping;
}

void Entry::setJyutping(const std::string &jyutping)
{
    _jyutping = jyutping;
}

std::vector<int> Entry::getJyutpingNumbers() const
{
    std::vector<int> jyutpingNumbers;

    if (_jyutping.empty()) {
        return getPinyinNumbers();
    }

    size_t pos = _jyutping.find_first_of("0123456");
    while(pos != std::string::npos) {
        jyutpingNumbers.push_back(_jyutping.at(pos) - '0');
        pos = _jyutping.find_first_of("0123456", pos + 1);
    }

return jyutpingNumbers;
}

std::string Entry::getPinyin(void) const
{
    return _pinyin;
}

std::string Entry::getPrettyPinyin(void) const
{
    return _prettyPinyin;
}

void Entry::setPinyin(const std::string &pinyin)
{
    _pinyin = pinyin;
}

std::vector<int> Entry::getPinyinNumbers() const
{
    std::vector<int> pinyinNumbers;

    size_t pos = _pinyin.find_first_of("012345");
    while(pos != std::string::npos) {
        pinyinNumbers.push_back(_pinyin.at(pos) - '0');
        pos = _pinyin.find_first_of("012345", pos + 1);
    }

    return pinyinNumbers;
}

std::vector<DefinitionsSet> Entry::getDefinitionsSets(void) const
{
    return _definitions;
}

std::string Entry::getDefinitionSnippet(void) const
{
    if (_definitions.empty()) {
        return "";
    }

    for (auto definition: _definitions) {
        if (!definition.isEmpty()) {
            return definition.getDefinitionsSnippet();
        }
    }

    return "";
}

void Entry::addDefinitions(const std::string &source,
                           const std::vector<Definition::Definition> &definitions)
{
    _definitions.push_back(DefinitionsSet{source, definitions});
}

void Entry::refreshColours(const EntryColourPhoneticType type)
{
    std::vector<int> tones;
    switch (type) {
    case EntryColourPhoneticType::NONE: {
        _colouredSimplified = _simplified;
        _colouredTraditional = _traditional;
        _colouredSimplifiedDifference = _simplifiedDifference;
        _colouredTraditionalDifference = _traditionalDifference;
        return;
    }
    case EntryColourPhoneticType::JYUTPING: {
        tones = getJyutpingNumbers();
        break;
    }
    case EntryColourPhoneticType::PINYIN: {
        tones = getPinyinNumbers();
        break;
    }
    }

    // Create coloured versions of Simplified and Traditional characters
    _colouredSimplified
        = ChineseUtils::applyColours(_simplified,
                                     tones,
                                     Settings::jyutpingToneColours,
                                     Settings::pinyinToneColours,
                                     type);
    _colouredTraditional
        = ChineseUtils::applyColours(_traditional,
                                     tones,
                                     Settings::jyutpingToneColours,
                                     Settings::pinyinToneColours,
                                     type);
    _colouredSimplifiedDifference
        = ChineseUtils::applyColours(_simplifiedDifference,
                                     tones,
                                     Settings::jyutpingToneColours,
                                     Settings::pinyinToneColours,
                                     type);
    _colouredTraditionalDifference
        = ChineseUtils::applyColours(_traditionalDifference,
                                     tones,
                                     Settings::jyutpingToneColours,
                                     Settings::pinyinToneColours,
                                     type);
}

void Entry::setIsWelcome(const bool isWelcome)
{
    _isWelcome = isWelcome;
}

bool Entry::isWelcome(void) const
{
    return _isWelcome;
}

void Entry::setIsEmpty(const bool isEmpty)
{
    _isEmpty = isEmpty;
}

bool Entry::isEmpty(void) const
{
    return _isEmpty;
}
