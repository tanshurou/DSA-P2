// ResultLogger.cpp
#include "ResultLogger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>  // for setw, setfill
#include <algorithm>


// ------------------------
// Construction / Cleanup
// ------------------------

ResultLogger::ResultLogger(int recentSize)
  : recentTop(nullptr),
    recentCount(0),
    recentMaxSize(recentSize),
    historyHead(nullptr),
    historyTail(nullptr)
{}

ResultLogger::~ResultLogger() {
    clear();
}

void ResultLogger::clear() {
    // Clear stack
    while (recentTop) {
        StackNode* tmp = recentTop;
        recentTop = recentTop->next;
        delete tmp;
    }
    recentCount = 0;

    // Clear full history
    while (historyHead) {
        ListNode* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;
}

// ------------------------
// Internal Stack Helpers
// ------------------------

void ResultLogger::pushRecent(const MatchResult& result) {
    StackNode* node = new StackNode(result);
    node->next = recentTop;
    recentTop = node;
    recentCount++;

    if (recentCount > recentMaxSize) {
        popBottomOfStack();
        recentCount--;
    }
}

void ResultLogger::popBottomOfStack() {
    if (!recentTop) return;
    if (!recentTop->next) {
        delete recentTop;
        recentTop = nullptr;
        return;
    }
    StackNode* cur = recentTop;
    while (cur->next->next) {
        cur = cur->next;
    }
    delete cur->next;
    cur->next = nullptr;
}

// ------------------------
// Core Functionality
// ------------------------

void ResultLogger::addResult(const MatchResult& r) {
    // 1) Push to recent stack
    pushRecent(r);

    // 2) Append to full history
    ListNode* node = new ListNode(r);
    if (!historyTail) {
        historyHead = historyTail = node;
    } else {
        historyTail->next = node;
        historyTail = node;
    }
}

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) {
    outCount = (n < recentCount ? n : recentCount);
    if (outCount == 0) return nullptr;

    MatchResult* arr = new MatchResult[outCount];
    StackNode*   cur = recentTop;
    for (int i = 0; i < outCount; ++i) {
        arr[i] = cur->data;
        cur = cur->next;
    }
    return arr;
}

MatchResult* ResultLogger::getPlayerHistory(int playerID, int& outCount) {
    // First pass: count matches
    int cnt = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.player1ID == playerID ||
            cur->data.player2ID == playerID) {
            ++cnt;
        }
    }
    outCount = cnt;
    if (!cnt) return nullptr;

    // Second pass: collect
    MatchResult* arr = new MatchResult[cnt];
    int           idx = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.player1ID == playerID ||
            cur->data.player2ID == playerID) {
            arr[idx++] = cur->data;
        }
    }
    return arr;
}

// ------------------------
// CSV Persistence
// ------------------------

void ResultLogger::saveHistoryToCSV(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open())
        throw std::runtime_error("Cannot open " + filename + " for writing");

    // Header
    out << "match_id,round,player1_id,player2_id,"
           "player1_score,player2_score,winner_id,"
           "duration_secs,timestamp\n";

    // ISO timestamp helper
    auto fmtTS = [&](const Timestamp& t) {
        std::ostringstream s;
        s << std::setw(4) << std::setfill('0') << t.year << '-'
          << std::setw(2) << std::setfill('0') << t.month << '-'
          << std::setw(2) << std::setfill('0') << t.day   << 'T'
          << std::setw(2) << std::setfill('0') << t.hour  << ':'
          << std::setw(2) << std::setfill('0') << t.minute<< ':'
          << std::setw(2) << std::setfill('0') << t.second;
        return s.str();
    };

    // Write each match
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        out
          << r.matchID   << ','
          << r.round      << ','
          << r.player1ID  << ','
          << r.player2ID  << ','
          << r.player1Score << ','
          << r.player2Score << ','
          << r.winnerID   << ','
          << r.duration   << ','
          << fmtTS(r.timestamp)
          << '\n';
    }
    out.close();
}

void ResultLogger::loadHistoryFromCSV(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open())
        throw std::runtime_error("Cannot open " + filename + " for reading");

    clear();  // reset any existing data

    std::string line;
    // Skip header
    std::getline(in, line);

    while (std::getline(in, line)) {
        std::istringstream ss(line);
        MatchResult r;
        std::string ts_str;
        char comma;

        // Parse fields
        std::getline(ss, line, ','); r.matchID = std::stoi(line);
        std::getline(ss, r.round,   ',');
        std::getline(ss, line,      ','); r.player1ID   = std::stoi(line);
        std::getline(ss, line,      ','); r.player2ID   = std::stoi(line);
        std::getline(ss, line,      ','); r.player1Score= std::stoi(line);
        std::getline(ss, line,      ','); r.player2Score= std::stoi(line);
        std::getline(ss, line,      ','); r.winnerID    = std::stoi(line);
        std::getline(ss, line,      ','); r.duration    = std::stoi(line);
        std::getline(ss, ts_str);               // the rest is timestamp

        // Parse ISO timestamp YYYY-MM-DDTHH:MM:SS
        std::replace(ts_str.begin(), ts_str.end(), 'T', '-');
        std::replace(ts_str.begin()+10, ts_str.end(), ':', '-');
        std::istringstream ts_in(ts_str);
        ts_in  >> r.timestamp.year
               >> comma >> r.timestamp.month
               >> comma >> r.timestamp.day
               >> comma >> r.timestamp.hour
               >> comma >> r.timestamp.minute
               >> comma >> r.timestamp.second;

        // Add it (will go into both stack & history)
        addResult(r);
    }
    in.close();
}

// ------------------------
// Pretty-Printing
// ------------------------

void ResultLogger::printRecent(int n) const {
    int count;
    // We need a non-const call, so cast away constness here:
    auto arr = const_cast<ResultLogger*>(this)->getLastNResults(n, count);
    std::cout << "\n=== Last " << count << " Matches ===\n";
    for (int i = 0; i < count; ++i) {
        const auto& m = arr[i];
        std::cout
          << "Match " << m.matchID
          << " ["      << m.round << "] "
          << "("      << m.duration << "s) "
          << m.timestamp.year << '-'
          << std::setw(2) << std::setfill('0') << m.timestamp.month << '-'
          << std::setw(2) << std::setfill('0') << m.timestamp.day << 'T'
          << std::setw(2) << std::setfill('0') << m.timestamp.hour << ':'
          << std::setw(2) << std::setfill('0') << m.timestamp.minute << ':'
          << std::setw(2) << std::setfill('0') << m.timestamp.second
          << "\n  Player " << m.player1ID
          << " [" << m.player1Score << "] vs "
          << "Player "  << m.player2ID
          << " [" << m.player2Score << "] → Winner: "
          << m.winnerID
          << "\n\n";
    }
    delete[] arr;
}

void ResultLogger::printPlayerHistory(int playerID) const {
    int count;
    auto arr = const_cast<ResultLogger*>(this)->getPlayerHistory(playerID, count);
    std::cout << "\n=== History for Player " << playerID
              << " (" << count << " matches) ===\n";
    for (int i = 0; i < count; ++i) {
        const auto& m = arr[i];
        int opp     = (m.player1ID == playerID ? m.player2ID : m.player1ID);
        int score   = (m.player1ID == playerID ? m.player1Score : m.player2Score);
        int oppScore= (m.player1ID == playerID ? m.player2Score : m.player1Score);
        bool win    = (m.winnerID == playerID);

        std::cout
          << "Match " << m.matchID
          << " [" << m.round << "] "
          << (win ? "Win" : "Loss")
          << " vs Player " << opp
          << " (" << score << "-" << oppScore << ") "
          << m.timestamp.year << '-'
          << std::setw(2) << std::setfill('0') << m.timestamp.month << '-'
          << std::setw(2) << std::setfill('0') << m.timestamp.day << 'T'
          << std::setw(2) << std::setfill('0') << m.timestamp.hour << ':'
          << std::setw(2) << std::setfill('0') << m.timestamp.minute << ':'
          << std::setw(2) << std::setfill('0') << m.timestamp.second
          << "\n";
    }
    delete[] arr;
}
