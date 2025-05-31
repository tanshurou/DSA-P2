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

/** One match’s data, per your Matches.csv schema */
struct MatchResult {
    std::string matchID;
    std::string stage;
    std::string player1ID;
    std::string player2ID;
    std::string winnerID;    // "-1" if not completed
    std::string status;      // "Scheduled" or "Completed"
    std::string groupID;     // NEW column
    int         duration;    // seconds
    Timestamp   timestamp;   // from Date + Time
};

class ResultLogger {
public:
    explicit ResultLogger(int recentSize);
    ~ResultLogger();

    void addResult(const MatchResult& r);

    void printRecent(int n) const;
    void printPlayerHistory(const std::string& playerID) const;
    void printMatchesOnDate(const std::string& dateStr) const;

    void loadHistoryFromCSV(const std::string& filename);
    void saveHistoryToCSV(const std::string& filename) const;

private:
    struct ListNode {
        MatchResult data;
        ListNode*   next;
        explicit ListNode(const MatchResult& m): data(m), next(nullptr) {}
    };

    // ring buffer for most‐recent
    int                      recentMaxSize;
    std::vector<MatchResult> recentBuffer;
    int                      bufferStart;
    int                      bufferCount;

    // full history
    ListNode* historyHead;
    ListNode* historyTail;

    void clearHistory();

    MatchResult* getLastNResults(int n, int& outCount) const;
    MatchResult* getPlayerHistory(const std::string& playerID, int& outCount) const;
};

#endif // RESULTLOGGER_HPP
