// ResultLogger.hpp
#ifndef RESULTLOGGER_HPP
#define RESULTLOGGER_HPP

#include <string>

// --- Simple timestamp; ISO-style formatting/parsing in CSV I/O ---
struct Timestamp {
    int year, month, day, hour, minute, second;
    Timestamp()
      : year(0), month(0), day(0), hour(0), minute(0), second(0) {}
    Timestamp(int y,int mo,int d,int h,int mi,int s)
      : year(y), month(mo), day(d), hour(h), minute(mi), second(s) {}
};

// --- Holds all data for a single match result ---
struct MatchResult {
    int         matchID;     // Unique match identifier
    std::string round;       // E.g. "Qualifier", "Quarterfinal", etc.
    int         player1ID;   // First player’s ID
    int         player2ID;   // Second player’s ID
    int         player1Score;
    int         player2Score;
    int         winnerID;    // player1ID or player2ID
    int         duration;    // in seconds
    Timestamp   timestamp;   // When match ended

    MatchResult()
      : matchID(0), round(),
        player1ID(0), player2ID(0),
        player1Score(0), player2Score(0),
        winnerID(0), duration(0),
        timestamp() {}
};

// --- Node for the recent-results stack (LIFO) ---
struct StackNode {
    MatchResult data;
    StackNode*  next;
    StackNode(const MatchResult& mr) : data(mr), next(nullptr) {}
};

// --- Node for the full-history linked list (FIFO) ---
struct ListNode {
    MatchResult data;
    ListNode*   next;
    ListNode(const MatchResult& mr) : data(mr), next(nullptr) {}
};

class ResultLogger {
private:
    // Recent-results stack (bounded size)
    StackNode* recentTop;
    int        recentCount;
    const int  recentMaxSize;

    // Full-history linked list
    ListNode* historyHead;
    ListNode* historyTail;

    // Helpers for stack management
    void pushRecent(const MatchResult& result);
    void popBottomOfStack();

public:
    // ctor/dtor
    ResultLogger(int recentSize = 10);
    ~ResultLogger();

    // Core APIs
    void addResult(const MatchResult& result);
    MatchResult* getLastNResults(int n, int& outCount);
    MatchResult* getPlayerHistory(int playerID, int& outCount);
    void clear();

    // Persistence (CSV)
    void saveHistoryToCSV(const std::string& filename) const;
    void loadHistoryFromCSV(const std::string& filename);

    // Pretty-printing
    void printRecent(int n) const;
    void printPlayerHistory(int playerID) const;
};

#endif // RESULTLOGGER_HPP
