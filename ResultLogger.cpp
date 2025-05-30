// ResultLogger.cpp
#include "ResultLogger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <string>
#include <cstdio>
#include <algorithm>

// --- ctor / dtor / clear ----

ResultLogger::ResultLogger(int recentSize)
  : recentMaxSize(recentSize),
    recentBuffer(new MatchResult[recentSize]),
    bufferStart(0),
    bufferCount(0),
    historyHead(nullptr),
    historyTail(nullptr)
{}

ResultLogger::~ResultLogger() {
    clearHistory();
    delete[] recentBuffer;
}

void ResultLogger::clearHistory() {
    // --- clear full-history linked list ---
    while (historyHead) {
        ListNode* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;

    // --- reset circular buffer ---
    bufferStart = bufferCount = 0;

    // --- clear stats maps ---
    playerStats.clear();
    head2headCount.clear();

    // --- clear head-to-head match lists ---
    for (auto &m1 : head2headList) {
        for (auto &m2 : m1.second) {
            H2HMatchNode* cur = m2.second;
            while (cur) {
                H2HMatchNode* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
    }
    head2headList.clear();

    // --- clear stage summary ---
    stageSummary.clear();
}

// --- addResult & helpers ----

void ResultLogger::addResult(const MatchResult& r) {
    // 1) Circular queue insert
    if (bufferCount < recentMaxSize) {
        recentBuffer[bufferCount++] = r;
    } else {
        recentBuffer[bufferStart] = r;
        bufferStart = (bufferStart + 1) % recentMaxSize;
    }

    // 2) Append to full-history FIFO
    ListNode* node = new ListNode(r);
    if (!historyTail) {
        historyHead = historyTail = node;
    } else {
        historyTail->next = node;
        historyTail = node;
    }

    // 3) If completed, update all stats
    if (r.status == "Completed" && r.winnerID != "-1") {
        // 3a) overall win/loss
        playerStats[r.winnerID].first++;
        std::string loser = (r.player1ID == r.winnerID ? r.player2ID : r.player1ID);
        playerStats[loser].second++;

        // 3b) stage summary
        auto &ss = stageSummary[r.stage];
        ss.first++;
        ss.second += r.duration;

        // 3c) head-to-head count
        head2headCount[r.winnerID][loser]++;

        // 3d) head-to-head match list
        H2HMatchNode* hn = new H2HMatchNode(r.matchID);
        hn->next = head2headList[r.winnerID][loser];
        head2headList[r.winnerID][loser] = hn;
    }
}

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) const {
    MatchResult* tmp = new MatchResult[n];
    int seen = 0;
    for (int i = 0; i < bufferCount && seen < n; ++i) {
        int idx = (bufferStart + bufferCount - 1 - i + recentMaxSize) % recentMaxSize;
        if (recentBuffer[idx].status == "Completed") {
            tmp[seen++] = recentBuffer[idx];
        }
    }
    outCount = seen;
    if (!seen) { delete[] tmp; return nullptr; }
    return tmp;
}

MatchResult* ResultLogger::getPlayerHistory(const std::string& playerID, int& outCount) const {
    int cnt = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.status == "Completed" &&
           (cur->data.player1ID == playerID || cur->data.player2ID == playerID))
        {
            ++cnt;
        }
    }
    outCount = cnt;
    if (!cnt) return nullptr;

    MatchResult* arr = new MatchResult[cnt];
    int idx = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.status == "Completed" &&
           (cur->data.player1ID == playerID || cur->data.player2ID == playerID))
        {
            arr[idx++] = cur->data;
        }
    }
    return arr;
}

// --- CSV loader ----

void ResultLogger::loadHistoryFromCSV(const std::string& fn) {
    std::ifstream in(fn);
    if (!in.is_open()) throw std::runtime_error("Cannot open " + fn);
    clearHistory();

    std::string line;
    std::getline(in, line);  // skip header

    while (std::getline(in, line)) {
        std::istringstream ss(line);
        MatchResult r;
        std::string dateStr, timeStr, durStr;

        std::getline(ss, r.matchID,   ',');
        std::getline(ss, r.stage,     ',');
        std::getline(ss, r.player1ID, ',');
        std::getline(ss, r.player2ID, ',');
        std::getline(ss, r.winnerID,  ',');
        std::getline(ss, r.status,    ',');
        std::getline(ss, r.groupID,   ',');
        std::getline(ss, timeStr,     ',');
        std::getline(ss, durStr,      ',');
        r.duration = std::stoi(durStr);
        std::getline(ss, dateStr);

        int d = std::stoi(dateStr);
        r.timestamp.year   = d / 10000;
        r.timestamp.month  = (d / 100) % 100;
        r.timestamp.day    = d % 100;
        int t = std::stoi(timeStr);
        r.timestamp.hour   = t / 100;
        r.timestamp.minute = t % 100;
        r.timestamp.second = 0;

        addResult(r);
    }
}

// --- printRecent ----

void ResultLogger::printRecent(int n) const {
    int cnt;
    auto* arr = getLastNResults(n, cnt);
    if (!cnt) {
        std::cout << "\n=== No recent completed matches ===\n";
        return;
    }

    std::cout << "\n=== Last " << cnt << " Completed Matches ===\n"
              << std::left
              << std::setw(8)  << "MatchID"
              << std::setw(15) << "Stage"
              << std::setw(12) << "Status"
              << std::setw(10) << "Duration"
              << std::setw(20) << "Timestamp"
              << std::setw(20) << "Players"
              << "Winner\n"
              << std::string(8+15+12+10+20+20+6, '-') << "\n";

    for (int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        std::string players = m.player1ID + " vs " + m.player2ID;

        std::cout << std::left
                  << std::setw(8)  << m.matchID
                  << std::setw(15) << m.stage
                  << std::setw(12) << m.status
                  << std::setw(10) << (std::to_string(m.duration) + "s")
                  << std::setw(20) << ts
                  << std::setw(20) << players
                  << m.winnerID << "\n";
    }
    delete[] arr;
}

// --- printPlayerHistory ----

void ResultLogger::printPlayerHistory(const std::string& playerID) const {
    int cnt;
    auto* arr = getPlayerHistory(playerID, cnt);
    if (!cnt) {
        std::cout << "\n=== No completed history for Player " << playerID << " ===\n";
        return;
    }

    std::cout << "\n=== Completed History for Player " << playerID
              << " (" << cnt << " matches) ===\n"
              << std::left
              << std::setw(8)  << "MatchID"
              << std::setw(15) << "Stage"
              << std::setw(12) << "Status"
              << std::setw(20) << "Timestamp"
              << std::setw(20) << "Opponent"
              << "Winner\n"
              << std::string(8+15+12+20+20+6, '-') << "\n";

    for (int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        std::string opp = (m.player1ID == playerID ? m.player2ID : m.player1ID);

        std::cout << std::left
                  << std::setw(8)  << m.matchID
                  << std::setw(15) << m.stage
                  << std::setw(12) << m.status
                  << std::setw(20) << ts
                  << std::setw(20) << opp
                  << m.winnerID << "\n";
    }
    delete[] arr;
}

// --- printMatchesOnDate ----

void ResultLogger::printMatchesOnDate(const std::string& dateStr) const {
    if (dateStr.size() != 8) {
        std::cout << "Invalid date format (expected YYYYMMDD)\n";
        return;
    }
    int year  = std::stoi(dateStr.substr(0,4));
    int month = std::stoi(dateStr.substr(4,2));
    int day   = std::stoi(dateStr.substr(6,2));

    int count = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        if (r.status=="Completed"
         && r.timestamp.year==year
         && r.timestamp.month==month
         && r.timestamp.day==day)
        {
            ++count;
        }
    }
    if (!count) {
        std::cout << "\n=== No completed matches on "
                  << year << "-" << std::setw(2) << std::setfill('0') << month
                  << "-" << std::setw(2) << std::setfill('0') << day
                  << " ===\n";
        std::cout << std::setfill(' ');
        return;
    }

    std::cout << "\n=== Completed Matches on "
              << year << "-" << std::setw(2) << std::setfill('0') << month
              << "-" << std::setw(2) << std::setfill('0') << day
              << " ===\n"
              << std::left
              << std::setw(8)  << "MatchID"
              << std::setw(15) << "Stage"
              << std::setw(12) << "Status"
              << std::setw(10) << "Duration"
              << std::setw(20) << "Timestamp"
              << std::setw(20) << "Players"
              << "Winner\n"
              << std::string(8+15+12+10+20+20+6, '-') << "\n";

    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& m = cur->data;
        if (m.status=="Completed"
         && m.timestamp.year==year
         && m.timestamp.month==month
         && m.timestamp.day==day)
        {
            char ts[20];
            std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                         m.timestamp.year, m.timestamp.month, m.timestamp.day,
                         m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
            std::string players = m.player1ID + " vs " + m.player2ID;

            std::cout << std::left
                      << std::setw(8)  << m.matchID
                      << std::setw(15) << m.stage
                      << std::setw(12) << m.status
                      << std::setw(10) << (std::to_string(m.duration) + "s")
                      << std::setw(20) << ts
                      << std::setw(20) << players
                      << m.winnerID << "\n";
        }
    }
}

// --- printHeadToHead ----

void ResultLogger::printHeadToHead(const std::string& A, const std::string& B) const {
    int Awin = 0, Bwin = 0;
    auto itA = head2headCount.find(A);
    if (itA != head2headCount.end()) {
        auto itAB = itA->second.find(B);
        if (itAB != itA->second.end()) Awin = itAB->second;
    }
    auto itB = head2headCount.find(B);
    if (itB != head2headCount.end()) {
        auto itBA = itB->second.find(A);
        if (itBA != itB->second.end()) Bwin = itBA->second;
    }

    std::cout << "\n=== Head-to-Head: " << A << " vs " << B << " ===\n";
    if (Awin==0 && Bwin==0) {
        std::cout << "No head-to-head matches found between " << A << " and " << B << ".\n";
    } else {
        std::cout << A << " -> " << B << " : " << Awin << "\n"
                  << B << " -> " << A << " : " << Bwin << "\n";

        // list match IDs
        auto la = head2headList.find(A);
        if (la != head2headList.end()) {
            auto lab = la->second.find(B);
            if (lab != la->second.end()) {
                std::cout << A << " vs " << B << " match IDs: ";
                for (auto* cur = lab->second; cur; cur = cur->next)
                    std::cout << cur->matchID << (cur->next ? ", " : "");
                std::cout << "\n";
            }
        }
        auto lb = head2headList.find(B);
        if (lb != head2headList.end()) {
            auto lba = lb->second.find(A);
            if (lba != lb->second.end()) {
                std::cout << B << " vs " << A << " match IDs: ";
                for (auto* cur = lba->second; cur; cur = cur->next)
                    std::cout << cur->matchID << (cur->next ? ", " : "");
                std::cout << "\n";
            }
        }
    }

    // print win-rate for each
    auto printWR = [&](const std::string& p){
        auto it = playerStats.find(p);
        if (it==playerStats.end() || it->second.first+it->second.second==0) {
            std::cout << p << " has no completed matches.\n";
        } else {
            int w = it->second.first;
            int l = it->second.second;
            double pct = 100.0 * w / (w + l);
            std::cout << p << " win rate: "
                      << std::fixed << std::setprecision(1)
                      << pct << "% (" << w << "/" << (w+l) << ")\n";
        }
    };
    printWR(A);
    printWR(B);
}

// --- printStageSummary ----

void ResultLogger::printStageSummary() const {
    if (stageSummary.empty()) {
        std::cout << "\n=== No completed matches to summarize ===\n";
        return;
    }

    size_t w = 5; // length of "Stage"
    for (auto& kv : stageSummary)
        if (kv.first.size() > w) w = kv.first.size();

    std::cout << "\n=== Stage Summary ===\n"
              << std::left
              << std::setw(w+2) << "Stage"
              << std::setw(10) << "#Matches"
              << "AvgDur(s)\n"
              << std::string(w+2 + 10 + 9, '-') << "\n";

    for (auto& kv : stageSummary) {
        const auto& stage = kv.first;
        int cnt  = kv.second.first;
        int tot  = kv.second.second;
        int avg  = cnt ? tot/cnt : 0;
        std::cout << std::left
                  << std::setw(w+2) << stage
                  << std::setw(10)  << cnt
                  << avg << "\n";
    }
}
