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

    std::string csvPath = (argc > 1 ? argv[1] : "data/matches.csv");
    ResultLogger logger(10);

    try {
        logger.loadHistoryFromCSV(csvPath);
    } catch (const std::exception& e) {
        std::cerr << "Error loading CSV: " << e.what() << "\n";
        return 1;
    }

    while (true) {
        std::cout << "\n=== Tournament Logger Menu ===\n"
                  << "1) Show last 5 completed matches\n"
                  << "2) Show history for a player\n"
                  << "3) Show matches on a date\n"
                  << "4) Exit\n"
                  << "Select an option (1-4): ";
        int choice;
        if (!(std::cin >> choice)) break;

        if (choice == 1) {
            logger.printRecent(10);
        } else if (choice == 2) {
            std::string pid;
            std::cout << "Enter player ID: ";
            std::cin >> pid;
            logger.printPlayerHistory(pid);
        } else if (choice == 3) {
            std::string date;
            std::cout << "Enter date (YYYYMMDD): ";
            std::cin >> date;
            logger.printMatchesOnDate(date);
        } else if (choice == 4) {
            std::cout << "Exiting.\n";
            break;
        } else {
            std::cout << "Invalid selection.\n";
        }
    }

    return 0;
}
