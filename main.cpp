#include <iostream>
#include <string>
#include <limits>

#include "MatchScheduler.hpp"
#include "PlayerRegistration.hpp"
#include "ResultLogger.hpp"
#include "SpectatorManager.hpp"

using namespace std;

void showMainMenu();
void runSpectatorManager();

int main()
{
    std::cout << "============================================" << std::endl;
    std::cout << "ASIA PACIFIC UNIVERSITY ESPORTS CHAMPIONSHIP" << std::endl;
    std::cout << "          MANAGEMENT SYSTEM                " << std::endl;
    std::cout << "============================================" << std::endl;

    int choice = 0;
    SpectatorManager manager;

    do
    {
        showMainMenu();
        std::cout << "\nEnter your choice (1-5): ";
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
            case 1:
                runTournamentManager();
                break;

            case 2:
                runPlayerRegistration();
                break;

            case 3:
                runSpectatorManager();
                break;

            case 4:
                runResultLogger("data/matches.csv", 10);
                break;

            case 5:
                std::cout << "Exiting system. Goodbye!\n";
                break;

            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    } while (choice != 5);

    return 0;
}

void showMainMenu()
{
    std::cout << "\n========== MAIN MENU ==========" << std::endl;
    std::cout << "1. Match Scheduling & Player Progression" << std::endl;
    std::cout << "2. Tournament Registration & Player Queueing" << std::endl;
    std::cout << "3. Live Stream & Spectator Queue Management" << std::endl;
    std::cout << "4. Game Result Logging & Performance History" << std::endl;
    std::cout << "5. Exit" << std::endl;
}

void runSpectatorManager()
{
    SpectatorManager manager;
    manager.run();
}
