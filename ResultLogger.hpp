#ifndef RESULTLOGGER_HPP
#define RESULTLOGGER_HPP

#include <string>
#include <vector>

// Simple ISO‐style timestamp
struct Timestamp {
    int year, month, day, hour, minute, second;
    Timestamp(int y=0,int mo=0,int d=0,int h=0,int mi=0,int s=0)
      : year(y), month(mo), day(d), hour(h), minute(mi), second(s) {}
};

// Data recorded for each match
struct MatchResult {
    int matchID;
    std::string round;
    int player1ID, player2ID;
    int player1Score, player2Score;
    int winnerID;
    int duration;          // in seconds
    Timestamp timestamp;
};

class ResultLogger {
public:
    explicit ResultLogger(int recentSize);
    ~ResultLogger();

    // record and print
    void addResult(const MatchResult& r);
    void printRecent(int n) const;
    void printPlayerHistory(int playerID) const;

    // CSV persistence
    void loadHistoryFromCSV(const std::string& filename);
    void saveHistoryToCSV(const std::string& filename) const;

private:
    // full-history linked list
    struct ListNode {
        MatchResult data;
        ListNode*   next;
        explicit ListNode(const MatchResult& m): data(m), next(nullptr) {}
    };

    // ring buffer for most-recent results
    int                     recentMaxSize;
    std::vector<MatchResult> recentBuffer;
    int                     bufferStart;  // index of oldest element
    int                     bufferCount;  // how many valid entries

    // helpers for full history
    void clearHistory();
    MatchResult* getLastNResults(int n, int& outCount) const;
    MatchResult* getPlayerHistory(int playerID, int& outCount) const;

    // full history
    ListNode* historyHead;
    ListNode* historyTail;
};

#endif // RESULTLOGGER_HPP
