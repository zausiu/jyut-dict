#ifndef ENTRYPHONETICOPTIONS_H
#define ENTRYPHONETICOPTIONS_H

#include "logic/utils/qvariantutils.h"

#include <QMetaType>

// An enum that tells how to show the phonetic guide of an entry

enum class EntryPhoneticOptions : int {
    ONLY_JYUTPING,
    ONLY_PINYIN,
    PREFER_JYUTPING,
    PREFER_PINYIN,
};

enum class EntryColourPhoneticType : int {
    JYUTPING,
    PINYIN,
};

enum class CantoneseOptions : int {
    RAW_JYUTPING,
};

enum class MandarinOptions : int {
    RAW_PINYIN,
    PRETTY_PINYIN,
};

Q_DECLARE_METATYPE(EntryPhoneticOptions);
Q_DECLARE_METATYPE(EntryColourPhoneticType);
Q_DECLARE_METATYPE(CantoneseOptions);
Q_DECLARE_METATYPE(MandarinOptions);

#endif // ENTRYPHONETICOPTIONS_H
