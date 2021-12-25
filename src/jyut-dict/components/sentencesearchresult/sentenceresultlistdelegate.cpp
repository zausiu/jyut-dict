#include "sentenceresultlistdelegate.h"

#include "logic/entry/entrycharactersoptions.h"
#include "logic/entry/entryphoneticoptions.h"
#include "logic/sentence/sourcesentence.h"
#include "logic/settings/settingsutils.h"
#include "logic/utils/utils.h"
#include "logic/utils/utils_qt.h"

#include <QGuiApplication>
#include <QAbstractTextDocumentLayout>
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QPainterPath>
#endif
#include <QRectF>
#include <QTextDocument>
#include <QVariant>

SentenceResultListDelegate::SentenceResultListDelegate(QWidget *parent)
    : QStyledItemDelegate (parent)
{
    _settings = Settings::getSettings(this);
}

void SentenceResultListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    // Get the sentence to paint
    if (!index.data().canConvert<SourceSentence>()) {
        return;
    }
    SourceSentence sentence = qvariant_cast<SourceSentence>(index.data());

    painter->save();

    // Draw the rectangle behind each cell in the result view
    QColor backgroundColour;
    if (option.state & QStyle::State_Selected) {
        if (QGuiApplication::applicationState() == Qt::ApplicationInactive) {
            backgroundColour = option.palette
                                   .brush(QPalette::Inactive,
                                          QPalette::Highlight)
                                   .color();
        } else {
            backgroundColour = option.palette
                                   .brush(QPalette::Active,
                                          QPalette::Highlight)
                                   .color();
        }
        painter->fillRect(option.rect, backgroundColour);
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    // Get preferences from user
    EntryCharactersOptions characterOptions;
    EntryPhoneticOptions phoneticOptions;
    CantoneseOptions cantoneseOptions;
    MandarinOptions mandarinOptions;

    characterOptions
        = _settings
              ->value("characterOptions",
                      QVariant::fromValue(
                          EntryCharactersOptions::PREFER_TRADITIONAL))
              .value<EntryCharactersOptions>();
    phoneticOptions = _settings
                          ->value("phoneticOptions",
                                  QVariant::fromValue(
                                      EntryPhoneticOptions::PREFER_JYUTPING))
                          .value<EntryPhoneticOptions>();
    cantoneseOptions = _settings
                          ->value("cantoneseOptions",
                                  QVariant::fromValue(
                                      CantoneseOptions::RAW_JYUTPING))
                          .value<CantoneseOptions>();
    mandarinOptions = _settings
                          ->value("mandarinOptions",
                                  QVariant::fromValue(
                                      MandarinOptions::PRETTY_PINYIN))
                          .value<MandarinOptions>();

    QRect r = option.rect;
    QRect boundingRect;
    QFont font = painter->font();
    QFontMetrics metrics{font};

    // Draw language indicator
    std::string language = sentence.getSourceLanguage();
    QColor colour = Utils::getLanguageColour(language);
    // Adjust a few extra pixels to the right so that the rounded corners of
    // the "pill" look right
    r = r.adjusted(15, 11, 0, 0);

    painter->save();
    font.setPixelSize(12);
    painter->setFont(font);
    metrics = QFontMetrics(font);

    // Paint the first time so we can get the bounding rectangle
    QString languageString = Utils::getLanguageFromISO639(language);
    languageString = metrics
                         .elidedText(languageString, Qt::ElideRight, r.width())
                         .trimmed();
    painter->drawText(r, 0, languageString, &boundingRect);

    // Then draw the actual rounded rectangle that contains the language
    QPainterPath path;
    boundingRect = boundingRect.adjusted(-7, -2, 7, 2);
    path.addRoundedRect(boundingRect, 10, 10);
    if (option.state & QStyle::State_Selected) {
        painter->fillPath(path, backgroundColour.lighter(125));
        painter->setPen(Utils::getContrastingColour(backgroundColour));
    } else {
        painter->fillPath(path, colour);
        painter->setPen(Utils::getContrastingColour(colour));
    }

    painter->drawText(r, 0, languageString, &boundingRect);
    r = r.adjusted(-11, boundingRect.height() + 4, 0, 0);
    painter->restore();

    // Chinese characters
    painter->save();
    QFont oldFont = font;
#ifdef Q_OS_WIN
    font = QFont{"Microsoft Yahei"};
#endif
    font = QFont{"Noto Serif CJK HK"};
    font.setPixelSize(20);
    painter->setFont(font);
    if (option.state &QStyle::State_Selected) {
        painter->setPen(Utils::getContrastingColour(backgroundColour));
    }

    // Use QTextDocument for rich text
    r = option.rect.adjusted(11, 30, -11, 0);
    QTextDocument *doc = new QTextDocument{};
    doc->setHtml(QString(sentence.getCharacters(characterOptions).c_str()));
    doc->setTextWidth(r.width());
    doc->setDefaultFont(font);
    doc->setDocumentMargin(0);
    QAbstractTextDocumentLayout *documentLayout = doc->documentLayout();
    auto ctx = QAbstractTextDocumentLayout::PaintContext();
    ctx.palette.setColor(QPalette::Text, painter->pen().color());
    QRectF bounds = QRectF(0, 0, r.width(), 16);
    ctx.clip = bounds;
    painter->translate(11, r.y());
    documentLayout->draw(painter, ctx);
    painter->translate(-11, -r.y());
    painter->restore();

    delete doc;

    // Phonetic and definition snippets
    font = oldFont;
    QString snippet;
    font.setPixelSize(12);
    painter->setFont(font);
    r = r.adjusted(0, 28, 0, 0);
    metrics = QFontMetrics(font);
    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->setPen(
            Utils::getContrastingColour(backgroundColour).darker(125));
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        painter->setPen(QPen(option.palette.color(QPalette::PlaceholderText)));
#else
        painter->setPen(QPen(option.palette.color(QPalette::Disabled, QPalette::WindowText)));
#endif
    }
    QString phonetic = metrics
                           .elidedText(sentence
                                           .getPhonetic(phoneticOptions,
                                                        cantoneseOptions,
                                                        mandarinOptions)
                                           .c_str(),
                                       Qt::ElideRight,
                                       r.width())
                           .trimmed();
    painter->drawText(r, 0, phonetic, &boundingRect);
    r = r.adjusted(0, boundingRect.height(), 0, 0);
    painter->restore();

    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->setPen(Utils::getContrastingColour(backgroundColour));
    }
#ifdef Q_OS_WIN
    oldFont = font;
    QString snippetLanguage = QString{sentence.getSentenceSnippetLanguage()
            .c_str()}.trimmed();
    if (snippetLanguage == "cmn" || snippetLanguage == "yue") {
        font = QFont{"Microsoft Yahei"};
    } else {
        font = oldFont;
    }
#endif
    font.setPixelSize(12);
    painter->setFont(font);
#ifdef Q_OS_WIN
    if (snippetLanguage == "cmn" || snippetLanguage == "yue") {
        r.setY(
            option.rect.bottom() - 15
            - metrics.boundingRect(sentence.getSentenceSnippet().c_str()).height());
    } else {
        r.setY(
            option.rect.bottom() - 10
            - metrics.boundingRect(sentence.getSentenceSnippet().c_str()).height());
    }
#elif defined(Q_OS_LINUX)
    QString snippetLanguage = QString{sentence.getSentenceSnippetLanguage()
            .c_str()}.trimmed();
    if (snippetLanguage == "cmn" || snippetLanguage == "yue") {
        r.setY(
            option.rect.bottom() - 7
            - metrics.boundingRect(sentence.getSentenceSnippet().c_str()).height());
    } else {
        r.setY(
            option.rect.bottom() - 8
            - metrics.boundingRect(sentence.getSentenceSnippet().c_str()).height());
    }
#else
    r.setY(
        option.rect.bottom() - 10
        - metrics.boundingRect(sentence.getSentenceSnippet().c_str()).height());
#endif
    snippet = metrics
                  .elidedText(sentence.getSentenceSnippet().c_str(),
                              Qt::ElideRight,
                              r.width())
                  .trimmed();
    painter->drawText(r, 0, snippet, &boundingRect);
    r = r.adjusted(0, boundingRect.height(), 0, 0);
    painter->restore();

    // Bottom divider
    QRect rct = option.rect;
    rct.setY(rct.bottom() - 1);
    painter->fillRect(rct, option.palette.alternateBase());

#ifdef Q_OS_WIN
    font = oldFont;
#endif
    painter->restore();
}

QSize SentenceResultListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    (void) (option);

    SourceSentence sentence = qvariant_cast<SourceSentence>(index.data());

    return QSize(100, 100);
}
