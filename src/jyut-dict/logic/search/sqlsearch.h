#ifndef SQLSEARCH_H
#define SQLSEARCH_H

#include "logic/database/sqldatabasemanager.h"
#include "logic/entry/entry.h"
#include "logic/search/isearch.h"
#include "logic/search/isearchobservable.h"

#include <QObject>
#include <QtSql>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

// SQLSearch searches the database provided by SQLDatabaseManager.

class SQLSearch : public QObject,
                  virtual public ISearch,
                  virtual public ISearchObservable
{
public:
    SQLSearch();
    SQLSearch(std::shared_ptr<SQLDatabaseManager> manager);
    SQLSearch(const SQLSearch &search);
    ~SQLSearch() override;

    void registerObserver(ISearchObserver *observer) override;
    void deregisterObserver(ISearchObserver *observer) override;

    void searchSimplified(const QString searchTerm) override;
    void searchTraditional(const QString searchTerm) override;
    void searchJyutping(const QString searchTerm) override;
    void searchPinyin(const QString searchTerm) override;
    void searchEnglish(const QString searchTerm) override;

    void searchByUnique(const QString simplified,
                        const QString traditional,
                        const QString jyutping,
                        const QString pinyin) override;

    void searchTraditionalSentences(const QString searchTerm);

private:
    void notifyObserversOfEmptySet(bool emptyQuery);
    void notifyObservers(const std::vector<Entry> &results, bool emptyQuery) override;
    void notifyObservers(const std::vector<SourceSentence> &results,
                         bool emptyQuery) override;

    unsigned long long generateAndSetQueryID(void);
    bool checkQueryIDCurrent(const unsigned long long queryID);

    void runThread(void (SQLSearch::*threadFunction)(const QString searchTerm, const unsigned long long queryID),
                   const QString &searchTerm, const unsigned long long queryID);
    void searchSimplifiedThread(const QString searchTerm,
                                const unsigned long long queryID);
    void searchTraditionalThread(const QString searchTerm,
                                 const unsigned long long queryID);
    void searchJyutpingThread(const QString searchTerm,
                              const unsigned long long queryID);
    void searchPinyinThread(const QString searchTerm,
                            const unsigned long long queryID);
    void searchEnglishThread(const QString searchTerm,
                             const unsigned long long queryID);

    void searchByUniqueThread(const QString simplified,
                              const QString traditional,
                              const QString jyutping,
                              const QString pinyin,
                              const unsigned long long queryID);

    void searchTraditionalSentencesThread(const QString searchTerm,
                                          const unsigned long long queryID);

    int segmentPinyin(const QString &string, std::vector<std::string> &words);
    int segmentJyutping(const QString &string, std::vector<std::string> &words);
    std::vector<std::string> explodePhonetic(const QString &string,
                                             const char delimiter);
    std::string implodePhonetic(const std::vector<std::string> &words,
                                const char *delimiter,
                                bool surroundWithQuotes=false);
    std::vector<Entry> parseEntries(QSqlQuery &query);
    std::vector<SourceSentence> parseSentences(QSqlQuery &query);

    std::list<ISearchObserver *> _observers;

    std::shared_ptr<SQLDatabaseManager> _manager;
    QString _currentSearchString;
    QSqlQuery _query;

    std::mutex _notifyMutex;

    std::atomic<unsigned long long> _queryID;
    std::mt19937_64 _generator;
    std::uniform_int_distribution<unsigned long long> _dist;
};

Q_DECLARE_METATYPE(SQLSearch);

#endif // SQLSEARCH_H
