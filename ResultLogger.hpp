#ifndef RESULTLOGGER_HPP
#define RESULTLOGGER_HPP

#include <string>
#include <map>
#include <unordered_map>

struct Timestamp {
    int year, month, day, hour, minute, second;
    Timestamp(int y = 0, int mo = 0, int d = 0, int h = 0, int mi = 0, int s = 0)
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

    // Load all lines from a CSV file (no header-skip). Overwrites any existing data.
    void loadHistoryFromCSV(const std::string& filename);

    // “Match Info” submenu:
    //   - printRecent(n): show the last n completed matches
    //   - printMatchesOnDate(dateStr): show all completed matches on YYYYMMDD
    //   - printStageSummary(): show per-stage #matches and average duration
    void printRecent(int n) const;
    void printMatchesOnDate(const std::string& dateStr) const;
    void printStageSummary() const;

    // “Player Info” submenu:
    //   - printPlayerHistory(playerID): show all completed matches involving that player
    //   - printHeadToHead(A, B): show head-to-head summary (counts, percentage, and match IDs)
    void printPlayerHistory(const std::string& playerID) const;
    void printHeadToHead(const std::string& A, const std::string& B) const;

private:
    struct ListNode {
        MatchResult data;
        ListNode*   next;
        explicit ListNode(const MatchResult& m) : data(m), next(nullptr) {}
    };

    struct H2HMatchNode {
        std::string   matchID;
        H2HMatchNode* next;
        explicit H2HMatchNode(const std::string& id) : matchID(id), next(nullptr) {}
    };

    // 1) Circular buffer of most-recent matches
    MatchResult*                                 recentBuffer;
    int                                          recentMaxSize;
    int                                          bufferStart;
    int                                          bufferCount;

    // 2) Full-history FIFO as a singly-linked list
    ListNode*                                    historyHead;
    ListNode*                                    historyTail;

    // 3) Win/Loss stats for each player (so that every loser appears)
    std::unordered_map<std::string, std::pair<int,int>> playerStats;

    // 4) Head-to-head:
    //    head2headCount[A][B] = number of times A beat B
    std::unordered_map<std::string,
      std::unordered_map<std::string,int>>        head2headCount;
    //    head2headList[A][B] = linked list of matchIDs where A beat B
    std::unordered_map<std::string,
      std::unordered_map<std::string, H2HMatchNode*>> head2headList;

    // 5) Stage summary: stage → (totalMatches, totalDuration)
    std::map<std::string, std::pair<int,int>>    stageSummary;

    // Helpers
    void clearHistory();
    void addResult(const MatchResult& r);
    MatchResult* getLastNResults(int n, int& outCount) const;
    MatchResult* getPlayerHistory(const std::string& pid, int& outCount) const;
};

// -----------------------------------------------------------------
// Free-function that wraps everything into one “run” you can call
// from your main. You pass in the CSV path and buffer-size.
// -----------------------------------------------------------------
void runResultLogger(const std::string& csvPath, int recentSize);

#endif // RESULTLOGGER_HPP
