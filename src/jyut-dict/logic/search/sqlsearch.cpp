#include "sqlsearch.h"

#include "logic/database/queryparseutils.h"
#include "logic/utils/chineseutils.h"
#include "logic/utils/utils.h"

#include <QtConcurrent/QtConcurrent>

#ifdef Q_OS_WIN
#include <cctype>
#endif
#include <functional>
#include <sstream>
#include <unordered_set>

SQLSearch::SQLSearch()
    : QObject()
{
    std::random_device rd;
    _generator = std::mt19937_64{rd()};
}

SQLSearch::SQLSearch(std::shared_ptr<SQLDatabaseManager> manager)
    : QObject()
    , _manager{manager}
{
    std::random_device rd;
    _generator = std::mt19937_64{rd()};
}

SQLSearch::SQLSearch(const SQLSearch &search)
    : QObject()
{
    if (search._manager != nullptr) {
        _manager = search._manager;
    }

    std::random_device rd;
    _generator = std::mt19937_64{rd()};
}

SQLSearch::~SQLSearch()
{

}

void SQLSearch::registerObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.push_back(observer);
}

void SQLSearch::deregisterObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.remove(observer);
}

// Do not call this function without first acquiring the _notifyMutex!
void SQLSearch::notifyObservers(const std::vector<Entry> &results, bool emptyQuery)
{
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(results, emptyQuery);
        ++it;
    }
}

// Do not call this function without first acquiring the _notifyMutex!
void SQLSearch::notifyObservers(const std::vector<SourceSentence> &results,
                                bool emptyQuery)
{
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(results, emptyQuery);
        ++it;
    }
}

// This version assumes an empty result set
void SQLSearch::notifyObserversOfEmptySet(bool emptyQuery,
                                          const unsigned long long queryID)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    if (queryID != _queryID) {
        return;
    }

    std::vector<Entry> results{};
    notifyObservers(results, emptyQuery);
}

void SQLSearch::notifyObserversIfQueryIdCurrent(const std::vector<Entry> &results,
                                                bool emptyQuery,
                                                const unsigned long long queryID)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    if (queryID != _queryID) {
        return;
    }

    notifyObservers(results, emptyQuery);
}

void SQLSearch::notifyObserversIfQueryIdCurrent(const std::vector<SourceSentence> &results,
                                                bool emptyQuery,
                                                const unsigned long long queryID)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    if (queryID != _queryID) {
        return;
    }

    notifyObservers(results, emptyQuery);
}

unsigned long long SQLSearch::generateAndSetQueryID(void) {
    unsigned long long queryID = _dist(_generator);
    _queryID = queryID;
    return queryID;
}

bool SQLSearch::checkQueryIDCurrent(const unsigned long long queryID) {
    if (queryID != _queryID) {
        return false;
    }
    return true;
}

void SQLSearch::searchSimplified(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchSimplifiedThread, searchTerm, queryID);
}

void SQLSearch::searchTraditional(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchTraditionalThread, searchTerm, queryID);
}

void SQLSearch::searchJyutping(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchJyutpingThread, searchTerm, queryID);
}

void SQLSearch::searchPinyin(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchPinyinThread, searchTerm, queryID);
}

void SQLSearch::searchEnglish(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchEnglishThread, searchTerm, queryID);
}

void SQLSearch::searchByUnique(const QString simplified,
                               const QString traditional,
                               const QString jyutping,
                               const QString pinyin)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }

    unsigned long long queryID = generateAndSetQueryID();
    QtConcurrent::run(this,
                      &SQLSearch::searchByUniqueThread,
                      simplified,
                      traditional,
                      jyutping,
                      pinyin,
                      queryID);
}

void SQLSearch::searchTraditionalSentences(const QString searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchTraditionalSentencesThread, searchTerm, queryID);
}

void SQLSearch::runThread(void (SQLSearch::*threadFunction)(const QString searchTerm, const unsigned long long queryID),
                          const QString &searchTerm, const unsigned long long queryID)
{
    if (searchTerm.isEmpty()) {
        notifyObserversOfEmptySet(true, queryID);
        return;
    }

    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }

    QtConcurrent::run(this, threadFunction, searchTerm, queryID);
}

// For SearchSimplified and SearchTraditional, we use LIKE instead of MATCH
// even though the database is FTS5-compatible.
// This is because FTS searches using the space as a separator, and
// Chinese words and phrases are not separated by spaces.
void SQLSearch::searchSimplifiedThread(const QString searchTerm,
                                       const unsigned long long queryID)
{
    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
        "SELECT traditional, simplified, pinyin, jyutping, "
        "group_concat(sourcename || ' ' || definition, '●') AS definitions "
        "FROM entries, definitions, sources "
        "WHERE simplified LIKE ? "
        "AND entry_id = fk_entry_id "
        "AND source_id = fk_source_id "
        "GROUP BY entry_id "
        "ORDER BY frequency DESC");
    query.addBindValue(searchTerm + "%");
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

void SQLSearch::searchTraditionalThread(const QString searchTerm,
                                        const unsigned long long queryID)
{
    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
        "SELECT traditional, simplified, pinyin, jyutping, "
        "group_concat(sourcename || ' ' || definition, '●') AS definitions "
        "FROM entries, definitions, sources "
        "WHERE traditional LIKE ? "
        "AND entry_id = fk_entry_id "
        "AND source_id = fk_source_id "
        "GROUP BY entry_id "
        "ORDER BY frequency DESC");
    query.addBindValue(searchTerm + "%");
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// For searching jyutping and pinyin, we use MATCH and then LIKE, in order
// to take advantage of the quick full-text-search matching, before then
// filtering the results to only those that begin with the query
// using a LIKE wildcard.
//
// This approach is approximately ten times faster than simply using the LIKE
// operator and the % wildcard.
//
// !NOTE! Using QSQLQuery's positional placeholder method automatically
// surrounds the bound value with single quotes, i.e. "'". There is no need
// to add another set of quotes around placeholder values.
void SQLSearch::searchJyutpingThread(const QString searchTerm,
                                     const unsigned long long queryID)
{
    std::vector<std::string> jyutpingWords = ChineseUtils::segmentJyutping(
        searchTerm);

    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
        "SELECT traditional, simplified, pinyin, jyutping, "
        "group_concat(sourcename || ' ' || definition, '●') AS definitions "
        "FROM entries, definitions, sources "
        "WHERE entry_id IN "
        "(SELECT rowid FROM entries_fts WHERE entries_fts MATCH ?"
        " AND jyutping LIKE ?) "
        "AND entry_id = fk_entry_id "
        "AND source_id = fk_source_id "
        "GROUP BY entry_id "
        "ORDER BY frequency DESC");
    const char *matchJoinDelimiter = "*";
    std::string matchTerm
        = ChineseUtils::constructRomanisationQuery(jyutpingWords,
                                                   matchJoinDelimiter,
                                                   /*surroundWithQuotes=*/true);
    const char *likeJoinDelimiter = "_";
    std::string likeTerm
        = ChineseUtils::constructRomanisationQuery(jyutpingWords,
                                                   likeJoinDelimiter);
    query.addBindValue("jyutping:" + QString(matchTerm.c_str()));
    query.addBindValue(QString(likeTerm.c_str()) + "%");
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

void SQLSearch::searchPinyinThread(const QString searchTerm,
                                   const unsigned long long queryID)
{
    // Replace "v" and "ü" with "u:" since "ü" is stored as "u:" in the table
    QString processedSearchTerm = searchTerm;
    int location = processedSearchTerm.indexOf("v");
    while (location != -1) {
        processedSearchTerm.remove(location, 1);
        processedSearchTerm.insert(location, "u:");
        location = processedSearchTerm.indexOf("v", location);
    }

    location = processedSearchTerm.indexOf("ü");
    while (location != -1) {
        processedSearchTerm.remove(location, 1);
        processedSearchTerm.insert(location, "u:");
        location = processedSearchTerm.indexOf("ü", location);
    }

    std::vector<std::string> pinyinWords = ChineseUtils::segmentPinyin(
        searchTerm);

    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare("SELECT traditional, simplified, pinyin, jyutping, "
                  "group_concat(sourcename || ' ' || definition, '●') AS definitions "
                  "FROM entries, definitions, sources "
                  "WHERE entry_id IN "
                  "(SELECT rowid FROM entries_fts WHERE entries_fts MATCH ?"
                  " AND pinyin LIKE ?) "
                  "AND entry_id = fk_entry_id "
                  "AND source_id = fk_source_id "
                  "GROUP BY entry_id "
                  "ORDER BY frequency DESC");
    const char *matchJoinDelimiter = "*";
    std::string matchTerm
        = ChineseUtils::constructRomanisationQuery(pinyinWords,
                                                   matchJoinDelimiter,
                                                   /*surroundWithQuotes=*/true);
    const char *likeJoinDelimiter = "_";
    std::string likeTerm
        = ChineseUtils::constructRomanisationQuery(pinyinWords,
                                                   likeJoinDelimiter,
                                                   /*surroundWithQuotes=*/false);
    query.addBindValue("pinyin:" + QString{matchTerm.c_str()});
    query.addBindValue(QString(likeTerm.c_str()) + "%");
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// Searching English is the most complicated query.
// First, we match against the full-text columns in definitions_fts,
// and extract the foreign key that corresponds to the entry that matches
// that definition.
// After that, we select all the definitions that match those entries,
// along with the source that accompanies that entry,
// grouping by the entry ID and merging all the definitions into one column.
//
// For some reason, using two subqueries is ~2-3x faster than doing two
// INNER JOINs. This is related in some way to the fk_entry_id index, but I'm
// not entirely sure why.
void SQLSearch::searchEnglishThread(const QString searchTerm,
                                    const unsigned long long queryID)
{
    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
                //// Get list of entry ids where at least one definition matches the query
                "WITH matching_entry_ids AS ( "
                "  SELECT fk_entry_id FROM definitions_fts WHERE definitions_fts MATCH ? "
                "), "
                " "
                "matching_definition_ids AS ( "
                "  SELECT definition_id, definition FROM definitions WHERE fk_entry_id IN matching_entry_ids "
                "), "
                " "
                //// Get corresponding sentence ids for each of those definitions
                //// This CTE is used twice; would be nice if could materialize it
                "matching_chinese_sentence_ids AS ( "
                "  SELECT definition_id, fk_chinese_sentence_id "
                "  FROM matching_definition_ids AS mdi "
                "  JOIN definitions_chinese_sentences_links AS dcsl ON mdi.definition_id = dcsl.fk_definition_id "
                "), "
                " "
                //// Get translations for each of those sentences
                "matching_translations AS ( "
                "  SELECT mcsi.fk_chinese_sentence_id, "
                "    json_group_array(DISTINCT "
                "      json_object('sentence', sentence, "
                "                  'language', language, "
                "                  'direct', direct "
                "    )) AS translation "
                "  FROM matching_chinese_sentence_ids AS mcsi "
                "  JOIN sentence_links AS sl ON mcsi.fk_chinese_sentence_id = sl.fk_chinese_sentence_id "
                "  JOIN nonchinese_sentences AS ncs ON ncs.non_chinese_sentence_id = sl.fk_non_chinese_sentence_id "
                "  GROUP BY mcsi.fk_chinese_sentence_id "
                "), "
                " "
                //// Create sentence object for each sentence
                "matching_sentences AS ( "
                "  SELECT chinese_sentence_id, "
                "    json_object('traditional', traditional, "
                "                'simplified', simplified, "
                "                'pinyin', pinyin, "
                "                'jyutping', jyutping, "
                "                'language', language, "
                "                'translations', json(translation)) AS sentence "
                "  FROM chinese_sentences AS cs "
                "  LEFT JOIN matching_translations AS mt ON cs.chinese_sentence_id = mt.fk_chinese_sentence_id "
                "), "
                " "
                //// Create definition object for each definition
                "matching_definitions AS ( "
                "  SELECT definitions.fk_entry_id, definitions.fk_source_id, "
                "    json_object('definition', definitions.definition, "
                "                'label', label, 'sentences', "
                "                json_group_array(json(sentence))) AS definition "
                "  FROM matching_definition_ids AS mdi "
                "  LEFT JOIN definitions ON mdi.definition_id = definitions.definition_id "
                "  LEFT JOIN matching_chinese_sentence_ids AS mcsi ON mdi.definition_id = mcsi.definition_id "
                "  LEFT JOIN matching_sentences AS ms ON mcsi.fk_chinese_sentence_id = ms.chinese_sentence_id "
                "  GROUP BY mdi.definition_id "
                "), "
                " "
//                //// Create definition groups for definitions of the same entry that come from the same source
                "matching_definition_groups AS ( "
                "  SELECT fk_entry_id, "
                "    json_object('source', sourcename, "
                "                'definitions', "
                "                json_group_array(json(definition))) AS definitions "
                "  FROM matching_definitions AS md "
                "  LEFT JOIN sources ON sources.source_id = md.fk_source_id "
                "  GROUP BY fk_entry_id, fk_source_id "
                "), "
                " "
                //// Construct the final entry object
                "matching_entries AS ( "
                "  SELECT traditional, simplified, jyutping, pinyin, json_group_array(json(definitions)) AS definitions "
                "  FROM matching_definition_groups AS mdg "
                "  LEFT JOIN entries ON entries.entry_id = mdg.fk_entry_id "
                "  GROUP BY entry_id "
                "  ORDER BY frequency DESC "
                ") "
                " "
                "SELECT simplified, traditional, jyutping, pinyin, definitions from matching_entries"
    );
    query.addBindValue("\"" + searchTerm + "\"");
    query.setForwardOnly(true);
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// To seach by unique, select by all the attributes that we have.
void SQLSearch::searchByUniqueThread(const QString simplified,
                                     const QString traditional,
                                     const QString jyutping,
                                     const QString pinyin,
                                     const unsigned long long queryID)
{
    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
        "SELECT traditional, simplified, pinyin, jyutping, "
        "group_concat(sourcename || ' ' || definition, '●') AS definitions "
        "FROM entries, definitions, sources "
        "WHERE simplified LIKE ? "
        "AND traditional LIKE ? "
        "AND jyutping LIKE ? "
        "AND pinyin LIKE ? "
        "AND entry_id = fk_entry_id "
        "AND source_id = fk_source_id "
        "GROUP BY entry_id "
        "ORDER BY frequency DESC");
    query.addBindValue(simplified);
    query.addBindValue(traditional);
    query.addBindValue(jyutping);
    query.addBindValue(pinyin);
    query.setForwardOnly(true);
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// To search for sentences, use the sentence_links table to JOIN
// between the chinese and non_chinese_sentences tables.
void SQLSearch::searchTraditionalSentencesThread(const QString searchTerm,
                                                 const unsigned long long queryID)
{
    std::vector<SourceSentence> results{};

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(
        "SELECT chinese_sentence_id, traditional, simplified, pinyin, "
        " jyutping, chinese_sentences.language AS source_language, "
        " group_concat(sourcename || ' ' || nonchinese_sentences.language "
        "|| ' ' || direct || ' ' || sentence, '●') AS sentences "
        "FROM chinese_sentences, sentence_links, sources "
        "JOIN nonchinese_sentences "
        "ON sentence_links.fk_chinese_sentence_id = "
        "chinese_sentences.chinese_sentence_id "
        "AND sentence_links.fk_non_chinese_sentence_id = "
        "nonchinese_sentences.non_chinese_sentence_id "
        "AND sentence_links.fk_source_id = sources.source_id "
        "AND traditional LIKE ? "
        "GROUP BY chinese_sentences.chinese_sentence_id");
    query.addBindValue("%" + searchTerm + "%");
    query.exec();

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseSentences(query);
    _manager->closeDatabase();

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}
