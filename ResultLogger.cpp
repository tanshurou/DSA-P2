#include "ResultLogger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>

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
    // clear ring buffer
    bufferStart = 0;
    bufferCount = 0;
    // clear full history linked list
    while (historyHead) {
        ListNode* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;
}

// ------------------------
// Core Functionality
// ------------------------

void ResultLogger::addResult(const MatchResult& r) {
    // 1) push into ring buffer
    if (bufferCount < recentMaxSize) {
        // still space: place at end
        int idx = (bufferStart + bufferCount) % recentMaxSize;
        recentBuffer[idx] = r;
        ++bufferCount;
    } else {
        // full: overwrite oldest, advance start
        recentBuffer[bufferStart] = r;
        bufferStart = (bufferStart + 1) % recentMaxSize;
    }

    // 2) append to full-history linked list
    ListNode* node = new ListNode(r);
    if (!historyTail) {
        historyHead = historyTail = node;
    } else {
        historyTail->next = node;
        historyTail = node;
    }
}

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) const {
    outCount = std::min(n, bufferCount);
    if (outCount == 0) return nullptr;
    MatchResult* arr = new MatchResult[outCount];

    // newest is at index (start + count - 1) % max, then go backwards
    for (int i = 0; i < outCount; ++i) {
        int idx = (bufferStart + bufferCount - 1 - i + recentMaxSize)
                  % recentMaxSize;
        arr[i] = recentBuffer[idx];
    }
    return arr;
}

MatchResult* ResultLogger::getPlayerHistory(int playerID, int& outCount) const {
    // count matching entries in full history
    int cnt = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.player1ID==playerID || cur->data.player2ID==playerID)
            ++cnt;
    }
    outCount = cnt;
    if (cnt == 0) return nullptr;

    // collect them in chronological order
    MatchResult* arr = new MatchResult[cnt];
    int idx = 0;
    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        if (cur->data.player1ID==playerID || cur->data.player2ID==playerID)
            arr[idx++] = cur->data;
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
        std::string ts_str;
        char comma;

        std::getline(ss, line, ','); r.matchID    = std::stoi(line);
        std::getline(ss, r.round,   ',');
        std::getline(ss, line, ',' ); r.player1ID    = std::stoi(line);
        std::getline(ss, line, ',' ); r.player2ID    = std::stoi(line);
        std::getline(ss, line, ',' ); r.player1Score = std::stoi(line);
        std::getline(ss, line, ',' ); r.player2Score = std::stoi(line);
        std::getline(ss, line, ',' ); r.winnerID     = std::stoi(line);
        std::getline(ss, line, ',' ); r.duration     = std::stoi(line);
        std::getline(ss, ts_str);

        std::replace(ts_str.begin(), ts_str.end(), 'T', '-');
        std::replace(ts_str.begin()+10, ts_str.end(), ':', '-');
        std::istringstream ts_in(ts_str);
        ts_in  >> r.timestamp.year
               >> comma >> r.timestamp.month
               >> comma >> r.timestamp.day
               >> comma >> r.timestamp.hour
               >> comma >> r.timestamp.minute
               >> comma >> r.timestamp.second;

        // reuse addResult to feed both buffer & list
        addResult(r);
    }
    in.close();
}

void ResultLogger::saveHistoryToCSV(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open())
        throw std::runtime_error("Cannot open " + filename + " for writing");

    out << "match_id,round,player1_id,player2_id,"
           "player1_score,player2_score,winner_id,"
           "duration_secs,timestamp\n";

    auto fmtTS = [&](const Timestamp& t){
        std::ostringstream s;
        s << std::setw(4)<<std::setfill('0')<<t.year << '-'
          << std::setw(2)<<std::setfill('0')<<t.month<< '-'
          << std::setw(2)<<std::setfill('0')<<t.day  << 'T'
          << std::setw(2)<<std::setfill('0')<<t.hour << ':'
          << std::setw(2)<<std::setfill('0')<<t.minute<< ':'
          << std::setw(2)<<std::setfill('0')<<t.second;
        return s.str();
    };

    for (ListNode* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        out
          << r.matchID<<','<<r.round<<','<<r.player1ID<<','<<r.player2ID<<','
          << r.player1Score<<','<<r.player2Score<<','<<r.winnerID<<','
          << r.duration<<','<<fmtTS(r.timestamp)<<'\n';
    }
    out.close();
}

// ------------------------
// Pretty-Printing (unchanged from before)
// ------------------------

void ResultLogger::printRecent(int n) const {
    int count;
    auto arr = getLastNResults(n, count);
    if (!count) {
        std::cout << "\n=== No recent matches to show ===\n";
        return;
    }

    std::cout << "\n=== Last " << count << " Matches ===\n"
              << std::left
              << std::setw(4)  << "ID"
              << std::setw(15) << "Round"
              << std::setw(10) << "Duration"
              << std::setw(20) << "Timestamp"
              << std::setw(25) << "Players (score)"
              << "Winner\n"
              << std::string(4+15+10+20+25+6, '-') << "\n";

    for (int i = 0; i < count; ++i) {
        const auto& m = arr[i];
        std::ostringstream ts;
        ts << std::setw(4)<<std::setfill('0')<<m.timestamp.year << '-'
           << std::setw(2)<<m.timestamp.month << '-'
           << std::setw(2)<<m.timestamp.day   << ' '
           << std::setw(2)<<m.timestamp.hour  << ':'
           << std::setw(2)<<m.timestamp.minute<< ':'
           << std::setw(2)<<m.timestamp.second;
        std::string players =
            std::to_string(m.player1ID) + "(" + std::to_string(m.player1Score) + ") vs " +
            std::to_string(m.player2ID) + "(" + std::to_string(m.player2Score) + ")";

        std::cout << std::left
                  << std::setw(4)  << m.matchID
                  << std::setw(15) << m.round
                  << std::setw(10) << (std::to_string(m.duration) + "s")
                  << std::setw(20) << ts.str()
                  << std::setw(25) << players
                  << m.winnerID
                  << "\n";
    }
    delete[] arr;
}

void ResultLogger::printPlayerHistory(int playerID) const {
    int count;
    auto arr = getPlayerHistory(playerID, count);
    if (!count) {
        std::cout << "\n=== No history for Player " << playerID << " ===\n";
        return;
    }

    std::cout << "\n=== History for Player " << playerID
              << " (" << count << " matches) ===\n"
              << std::left
              << std::setw(4)  << "ID"
              << std::setw(15) << "Round"
              << std::setw(8)  << "Result"
              << std::setw(10) << "Opponent"
              << std::setw(8)  << "Score"
              << "Timestamp\n"
              << std::string(4+15+8+10+8+10, '-') << "\n";

    for (int i = 0; i < count; ++i) {
        const auto& m = arr[i];
        bool win = (m.winnerID == playerID);
        int  opp = (m.player1ID == playerID ? m.player2ID : m.player1ID);
        int  sc  = (m.player1ID == playerID ? m.player1Score : m.player2Score);
        int  os  = (m.player1ID == playerID ? m.player2Score : m.player1Score);
        std::string result = win ? "Win" : "Loss";
        std::string score  = std::to_string(sc) + "-" + std::to_string(os);

        std::ostringstream ts;
        ts << std::setw(4)<<std::setfill('0')<<m.timestamp.year << '-'
           << std::setw(2)<<m.timestamp.month << '-'
           << std::setw(2)<<m.timestamp.day   << ' '
           << std::setw(2)<<m.timestamp.hour  << ':'
           << std::setw(2)<<m.timestamp.minute<< ':'
           << std::setw(2)<<m.timestamp.second;

        std::cout << std::left
                  << std::setw(4)  << m.matchID
                  << std::setw(15) << m.round
                  << std::setw(8)  << result
                  << std::setw(10) << opp
                  << std::setw(8)  << score
                  << ts.str()
                  << "\n";
    }
    delete[] arr;
}
