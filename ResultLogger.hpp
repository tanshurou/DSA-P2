// ResultLogger.hpp
#ifndef RESULTLOGGER_HPP
#define RESULTLOGGER_HPP

#include <string>
#include <vector>

/** Simple date+time container */
struct Timestamp {
    int year, month, day, hour, minute, second;
    Timestamp(int y=0,int mo=0,int d=0,int h=0,int mi=0,int s=0)
      : year(y), month(mo), day(d), hour(h), minute(mi), second(s) {}
};

/** One match’s data, as per your “Matches.csv” schema */
struct MatchResult {
    std::string matchID;
    std::string stage;
    std::string player1ID;
    std::string player2ID;
    std::string winnerID;    // "-1" if not completed
    std::string status;      // "Scheduled" or "Completed"
    int         duration;    // in seconds
    Timestamp   timestamp;   // built from Date + Time columns
};

class ResultLogger {
public:
    explicit ResultLogger(int recentSize);
    ~ResultLogger();

    /** Add one match to both the ring buffer and full history */
    void addResult(const MatchResult& r);

    /** Print the N most‐recent completed matches */
    void printRecent(int n) const;

    /** Print all completed matches for a given player */
    void printPlayerHistory(const std::string& playerID) const;

    /** Print all completed matches on a given date (YYYYMMDD) */
    void printMatchesOnDate(const std::string& dateStr) const;

    /** Load/save using your teammate’s Matches.csv schema */
    void loadHistoryFromCSV(const std::string& filename);
    void saveHistoryToCSV(const std::string& filename) const;

private:
    struct ListNode {
        MatchResult data;
        ListNode*   next;
        explicit ListNode(const MatchResult& m): data(m), next(nullptr) {}
    };

    // ring buffer for most-recent matches
    int                      recentMaxSize;
    std::vector<MatchResult> recentBuffer;
    int                      bufferStart;
    int                      bufferCount;

    // full history linked list
    ListNode* historyHead;
    ListNode* historyTail;

    void clearHistory();

    // helpers that filter out scheduled matches
    MatchResult* getLastNResults(int n, int& outCount) const;
    MatchResult* getPlayerHistory(const std::string& playerID, int& outCount) const;
};

#endif // RESULTLOGGER_HPP
