#include "ResultLogger.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
  #include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::string csvPath = (argc > 1 ? argv[1] : "data/match_history.csv");
    ResultLogger logger(5);

    try {
        logger.loadHistoryFromCSV(csvPath);
        std::cout << "Loaded existing history from " << csvPath << "\n";
    } catch (const std::exception& e) {
        std::cout << "No existing CSV or load error: " << e.what() << "\n";
    }

    // example data
    {
        Timestamp t1(2025,5,24,12,0,0);
        MatchResult r1{1,"Quarterfinal",100,200,3,2,100,600,t1};
        logger.addResult(r1);

        Timestamp t2(2025,5,24,12,10,0);
        MatchResult r2{2,"Quarterfinal",101,201,1,2,201,450,t2};
        logger.addResult(r2);
    }

    logger.printRecent(5);

    int playerID;
    std::cout << "\nEnter player ID to view history: ";
    if (!(std::cin >> playerID)) {
        std::cerr << "Invalid input. Exiting.\n";
        return 1;
    }
    logger.printPlayerHistory(playerID);

    try {
        logger.saveHistoryToCSV(csvPath);
        std::cout << "\nSaved history to " << csvPath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error saving CSV: " << e.what() << "\n";
    }

    return 0;
}
