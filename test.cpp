#include "ResultLogger.hpp"
#include <iostream>

int main() {
    ResultLogger logger(5);

    // 1) Try loading any existing history
    try {
        logger.loadHistoryFromCSV("match_history.csv");
        std::cout << "Loaded existing history.\n";
    } catch (const std::exception& e) {
        std::cout << "No existing CSV or load error: " << e.what() << "\n";
    }

    // 2) Add a couple of dummy results
    Timestamp t1(2025,5,24,12,0,0);
    MatchResult r1;
    r1.matchID     = 1;
    r1.round       = "Quarterfinal";
    r1.player1ID   = 100; r1.player2ID = 200;
    r1.player1Score= 3;   r1.player2Score = 2;
    r1.winnerID    = 100;
    r1.duration    = 600;
    r1.timestamp   = t1;
    logger.addResult(r1);

    Timestamp t2(2025,5,24,12,10,0);
    MatchResult r2;
    r2.matchID     = 2;
    r2.round       = "Quarterfinal";
    r2.player1ID   = 101; r2.player2ID = 201;
    r2.player1Score= 1;   r2.player2Score = 2;
    r2.winnerID    = 201;
    r2.duration    = 450;
    r2.timestamp   = t2;
    logger.addResult(r2);

    // 3) Print out recent and per-player history
    logger.printRecent(5);
    logger.printPlayerHistory(100);

    // 4) Save out to CSV for persistence
    try {
        logger.saveHistoryToCSV("match_history.csv");
        std::cout << "Saved history to match_history.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "Error saving CSV: " << e.what() << "\n";
    }

    return 0;
}
