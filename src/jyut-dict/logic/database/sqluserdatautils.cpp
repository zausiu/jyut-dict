#include "sqluserdatautils.h"

#include "logic/database/queryparseutils.h"
#include "logic/utils/utils.h"

#include <QtConcurrent/QtConcurrent>

SQLUserDataUtils::SQLUserDataUtils(std::shared_ptr<SQLDatabaseManager> manager)
    : _manager{manager}
{
}

SQLUserDataUtils::~SQLUserDataUtils() {}

void SQLUserDataUtils::registerObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.push_back(observer);
}

void SQLUserDataUtils::deregisterObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.remove(observer);
}

void SQLUserDataUtils::notifyObservers(const std::vector<Entry> &results,
                                       bool emptyQuery)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(results, emptyQuery);
        ++it;
    }
}

void SQLUserDataUtils::notifyObservers(bool entryExists, Entry entry)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(entryExists, entry);
        ++it;
    }
}

void SQLUserDataUtils::searchForAllFavouritedWords(void)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserDataUtils::searchForAllFavouritedWordsThread);
}

void SQLUserDataUtils::checkIfEntryHasBeenFavourited(Entry entry)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserDataUtils::checkIfEntryHasBeenFavouritedThread,
                      entry);
}

void SQLUserDataUtils::favouriteEntry(Entry entry)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserDataUtils::favouriteEntryThread,
                      entry);
}

void SQLUserDataUtils::unfavouriteEntry(Entry entry)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserDataUtils::unfavouriteEntryThread,
                      entry);
}

void SQLUserDataUtils::searchForAllFavouritedWordsThread(void)
{
    std::vector<Entry> results{};

    QSqlQuery query{_manager->getDatabase()};

    // Select the fields of the entry and its definitions from the entry_id
    // from the temporary table, then sort by the timestamp.
    query.exec(
        //// Get list of entry ids whose simplified form matches the query
        //// This CTE is used twice; would be nice if could materialize it
        "WITH matching_entry_ids AS ( "
        "  SELECT entry_id, timestamp from entries "
        "  INNER JOIN user.favourite_words "
        "    ON entries.simplified = favourite_words.simplified "
        "    AND entries.traditional = favourite_words.traditional "
        "    AND entries.jyutping = favourite_words.jyutping "
        "    AND entries.pinyin = favourite_words.pinyin "
        "), "
        " "
        //// Get the list of all definitions for those entries
        //// This CTE is used twice; would be nice if could materialize it
        "matching_definition_ids AS ( "
        "  SELECT definition_id, definition FROM definitions WHERE fk_entry_id IN ( "
        "       SELECT entry_id FROM matching_entry_ids "
        "  ) "
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
        //// Create definition groups for definitions of the same entry that come from the same source
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
        "  SELECT simplified, traditional, jyutping, pinyin, json_group_array(json(definitions)) AS definitions "
        "  FROM matching_definition_groups AS mdg "
        "  LEFT JOIN entries ON entries.entry_id = mdg.fk_entry_id "
        "  LEFT JOIN matching_entry_ids AS mei ON entries.entry_id = mei.entry_id "
        "  GROUP BY entries.entry_id "
        "  ORDER BY timestamp DESC "
        ") "
        " "
        "SELECT simplified, traditional, jyutping, pinyin, definitions from matching_entries"
    );

    results = QueryParseUtils::parseEntries(query);
    _manager->closeDatabase();

    notifyObservers(results, /*emptyQuery=*/false);
}

void SQLUserDataUtils::checkIfEntryHasBeenFavouritedThread(Entry entry)
{
    bool existence = false;
    QSqlQuery query{_manager->getDatabase()};

    query.prepare(
        "SELECT EXISTS (SELECT 1 FROM user.favourite_words WHERE "
        "traditional=? AND simplified=? AND jyutping=? AND pinyin=?) "
        "AS existence");
    query.addBindValue(entry.getTraditional().c_str());
    query.addBindValue(entry.getSimplified().c_str());
    query.addBindValue(entry.getJyutping().c_str());
    query.addBindValue(entry.getPinyin().c_str());
    query.exec();
    existence = QueryParseUtils::parseExistence(query);
    _manager->closeDatabase();

    notifyObservers(existence, entry);
}

void SQLUserDataUtils::favouriteEntryThread(Entry entry)
{
    QSqlQuery query{_manager->getDatabase()};

    query.prepare(
        "INSERT INTO user.favourite_words(traditional, simplified, "
        " jyutping, pinyin, fk_list_id, timestamp) "
        "values(?, ?, ?, ?, 1, datetime(\"now\")) ");
    query.addBindValue(entry.getTraditional().c_str());
    query.addBindValue(entry.getSimplified().c_str());
    query.addBindValue(entry.getJyutping().c_str());
    query.addBindValue(entry.getPinyin().c_str());
    query.exec();
    query.exec("COMMIT");
    _manager->closeDatabase();

    checkIfEntryHasBeenFavourited(entry);
    searchForAllFavouritedWords();
}

void SQLUserDataUtils::unfavouriteEntryThread(Entry entry)
{
    QSqlQuery query{_manager->getDatabase()};

    query.prepare(
        "DELETE FROM user.favourite_words WHERE "
        "traditional=? AND simplified=? AND jyutping=? AND pinyin=?");
    query.addBindValue(entry.getTraditional().c_str());
    query.addBindValue(entry.getSimplified().c_str());
    query.addBindValue(entry.getJyutping().c_str());
    query.addBindValue(entry.getPinyin().c_str());
    query.exec();
    query.exec("COMMIT");
    _manager->closeDatabase();

    checkIfEntryHasBeenFavourited(entry);
    searchForAllFavouritedWords();
}
