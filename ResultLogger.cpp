// ResultLogger.cpp
#include "ResultLogger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdio>

/*
 ASCII-art table printer with full column borders.
 rows[0] is the header; every row must have the same column count.
*/
static void printTable(const std::vector<std::vector<std::string>>& rows) {
    if (rows.empty()) return;
    size_t cols = rows[0].size();
    std::vector<size_t> widths(cols, 0);

    // 1) compute max width per column
    for (auto& row : rows)
        for (size_t c = 0; c < cols; ++c)
            widths[c] = std::max(widths[c], row[c].size());

    // helper to print a border line
    auto printBorder = [&](char left, char sep, char right) {
        std::cout << left;
        for (size_t c = 0; c < cols; ++c) {
            std::cout << std::string(widths[c] + 2, '-')
                      << (c + 1 < cols ? sep : right);
        }
        std::cout << "\n";
    };

    // top border
    printBorder('+', '+', '+');

    // header row
    std::cout << "|";
    for (size_t c = 0; c < cols; ++c) {
        std::cout << " " << std::left
                  << std::setw(widths[c]) << rows[0][c]
                  << " |";
    }
    std::cout << "\n";

    // header/data separator
    printBorder('+', '+', '+');

    // data rows
    for (size_t r = 1; r < rows.size(); ++r) {
        std::cout << "|";
        for (size_t c = 0; c < cols; ++c) {
            std::cout << " " << std::left
                      << std::setw(widths[c]) << rows[r][c]
                      << " |";
        }
        std::cout << "\n";
    }

    // bottom border
    printBorder('+', '+', '+');
}

// --- ctor / dtor / clear

ResultLogger::ResultLogger(int recentSize)
  : recentMaxSize(recentSize),
    recentBuffer(recentSize),
    bufferStart(0),
    bufferCount(0),
    historyHead(nullptr),
    historyTail(nullptr)
{}

ResultLogger::~ResultLogger() {
    clearHistory();
}

void ResultLogger::clearHistory() {
    while (historyHead) {
        auto* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;
    bufferStart = bufferCount = 0;
}

// --- add & filtered helpers

void ResultLogger::addResult(const MatchResult& r) {
    if (bufferCount < recentMaxSize) {
        int idx = (bufferStart + bufferCount) % recentMaxSize;
        recentBuffer[idx] = r;
        ++bufferCount;
    } else {
        recentBuffer[bufferStart] = r;
        bufferStart = (bufferStart + 1) % recentMaxSize;
    }
    auto* node = new ListNode(r);
    if (!historyTail) historyHead = historyTail = node;
    else {
        historyTail->next = node;
        historyTail = node;
    }
}

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) const {
    std::vector<MatchResult> tmp;
    for (int seen = 0, i = 0; i < bufferCount && seen < n; ++i) {
        int idx = (bufferStart + bufferCount - 1 - i + recentMaxSize) % recentMaxSize;
        const auto& m = recentBuffer[idx];
        if (m.status == "Completed") {
            tmp.push_back(m);
            ++seen;
        }
    }
    outCount = static_cast<int>(tmp.size());
    if (!outCount) return nullptr;
    auto* arr = new MatchResult[outCount];
    for (int i = 0; i < outCount; ++i) arr[i] = tmp[i];
    return arr;
}

MatchResult* ResultLogger::getPlayerHistory(const std::string& playerID, int& outCount) const {
    int cnt = 0;
    for (auto* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.status == "Completed" &&
           (cur->data.player1ID == playerID || cur->data.player2ID == playerID))
            ++cnt;
    }
    outCount = cnt;
    if (!cnt) return nullptr;
    auto* arr = new MatchResult[cnt];
    int idx = 0;
    for (auto* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.status == "Completed" &&
           (cur->data.player1ID == playerID || cur->data.player2ID == playerID))
        {
            arr[idx++] = cur->data;
        }
    }
    return arr;
}

// --- CSV I/O (with GroupID)

void ResultLogger::loadHistoryFromCSV(const std::string& fn) {
    std::ifstream in(fn);
    if (!in) throw std::runtime_error("Cannot open " + fn + " for reading");
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
        std::getline(ss, r.groupID,   ',');  // read GroupID
        std::getline(ss, timeStr,     ',');
        std::getline(ss, durStr,      ','); r.duration = std::stoi(durStr);
        std::getline(ss, dateStr);

        int d = std::stoi(dateStr);
        r.timestamp.year  = d / 10000;
        r.timestamp.month = (d / 100) % 100;
        r.timestamp.day   = d % 100;
        int t = std::stoi(timeStr);
        r.timestamp.hour   = t / 100;
        r.timestamp.minute = t % 100;
        r.timestamp.second = 0;

        addResult(r);
    }
}

void ResultLogger::saveHistoryToCSV(const std::string& fn) const {
    std::ofstream out(fn);
    if (!out) throw std::runtime_error("Cannot open " + fn + " for writing");
    out << "MatchID,Stage,Player1ID,Player2ID,WinnerID,MatchStatus,"
           "GroupID,Time,DurationInSeconds,Date\n";

    for (auto* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        char db[9], tb[5];
        std::sprintf(db, "%04d%02d%02d",
                     r.timestamp.year, r.timestamp.month, r.timestamp.day);
        std::sprintf(tb, "%02d%02d",
                     r.timestamp.hour, r.timestamp.minute);
        out << r.matchID   << ','
            << r.stage     << ','
            << r.player1ID << ','
            << r.player2ID << ','
            << r.winnerID  << ','
            << r.status    << ','
            << r.groupID   << ','
            << tb          << ','
            << r.duration  << ','
            << db          << '\n';
    }
}

// --- Printing Tables using printTable

void ResultLogger::printRecent(int n) const {
    int cnt; auto* arr = getLastNResults(n, cnt);
    if (cnt == 0) {
        std::cout << "\n=== No recent completed matches ===\n";
        return;
    }
    std::vector<std::vector<std::string>> table;
    table.emplace_back(std::vector<std::string>{
        "MatchID","Stage","Status","Duration","Timestamp","Players","Winner"
    });
    for (int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        table.emplace_back(std::vector<std::string>{
            m.matchID,
            m.stage,
            m.status,
            std::to_string(m.duration) + "s",
            ts,
            m.player1ID + " vs " + m.player2ID,
            m.winnerID
        });
    }
    delete[] arr;
    printTable(table);
}

void ResultLogger::printPlayerHistory(const std::string& playerID) const {
    int cnt; auto* arr = getPlayerHistory(playerID, cnt);
    if (cnt == 0) {
        std::cout << "\n=== No completed history for Player " << playerID << " ===\n";
        return;
    }
    std::vector<std::vector<std::string>> table;
    table.emplace_back(std::vector<std::string>{
        "MatchID","Stage","Status","Timestamp","Opponent","Winner"
    });
    for (int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        std::string opp = (m.player1ID == playerID ? m.player2ID : m.player1ID);
        table.emplace_back(std::vector<std::string>{
            m.matchID,
            m.stage,
            m.status,
            ts,
            opp,
            m.winnerID
        });
    }
    delete[] arr;
    printTable(table);
}

void ResultLogger::printMatchesOnDate(const std::string& dateStr) const {
    if (dateStr.size() != 8) {
        std::cout << "Invalid date format (expected YYYYMMDD)\n";
        return;
    }
    int y = std::stoi(dateStr.substr(0,4)),
        mo = std::stoi(dateStr.substr(4,2)),
        da = std::stoi(dateStr.substr(6,2));

    std::vector<MatchResult> found;
    for (auto* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        if (r.status == "Completed"
         && r.timestamp.year  == y
         && r.timestamp.month == mo
         && r.timestamp.day   == da)
        {
            found.push_back(r);
        }
    }

    if (found.empty()) {
        std::cout << "\n=== No completed matches on "
                  << y << "-" << std::setw(2) << std::setfill('0') << mo
                  << "-" << std::setw(2) << std::setfill('0') << da
                  << " ===\n";
        std::cout << std::setfill(' ');
        return;
    }

    std::vector<std::vector<std::string>> table;
    table.emplace_back(std::vector<std::string>{
        "MatchID","Stage","Status","Duration","Timestamp","Players","Winner"
    });
    for (auto& m : found) {
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        table.emplace_back(std::vector<std::string>{
            m.matchID,
            m.stage,
            m.status,
            std::to_string(m.duration) + "s",
            ts,
            m.player1ID + " vs " + m.player2ID,
            m.winnerID
        });
    }
    printTable(table);
}
