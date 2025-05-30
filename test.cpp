// test.cpp
#include "ResultLogger.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string csvPath = (argc>1? argv[1] : "data/matches.csv");
    ResultLogger logger(10);

    try {
        logger.loadHistoryFromCSV(csvPath);
    } catch (const std::runtime_error& e) {
        std::cerr << "CSV load error: " << e.what() << "\n";
        return 1;
    }

    while (true) {
        std::cout << "\n=== Menu ===\n"
                  << "1) Last 10 matches\n"
                  << "2) Player history\n"
                  << "3) Matches on date\n"
                  << "4) Head-to-Head\n"
                  << "5) Stage Summary\n"
                  << "6) Exit\n"
                  << "Choice: ";
        int c; if (!(std::cin>>c)) break;

        if (c == 1) {
            logger.printRecent(10);
        }
        else if (c == 2) {
            std::string p; 
            std::cout<<"Player ID: "; std::cin>>p;
            logger.printPlayerHistory(p);
        }
        else if (c == 3) {
            std::string d;
            std::cout<<"Date (YYYYMMDD): "; std::cin>>d;
            logger.printMatchesOnDate(d);
        }
        else if (c == 4) {
            std::string A,B;
            std::cout<<"Player A: "; std::cin>>A;
            std::cout<<"Player B: "; std::cin>>B;
            logger.printHeadToHead(A,B);
        }
        else if (c == 5) {
            logger.printStageSummary();
        }
        else if (c == 6) {
            break;
        }
        else {
            std::cout<<"Invalid choice.\n";
        }
    }
    return 0;
}
