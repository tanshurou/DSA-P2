#include "ResultLogger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cctype>

// -----------------------------------------------------------------
// Helpers for drawing ASCII-border tables
// -----------------------------------------------------------------
static void printHorizontalBorder(const int widths[], int cols) {
    std::cout << '+';
    for(int c = 0; c < cols; ++c) {
        for(int i = 0; i < widths[c] + 2; ++i) std::cout << '-';
        std::cout << '+';
    }
    std::cout << "\n";
}

static void printRow(const std::string cells[], const int widths[], int cols) {
    std::cout << '|';
    for(int c = 0; c < cols; ++c) {
        std::cout << ' ' << std::left << std::setw(widths[c]) << cells[c] << ' ' << '|';
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------
// Constructor / Destructor / clearHistory()
// -----------------------------------------------------------------
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
    // 1) Clear full-history linked list
    while(historyHead) {
        ListNode* tmp = historyHead;
        historyHead = historyHead->next;
        delete tmp;
    }
    historyTail = nullptr;

    // 2) Reset circular buffer
    bufferStart = bufferCount = 0;

    // 3) Clear win/loss stats
    playerStats.clear();

    // 4) Clear head-to-head match lists
    for(auto &m1 : head2headList) {
        for(auto &m2 : m1.second) {
            H2HMatchNode* cur = m2.second;
            while(cur) {
                H2HMatchNode* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
    }
    head2headList.clear();
    head2headCount.clear();

    // 5) Clear stage summary
    stageSummary.clear();
}

// -----------------------------------------------------------------
// addResult(...) and related helpers
// -----------------------------------------------------------------
void ResultLogger::addResult(const MatchResult& r) {
    // A) Circular buffer insert
    if(bufferCount < recentMaxSize) {
        recentBuffer[bufferCount++] = r;
    }
    else {
        recentBuffer[bufferStart] = r;
        bufferStart = (bufferStart + 1) % recentMaxSize;
    }

    // B) Append to full-history FIFO
    ListNode* node = new ListNode(r);
    if(!historyTail) {
        historyHead = historyTail = node;
    } else {
        historyTail->next = node;
        historyTail = node;
    }

    // C) If “Completed”, update stats
    if(r.status == "Completed" && r.winnerID != "-1") {
        // 1) Win/Loss for each player
        playerStats[r.winnerID].first++;
        std::string loser = (r.player1ID == r.winnerID ? r.player2ID : r.player1ID);
        playerStats[loser].second++;

        // 2) Stage summary
        auto &ss = stageSummary[r.stage];
        ss.first++;
        ss.second += r.duration;

        // 3) Head-to-head count
        head2headCount[r.winnerID][loser]++;

        // 4) Head-to-head match-ID linked list
        H2HMatchNode* hn = new H2HMatchNode(r.matchID);
        hn->next = head2headList[r.winnerID][loser];
        head2headList[r.winnerID][loser] = hn;
    }
}

MatchResult* ResultLogger::getLastNResults(int n, int& outCount) const {
    MatchResult* tmp = new MatchResult[n];
    int seen = 0;
    for(int i = 0; i < bufferCount && seen < n; ++i) {
        int idx = (bufferStart + bufferCount - 1 - i + recentMaxSize) % recentMaxSize;
        if(recentBuffer[idx].status == "Completed") {
            tmp[seen++] = recentBuffer[idx];
        }
    }
    outCount = seen;
    if(!seen) {
        delete[] tmp;
        return nullptr;
    }
    return tmp;
}

MatchResult* ResultLogger::getPlayerHistory(const std::string& pid, int& outCount) const {
    // Count how many “Completed” matches involve pid
    int cnt = 0;
    for(ListNode* cur = historyHead; cur; cur = cur->next) {
        if(cur->data.status == "Completed" &&
           (cur->data.player1ID == pid || cur->data.player2ID == pid)) {
            cnt++;
        }
    }
    outCount = cnt;
    if(!cnt) return nullptr;

    // Collect them
    MatchResult* arr = new MatchResult[cnt];
    int idx = 0;
    for(ListNode* cur = historyHead; cur; cur = cur->next) {
        if(cur->data.status == "Completed" &&
           (cur->data.player1ID == pid || cur->data.player2ID == pid)) {
            arr[idx++] = cur->data;
        }
    }
    return arr;
}

// -----------------------------------------------------------------
// CSV loader (no header-skip!)
// -----------------------------------------------------------------
void ResultLogger::loadHistoryFromCSV(const std::string& fn) {
    std::ifstream in(fn);
    if(!in.is_open()) throw std::runtime_error("Cannot open " + fn);
    clearHistory();

    std::string line;
    // ** NOTE **: We do NOT skip the first line; the CSV has no header row.
    while(std::getline(in, line)) {
        if(line.empty()) continue;
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

// -----------------------------------------------------------------
// printRecent(...)
// -----------------------------------------------------------------
void ResultLogger::printRecent(int n) const {
    int cnt;
    MatchResult* arr = getLastNResults(n, cnt);
    if(!cnt) {
        std::cout << "\n=== No recent completed matches ===\n";
        return;
    }

    const int widths[7] = {8, 15, 12, 10, 20, 20, 6};
    const std::string header[7] = {
        "MatchID", "Stage", "Status", "Duration", "Timestamp", "Players", "Winner"
    };

    printHorizontalBorder(widths, 7);
    printRow(header, widths, 7);
    printHorizontalBorder(widths, 7);

    for(int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        std::string players = m.player1ID + " vs " + m.player2ID;
        const std::string row[7] = {
            m.matchID,
            m.stage,
            m.status,
            std::to_string(m.duration) + "s",
            ts,
            players,
            m.winnerID
        };
        printRow(row, widths, 7);
    }

    printHorizontalBorder(widths, 7);
    delete[] arr;
}

// -----------------------------------------------------------------
// printMatchesOnDate(...)
// -----------------------------------------------------------------
void ResultLogger::printMatchesOnDate(const std::string& dateStr) const {
    // Date must be exactly 8 digits
    if(dateStr.size() != 8) {
        std::cout << "Invalid date format (expected YYYYMMDD)\n";
        return;
    }
    for(char c : dateStr) {
        if(!std::isdigit(c)) {
            std::cout << "Invalid date format (non-digit detected)\n";
            return;
        }
    }

    int year  = std::stoi(dateStr.substr(0,4));
    int month = std::stoi(dateStr.substr(4,2));
    int day   = std::stoi(dateStr.substr(6,2));

    int cnt = 0;
    for(auto* cur = historyHead; cur; cur = cur->next) {
        const auto& r = cur->data;
        if(r.status == "Completed" &&
           r.timestamp.year == year &&
           r.timestamp.month == month &&
           r.timestamp.day == day) {
            cnt++;
        }
    }
    if(!cnt) {
        std::cout << "\n=== No completed matches on "
                  << year << "-" << std::setw(2) << std::setfill('0') << month
                  << "-" << std::setw(2) << std::setfill('0') << day
                  << " ===\n";
        std::cout << std::setfill(' ');
        return;
    }

    const int widths[7] = {8, 15, 12, 10, 20, 20, 6};
    const std::string header[7] = {
        "MatchID", "Stage", "Status", "Duration", "Timestamp", "Players", "Winner"
    };

    printHorizontalBorder(widths, 7);
    printRow(header, widths, 7);
    printHorizontalBorder(widths, 7);

    for(auto* cur = historyHead; cur; cur = cur->next) {
        const auto& m = cur->data;
        if(m.status == "Completed" &&
           m.timestamp.year == year &&
           m.timestamp.month == month &&
           m.timestamp.day == day) {
            char ts[20];
            std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                         m.timestamp.year, m.timestamp.month, m.timestamp.day,
                         m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
            std::string players = m.player1ID + " vs " + m.player2ID;
            const std::string row[7] = {
                m.matchID,
                m.stage,
                m.status,
                std::to_string(m.duration) + "s",
                ts,
                players,
                m.winnerID
            };
            printRow(row, widths, 7);
        }
    }

    printHorizontalBorder(widths, 7);
}

// -----------------------------------------------------------------
// printStageSummary(...)
// -----------------------------------------------------------------
void ResultLogger::printStageSummary() const {
    if(stageSummary.empty()) {
        std::cout << "\n=== No completed matches to summarize ===\n";
        return;
    }

    size_t w = 5;
    for(const auto& kv : stageSummary) {
        if(kv.first.size() > w) w = kv.first.size();
    }
    int wi = static_cast<int>(w);
    const int widths[3] = { wi, 10, 9 };
    const std::string header[3] = {"Stage", "#Matches", "AvgDur"};

    printHorizontalBorder(widths, 3);
    printRow(header, widths, 3);
    printHorizontalBorder(widths, 3);

    for(const auto& kv : stageSummary) {
        const auto& stage = kv.first;
        int cnt = kv.second.first;
        int tot = kv.second.second;
        int avg = cnt ? tot / cnt : 0;
        const std::string row[3] = {
            stage,
            std::to_string(cnt),
            std::to_string(avg) + "s"
        };
        printRow(row, widths, 3);
    }

    printHorizontalBorder(widths, 3);
}

// -----------------------------------------------------------------
// printPlayerHistory(...)
// -----------------------------------------------------------------
void ResultLogger::printPlayerHistory(const std::string& playerID) const {
    if(playerID.empty()) {
        std::cout << "Player ID cannot be empty.\n";
        return;
    }

    int cnt;
    MatchResult* arr = getPlayerHistory(playerID, cnt);
    if(!cnt) {
        std::cout << "\n=== No completed history for Player " << playerID << " ===\n";
        return;
    }

    const int widths[6] = {8, 15, 12, 20, 20, 6};
    const std::string header[6] = {
        "MatchID", "Stage", "Status", "Timestamp", "Opponent", "Winner"
    };

    printHorizontalBorder(widths, 6);
    printRow(header, widths, 6);
    printHorizontalBorder(widths, 6);

    for(int i = 0; i < cnt; ++i) {
        const auto& m = arr[i];
        char ts[20];
        std::sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d",
                     m.timestamp.year, m.timestamp.month, m.timestamp.day,
                     m.timestamp.hour, m.timestamp.minute, m.timestamp.second);
        std::string opp = (m.player1ID == playerID ? m.player2ID : m.player1ID);
        const std::string row[6] = {
            m.matchID,
            m.stage,
            m.status,
            ts,
            opp,
            m.winnerID
        };
        printRow(row, widths, 6);
    }

    printHorizontalBorder(widths, 6);
    delete[] arr;
}

// -----------------------------------------------------------------
// printHeadToHead(...)  (concise, single-block summary)
// -----------------------------------------------------------------
void ResultLogger::printHeadToHead(const std::string& A, const std::string& B) const {
    if(A.empty() || B.empty()) {
        std::cout << "Player IDs cannot be empty.\n";
        return;
    }

    int Awin = 0, Bwin = 0;

    // 1) Find how many times A beat B:
    auto itA = head2headCount.find(A);
    if (itA != head2headCount.end()) {
        auto itAB = itA->second.find(B);
        if (itAB != itA->second.end()) {
            Awin = itAB->second;
        }
    }

    // 2) Find how many times B beat A:
    auto itB = head2headCount.find(B);
    if (itB != head2headCount.end()) {
        auto itBA = itB->second.find(A);
        if (itBA != itB->second.end()) {
            Bwin = itBA->second;
        }
    }

    std::cout << "\n=== Head-to-Head: " << A << " vs " << B << " ===\n";
    if (Awin == 0 && Bwin == 0) {
        std::cout << "No head-to-head matches found between "
                  << A << " and " << B << ".\n";
        return;
    }

    // 3) Compute totals and percentages
    int total = Awin + Bwin;
    double pctA = 100.0 * Awin / total;
    double pctB = 100.0 * Bwin / total;

    std::cout << "Total matches played: " << total << "\n"
              << A << " wins: " << Awin
              << " (" << std::fixed << std::setprecision(1) << pctA << "%)\n"
              << B << " wins: " << Bwin
              << " (" << std::fixed << std::setprecision(1) << pctB << "%)\n";

    // 4) Gather all head-to-head match IDs
    std::string allMatches;

    // 4a) Match IDs where A beat B
    auto la = head2headList.find(A);
    if (la != head2headList.end()) {
        auto lab = la->second.find(B);
        if (lab != la->second.end()) {
            for (H2HMatchNode* cur = lab->second; cur; cur = cur->next) {
                allMatches += cur->matchID;
                if (cur->next) allMatches += ", ";
            }
        }
    }

    // 4b) Match IDs where B beat A
    auto lb = head2headList.find(B);
    if (lb != head2headList.end()) {
        auto lba = lb->second.find(A);
        if (lba != lb->second.end()) {
            if (!allMatches.empty()) allMatches += ", ";
            for (H2HMatchNode* cur = lba->second; cur; cur = cur->next) {
                allMatches += cur->matchID;
                if (cur->next) allMatches += ", ";
            }
        }
    }

    std::cout << "Match IDs: " << allMatches << "\n";
}

// -----------------------------------------------------------------
// Free-function “runResultLogger(…)” with input validation
// -----------------------------------------------------------------
void runResultLogger(const std::string& csvPath, int recentSize) {
    ResultLogger logger(recentSize);

    // Attempt to load CSV
    try {
        logger.loadHistoryFromCSV(csvPath);
    }
    catch(const std::runtime_error& e) {
        std::cerr << "Error loading CSV: " << e.what() << "\n";
        return;
    }

    // Helper to read an integer choice in [min..max], returns -1 on EOF
    auto readIntInRange = [&](int min, int max) -> int {
        while (true) {
            std::string line;
            if (!std::getline(std::cin, line)) {
                // EOF encountered
                return -1;
            }
            std::istringstream iss(line);
            int choice;
            if (iss >> choice && choice >= min && choice <= max) {
                return choice;
            }
            std::cout << "Please enter a valid choice (" << min << "-" << max << "): ";
        }
    };

    // Helper to read a non-empty string (returns empty on EOF)
    auto readNonEmptyString = [&]() -> std::string {
        while (true) {
            std::string s;
            if (!std::getline(std::cin, s)) {
                return std::string(); // EOF
            }
            if (!s.empty()) return s;
            std::cout << "Input cannot be empty, please try again: ";
        }
    };

    // --- Main interactive loop ---
    while (true) {
        std::cout << "\n===Results & History===\n"
                  << "1) Match Info\n"
                  << "2) Player Info\n"
                  << "3) Exit\n"
                  << "Choice: ";
        int choice = readIntInRange(1, 3);
        if (choice < 0) break; // EOF

        if (choice == 1) {
            // Match Info submenu
            while (true) {
                std::cout << "\n--- Match Info ---\n"
                          << "1) Last " << recentSize << " matches\n"
                          << "2) Matches on date\n"
                          << "3) Stage summary\n"
                          << "4) Back\n"
                          << "Choice: ";
                int m = readIntInRange(1, 4);
                if (m < 0) return; // EOF => exit entire program

                if (m == 1) {
                    logger.printRecent(recentSize);
                }
                else if (m == 2) {
                    std::cout << "Enter date (YYYYMMDD): ";
                    std::string date = readNonEmptyString();
                    if (date.empty()) return; // EOF
                    logger.printMatchesOnDate(date);
                }
                else if (m == 3) {
                    logger.printStageSummary();
                }
                else { // m == 4
                    break; // back to Main Menu
                }
            }
        }
        else if (choice == 2) {
            // Player Info submenu
            while (true) {
                std::cout << "\n--- Player Info ---\n"
                          << "1) Player history\n"
                          << "2) Head-to-Head\n"
                          << "3) Back\n"
                          << "Choice: ";
                int p = readIntInRange(1, 3);
                if (p < 0) return; // EOF => exit

                if (p == 1) {
                    std::cout << "Enter player ID: ";
                    std::string pid = readNonEmptyString();
                    if (pid.empty()) return;
                    logger.printPlayerHistory(pid);
                }
                else if (p == 2) {
                    std::cout << "Player A: ";
                    std::string A = readNonEmptyString();
                    if (A.empty()) return;
                    std::cout << "Player B: ";
                    std::string B = readNonEmptyString();
                    if (B.empty()) return;
                    logger.printHeadToHead(A, B);
                }
                else { // p == 3
                    break; // back to Main Menu
                }
            }
        }
        else { // choice == 3
            break; // Exit program
        }
    }
}
