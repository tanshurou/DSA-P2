// test.cpp
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
        std::cout << "Loaded history from " << csvPath << "\n";
    } catch (const std::exception& e) {
        std::cout << "Could not load CSV: " << e.what() << "\n";
    }

    // sample entries
    logger.addResult({"M001","Qualifiers","P001","P002","P001","Completed",
                      1500,{2025,5,20,9,0,0}});
    logger.addResult({"M002","Qualifiers","P003","P004","-1","Scheduled",
                      1800,{2025,5,20,10,0,0}});
    logger.addResult({"M003","Group stage","P001","P003","P003","Completed",
                      1700,{2025,5,21,11,0,0}});

    // 1) recent completed matches
    logger.printRecent(5);

    // 2) history by player
    std::string playerID;
    std::cout << "\nEnter player ID to view history: ";
    std::cin >> playerID;
    logger.printPlayerHistory(playerID);

    // 3) matches by date
    std::string dateStr;
    std::cout << "\nEnter date (YYYYMMDD) to view matches: ";
    std::cin >> dateStr;
    logger.printMatchesOnDate(dateStr);

    // 4) save back
    try {
        logger.saveHistoryToCSV(csvPath);
        std::cout << "\nSaved history to " << csvPath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error saving CSV: " << e.what() << "\n";
    }

    return 0;
}
