// ResultLogger.cpp
#include "ResultLogger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <cstdio>

// ------------------------
// Construction / Cleanup
// ------------------------

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
    // clear linked list
    while (historyHead) {
        ListNode* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;
    // clear ring buffer
    bufferStart = bufferCount = 0;
}

// ------------------------
// Core Functionality
// ------------------------

void ResultLogger::addResult(const MatchResult& r) {
    // ring buffer insert (overwrite oldest once full)
    if (bufferCount < recentMaxSize) {
        int idx = (bufferStart + bufferCount) % recentMaxSize;
        recentBuffer[idx] = r;
        ++bufferCount;
    } else {
        recentBuffer[bufferStart] = r;
        bufferStart = (bufferStart + 1) % recentMaxSize;
    }
    // append to full-history list
    ListNode* node = new ListNode(r);
    if (!historyTail) {
        historyHead = historyTail = node;
    } else {
        historyTail->next = node;
        historyTail = node;
    }
}

// ------------------------
// FILTERED helpers
// ------------------------

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) const {
    std::vector<MatchResult> temp;
    for (int seen = 0, i = 0; i < bufferCount && seen < n; ++i) {
        int idx = (bufferStart + bufferCount - 1 - i + recentMaxSize) % recentMaxSize;
        const auto& m = recentBuffer[idx];
        if (m.status == "Completed") {
            temp.push_back(m);
            ++seen;
        }
    }
    outCount = static_cast<int>(temp.size());
    if (!outCount) return nullptr;
    MatchResult* arr = new MatchResult[outCount];
    for (int i = 0; i < outCount; ++i) arr[i] = temp[i];
    return arr;
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

// ------------------------
// CSV Persistence
// ------------------------

void ResultLogger::loadHistoryFromCSV(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open())
        throw std::runtime_error("Cannot open " + filename + " for reading");
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
    in.close();
}

void ResultLogger::saveHistoryToCSV(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open())
        throw std::runtime_error("Cannot open " + filename + " for writing");
    out << "MatchID,Stage,Player1ID,Player2ID,WinnerID,"
           "MatchStatus,Time,DurationInSeconds,Date\n";
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        char dateBuf[9], timeBuf[5];
        std::sprintf(dateBuf, "%04d%02d%02d",
                     r.timestamp.year, r.timestamp.month, r.timestamp.day);
        std::sprintf(timeBuf, "%02d%02d",
                     r.timestamp.hour, r.timestamp.minute);
        out << r.matchID   << ','
            << r.stage     << ','
            << r.player1ID << ','
            << r.player2ID << ','
            << r.winnerID  << ','
            << r.status    << ','
            << timeBuf     << ','
            << r.duration  << ','
            << dateBuf     << '\n';
    }
    out.close();
}

// ------------------------
// Pretty-Printing Tables
// ------------------------

#include <iostream>

void ResultLogger::printRecent(int n) const {
    int count;
    MatchResult* arr = getLastNResults(n, count);
    if (!count) {
        std::cout << "\n=== No recent completed matches ===\n";
        return;
    }
    std::cout << "\n=== Last " << count << " Completed Matches ===\n"
              << std::left
              << std::setw(8)  << "MatchID"
              << std::setw(15) << "Stage"
              << std::setw(12) << "Status"
              << std::setw(10) << "Duration"
              << std::setw(20) << "Timestamp"
              << std::setw(20) << "Players"
              << "Winner\n"
              << std::string(8+15+12+10+20+20+6, '-') << "\n";
    for (int i = 0; i < count; ++i) {
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

void ResultLogger::printPlayerHistory(const std::string& playerID) const {
    int count;
    MatchResult* arr = getPlayerHistory(playerID, count);
    if (!count) {
        std::cout << "\n=== No completed history for Player " << playerID << " ===\n";
        return;
    }
    std::cout << "\n=== Completed History for Player " << playerID
              << " (" << count << " matches) ===\n"
              << std::left
              << std::setw(8)  << "MatchID"
              << std::setw(15) << "Stage"
              << std::setw(12) << "Status"
              << std::setw(20) << "Timestamp"
              << std::setw(20) << "Opponent"
              << "Winner\n"
              << std::string(8+15+12+20+20+6, '-') << "\n";
    for (int i = 0; i < count; ++i) {
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

void ResultLogger::printMatchesOnDate(const std::string& dateStr) const {
    if (dateStr.size() != 8) {
        std::cout << "Invalid date format (expected YYYYMMDD)\n";
        return;
    }
    int year  = std::stoi(dateStr.substr(0,4));
    int month = std::stoi(dateStr.substr(4,2));
    int day   = std::stoi(dateStr.substr(6,2));

    std::vector<MatchResult> matches;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        if (r.status == "Completed"
         && r.timestamp.year  == year
         && r.timestamp.month == month
         && r.timestamp.day   == day)
        {
            matches.push_back(r);
        }
    }

    if (matches.empty()) {
        std::cout << "\n=== No completed matches on "
                  << year << "-" << std::setw(2) << std::setfill('0') << month
                  << "-" << std::setw(2) << std::setfill('0') << day
                  << " ===\n";
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

    for (auto& m : matches) {
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
