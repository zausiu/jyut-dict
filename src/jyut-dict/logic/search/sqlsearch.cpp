#include "sqlsearch.h"

#include "logic/database/queryparseutils.h"
#include "logic/search/searchqueries.h"
#include "logic/utils/chineseutils.h"
#include "logic/utils/utils.h"

#include <QtConcurrent/QtConcurrent>

#ifdef Q_OS_WIN
#include <cctype>
#endif

SQLSearch::SQLSearch()
{
    std::random_device rd;
    _generator = std::mt19937_64{rd()};
}

SQLSearch::SQLSearch(std::shared_ptr<SQLDatabaseManager> manager)
    : _manager{manager}
{
    std::random_device rd;
    _generator = std::mt19937_64{rd()};
}

SQLSearch::~SQLSearch()
{
    foreach (auto future, _futures) {
        future.waitForFinished();
    }
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

    std::vector<Entry> results;
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

bool SQLSearch::checkQueryIDCurrent(const unsigned long long queryID) const {
    if (queryID != _queryID) {
        return false;
    }
    return true;
}

void SQLSearch::searchSimplified(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchSimplifiedThread, searchTerm, queryID);
}

void SQLSearch::searchTraditional(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchTraditionalThread, searchTerm, queryID);
}

void SQLSearch::searchJyutping(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchJyutpingThread, searchTerm, queryID);
}

void SQLSearch::searchPinyin(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchPinyinThread, searchTerm, queryID);
}

void SQLSearch::searchEnglish(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchEnglishThread, searchTerm, queryID);
}

void SQLSearch::searchByUnique(const QString &simplified,
                               const QString &traditional,
                               const QString &jyutping,
                               const QString &pinyin)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }

    unsigned long long queryID = generateAndSetQueryID();
    auto future = QtConcurrent::run(&SQLSearch::searchByUniqueThread,
                                    this,
                                    simplified,
                                    traditional,
                                    jyutping,
                                    pinyin,
                                    queryID);
    _futures.append(future);
}

void SQLSearch::searchTraditionalSentences(const QString &searchTerm)
{
    unsigned long long queryID = generateAndSetQueryID();
    runThread(&SQLSearch::searchTraditionalSentencesThread, searchTerm, queryID);
}

void SQLSearch::runThread(void (SQLSearch::*threadFunction)(const QString &searchTerm, const unsigned long long queryID),
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

    auto future = QtConcurrent::run(threadFunction, this, searchTerm, queryID);
    _futures.append(future);
}

// NOTE: If you are modifying these functions, you may also want to modify
// the search functions in SQLUserDataUtils.cpp as well!

// For searching simplified and traditional, we use GLOB, so that wildcard
// characters like * and ? can be used.
void SQLSearch::searchSimplifiedThread(const QString &searchTerm,
                                       const unsigned long long queryID)
{
    // When the search term is surrounded by quotes, search for only term
    // inside quotes (not the quotes themselves)
    bool searchExactMatch
        = ((searchTerm.startsWith("\"") && searchTerm.endsWith("\""))
           || (searchTerm.startsWith("”") && searchTerm.endsWith("“")))
          && searchTerm.length() >= 3;
    bool dontAppendWildcard = searchTerm.endsWith("$");

    QString searchTermWithoutQuotes;
    QString searchTermWithoutEndPositionMarker;
    if (searchExactMatch) {
        searchTermWithoutQuotes = searchTerm.mid(1, searchTerm.size() - 2);
    } else if (dontAppendWildcard) {
        searchTermWithoutEndPositionMarker = searchTerm.chopped(1);
    }

    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_SIMPLIFIED_QUERY);
    if (searchExactMatch) {
        query.addBindValue(searchTermWithoutQuotes);
    } else if (dontAppendWildcard) {
        query.addBindValue(searchTermWithoutEndPositionMarker);
    } else {
        query.addBindValue(searchTerm + "*");
    }
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseEntries(query);

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

void SQLSearch::searchTraditionalThread(const QString &searchTerm,
                                        const unsigned long long queryID)
{
    bool searchExactMatch
        = ((searchTerm.startsWith("\"") && searchTerm.endsWith("\""))
           || (searchTerm.startsWith("“") && searchTerm.endsWith("”")))
          && searchTerm.length() >= 3;
    bool dontAppendWildcard = searchTerm.endsWith("$");

    QString searchTermWithoutQuotes;
    QString searchTermWithoutEndPositionMarker;
    if (searchExactMatch) {
        searchTermWithoutQuotes = searchTerm.mid(1, searchTerm.size() - 2);
    } else if (dontAppendWildcard) {
        searchTermWithoutEndPositionMarker = searchTerm.chopped(1);
    }

    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_TRADITIONAL_QUERY);
    if (searchExactMatch) {
        query.addBindValue(searchTermWithoutQuotes);
    } else if (dontAppendWildcard) {
        query.addBindValue(searchTermWithoutEndPositionMarker);
    } else {
        query.addBindValue(searchTerm + "*");
    }
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseEntries(query);

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// For searching Jyutping and Pinyin, we use GLOB, so that wildcard characters
// like * and ? can be used.
//
// !NOTE! Using QSQLQuery's positional placeholder method automatically
// surrounds the bound value with single quotes, i.e. "'". There is no need
// to add another set of quotes around placeholder values.
void SQLSearch::searchJyutpingThread(const QString &searchTerm,
                                     const unsigned long long queryID)
{
    // When the search term is surrounded by quotes, search for only term
    // inside quotes (not the quotes themselves)
    // Unlike the simplified/traditional search, only trigger exact match
    // searching if enclosed by Western quotes (U+0022).
    bool searchExactMatch = searchTerm.startsWith("\"")
                            && searchTerm.startsWith("\"")
                            && searchTerm.length() >= 3;
    bool dontAppendWildcard = searchTerm.endsWith("$");

    std::vector<std::string> jyutpingWords;
    if (searchExactMatch) {
        QString searchTermWithoutQuotes = searchTerm.mid(1,
                                                         searchTerm.size() - 2);
        Utils::split(searchTermWithoutQuotes.toStdString(), ' ', jyutpingWords);
    } else {
        jyutpingWords
            = ChineseUtils::segmentJyutping(searchTerm,
                                            /* removeSpecialCharacters */ true,
                                            /* removeGlobCharacters */ false);
    }

    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_JYUTPING_QUERY);

    // Don't add wildcard characters to GLOB term if searching for exact match
    const char *globJoinDelimiter = searchExactMatch ? "" : "?";
    std::string globTerm
        = ChineseUtils::constructRomanisationQuery(jyutpingWords,
                                                   globJoinDelimiter);

    query.addBindValue(
        QString{globTerm.c_str()}
        + QString{(searchExactMatch || dontAppendWildcard) ? "" : "*"});
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

void SQLSearch::searchPinyinThread(const QString &searchTerm,
                                   const unsigned long long queryID)
{
    // Replace "v" and "ü" with "u:" since "ü" is stored as "u:" in the table
    QString processedSearchTerm = searchTerm;
    long long location = processedSearchTerm.indexOf("v");
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

    bool searchExactMatch = processedSearchTerm.startsWith("\"")
                            && processedSearchTerm.endsWith("\"")
                            && processedSearchTerm.length() >= 3;
    bool dontAppendWildcard = searchTerm.endsWith("$");

    std::vector<std::string> pinyinWords;
    if (searchExactMatch) {
        QString searchTermWithoutQuotes
            = processedSearchTerm.mid(1, processedSearchTerm.size() - 2);
        Utils::split(searchTermWithoutQuotes.toStdString(), ' ', pinyinWords);
    } else {
        pinyinWords
            = ChineseUtils::segmentPinyin(processedSearchTerm,
                                          /* removeSpecialCharacters */ true,
                                          /* removeGlobCharacters */ false);
    }

    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_PINYIN_QUERY);

    // Don't add wildcard characters to GLOB term if searching for exact match
    const char *globJoinDelimiter = searchExactMatch ? "" : "?";
    std::string globTerm
        = ChineseUtils::constructRomanisationQuery(pinyinWords,
                                                   globJoinDelimiter);

    query.addBindValue(
        QString{globTerm.c_str()}
        + QString{(searchExactMatch || dontAppendWildcard) ? "" : "*"});
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) { return; }
    results = QueryParseUtils::parseEntries(query);

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

void SQLSearch::searchEnglishThread(const QString &searchTerm,
                                    const unsigned long long queryID)
{
    bool searchExactMatch = searchTerm.startsWith("\"")
                            && searchTerm.endsWith("\"")
                            && searchTerm.length() >= 3;
    QString searchTermWithoutQuotes;
    if (searchExactMatch) {
        searchTermWithoutQuotes = searchTerm.mid(1, searchTerm.size() - 2);
    }

    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_ENGLISH_QUERY);
    if (searchExactMatch) {
        query.addBindValue("\"" + searchTermWithoutQuotes + "\"");
        query.addBindValue(searchTermWithoutQuotes);
    } else {
        query.addBindValue("\"" + searchTerm + "\"");
        query.addBindValue("%" + searchTerm + "%");
    }
    query.setForwardOnly(true);
    query.exec();

    // Do not parse results if new query has been made
    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseEntries(query);

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// To seach by unique, select by all the attributes that we have.
void SQLSearch::searchByUniqueThread(const QString &simplified,
                                     const QString &traditional,
                                     const QString &jyutping,
                                     const QString &pinyin,
                                     const unsigned long long queryID)
{
    std::vector<Entry> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_UNIQUE_QUERY);
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

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}

// To search for sentences, use the sentence_links table to JOIN
// between the chinese and non_chinese_sentences tables.
void SQLSearch::searchTraditionalSentencesThread(const QString &searchTerm,
                                                 const unsigned long long queryID)
{
    std::vector<SourceSentence> results;

    QSqlQuery query{_manager->getDatabase()};
    query.prepare(SEARCH_TRADITIONAL_SENTENCES_QUERY);
    query.addBindValue("%" + searchTerm + "%");
    query.exec();

    if (!checkQueryIDCurrent(queryID)) {
        return;
    }
    results = QueryParseUtils::parseSentences(query);

    if (!checkQueryIDCurrent(queryID)) { return; }
    notifyObserversIfQueryIdCurrent(results, /*emptyQuery=*/false, queryID);
}
