#include "sqluserhistoryutils.h"

#include "logic/database/queryparseutils.h"
#include "logic/utils/utils.h"

#include <QtConcurrent/QtConcurrent>

SQLUserHistoryUtils::SQLUserHistoryUtils(std::shared_ptr<SQLDatabaseManager> manager)
    : _manager{manager}
{
}

SQLUserHistoryUtils::~SQLUserHistoryUtils() {}

void SQLUserHistoryUtils::registerObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.push_back(observer);
}

void SQLUserHistoryUtils::deregisterObserver(ISearchObserver *observer)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    _observers.remove(observer);
}

void SQLUserHistoryUtils::notifyObservers(const std::vector<searchTermHistoryItem> &results,
                                          bool emptyQuery)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(results, emptyQuery);
        ++it;
    }
}

void SQLUserHistoryUtils::notifyObservers(const std::vector<Entry> &results,
                                          bool emptyQuery)
{
    std::lock_guard<std::mutex> notifyLock{_notifyMutex};
    std::list<ISearchObserver *>::const_iterator it = _observers.begin();
    while (it != _observers.end()) {
        (static_cast<ISearchObserver *>(*it))->callback(results, emptyQuery);
        ++it;
    }
}

bool SQLUserHistoryUtils::checkForManager(void)
{
    if (!_manager) {
        std::cout << "No database specified!" << std::endl;
        return false;
    }
    return true;
}

void SQLUserHistoryUtils::addSearchToHistory(std::string search, int options)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserHistoryUtils::addSearchToHistoryThread,
                      search,
                      options);
}

void SQLUserHistoryUtils::addViewToHistory(Entry entry)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserHistoryUtils::addViewToHistoryThread,
                      entry);
}

void SQLUserHistoryUtils::searchAllSearchHistory(void)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this,
                      &SQLUserHistoryUtils::searchAllSearchHistoryThread);
}

void SQLUserHistoryUtils::clearAllSearchHistory(void)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this, &SQLUserHistoryUtils::clearAllSearchHistoryThread);
}

void SQLUserHistoryUtils::searchAllViewHistory(void)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this, &SQLUserHistoryUtils::searchAllViewHistoryThread);
}

void SQLUserHistoryUtils::clearAllViewHistory(void)
{
    if (!checkForManager()) {
        return;
    }
    QtConcurrent::run(this, &SQLUserHistoryUtils::clearAllViewHistoryThread);
}

void SQLUserHistoryUtils::addSearchToHistoryThread(std::string search,
                                                   int options)
{
    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.prepare("INSERT INTO user.search_history "
                      " (search_text, search_options, timestamp) "
                      "VALUES "
                      " (?, ?, datetime(\"now\"))");
        query.addBindValue(search.c_str());
        query.addBindValue(QVariant::fromValue(options));
        query.exec();

        _manager->closeDatabase();
    }
}

void SQLUserHistoryUtils::addViewToHistoryThread(Entry entry)
{
    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.prepare("INSERT INTO user.view_history "
                      " (traditional, simplified, jyutping, pinyin, timestamp) "
                      "VALUES "
                      " (?, ?, ?, ?, datetime(\"now\"))");
        query.addBindValue(entry.getTraditional().c_str());
        query.addBindValue(entry.getSimplified().c_str());
        query.addBindValue(entry.getJyutping().c_str());
        query.addBindValue(entry.getPinyin().c_str());
        query.exec();

        _manager->closeDatabase();
    }
}

void SQLUserHistoryUtils::searchAllSearchHistoryThread(void)
{
    std::vector<searchTermHistoryItem> results{};

    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.exec(
            "SELECT search_text AS text, search_options AS options, timestamp "
            "FROM user.search_history "
            "ORDER BY timestamp DESC "
            "LIMIT 1000");

        results = QueryParseUtils::parseHistoryItems(query);
        _manager->closeDatabase();
    }

    notifyObservers(results, /*emptyQuery=*/false);
}

void SQLUserHistoryUtils::clearAllSearchHistoryThread(void)
{
    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.exec("DELETE FROM user.search_history");

        _manager->closeDatabase();
    }

    searchAllSearchHistory();
}

void SQLUserHistoryUtils::searchAllViewHistoryThread(void)
{
    std::vector<Entry> results{};

    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.exec(
            "SELECT traditional, simplified, pinyin, jyutping, timestamp "
            "FROM user.view_history "
            "ORDER BY timestamp DESC "
            "LIMIT 1000");

        results = QueryParseUtils::parseEntries(query, /*parseDefinitions=*/false);
        _manager->closeDatabase();
    }

    notifyObservers(results, /*emptyQuery=*/false);
}

void SQLUserHistoryUtils::clearAllViewHistoryThread(void)
{
    {
        std::lock_guard<std::mutex> databaseLock(_databaseMutex);
        QSqlQuery query{_manager->getDatabase()};

        query.exec("DELETE FROM user.view_history");

        _manager->closeDatabase();
    }

    searchAllViewHistory();
}
