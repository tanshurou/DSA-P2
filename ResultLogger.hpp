// ResultLogger.hpp
#ifndef RESULTLOGGER_HPP
#define RESULTLOGGER_HPP

#include <string>
#include <map>
#include <unordered_map>

struct Timestamp {
    int year, month, day, hour, minute, second;
    Timestamp(int y=0,int mo=0,int d=0,int h=0,int mi=0,int s=0)
      : year(y), month(mo), day(d), hour(h), minute(mi), second(s) {}
};

struct MatchResult {
    std::string matchID;
    std::string stage;
    std::string player1ID;
    std::string player2ID;
    std::string winnerID;    // "-1" if not completed
    std::string status;      // "Scheduled" or "Completed"
    std::string groupID;
    int         duration;    // seconds
    Timestamp   timestamp;
};

class ResultLogger {
public:
    explicit ResultLogger(int recentSize);
    ~ResultLogger();

    // Load all match data from CSV
    void loadHistoryFromCSV(const std::string& filename);

    // Core reports
    void printRecent(int n) const;
    void printPlayerHistory(const std::string& playerID) const;
    void printMatchesOnDate(const std::string& dateStr) const;

    // Advanced features
    void printHeadToHead(const std::string& A, const std::string& B) const;
    void printStageSummary() const;

private:
    struct ListNode {
        MatchResult data;
        ListNode*   next;
        explicit ListNode(const MatchResult& m): data(m), next(nullptr) {}
    };

    struct H2HMatchNode {
        std::string   matchID;
        H2HMatchNode* next;
        explicit H2HMatchNode(const std::string& id): matchID(id), next(nullptr) {}
    };

    // 1) Circular queue buffer of most recent matches
    MatchResult*                                 recentBuffer;
    int                                           recentMaxSize;
    int                                           bufferStart;
    int                                           bufferCount;

    // 2) Full-history FIFO as a linked list
    ListNode*                                     historyHead;
    ListNode*                                     historyTail;

    // 3) Overall player win/loss stats for win-rate
    std::unordered_map<std::string, std::pair<int,int>> playerStats;

    // 4) Head-to-head counts: A -> (B -> wins)
    std::unordered_map<std::string,
      std::unordered_map<std::string,int>>       head2headCount;
    //    and match ID lists: A -> (B -> linked list of matchIDs)
    std::unordered_map<std::string,
      std::unordered_map<std::string,H2HMatchNode*>> head2headList;

    // 5) Stage summary: stage -> (totalMatches, totalDuration)
    std::map<std::string, std::pair<int,int>>    stageSummary;

    // Internal
    void clearHistory();
    void addResult(const MatchResult& r);
    MatchResult* getLastNResults(int n, int& outCount) const;
    MatchResult* getPlayerHistory(const std::string& playerID, int& outCount) const;
};

#endif // RESULTLOGGER_HPP
