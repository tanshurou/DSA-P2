// main.cpp
#include <iostream>
#include <string>
#include <limits>

#include "MatchScheduler.hpp"   // Task 1: runTournamentManager()
#include "ResultLogger.hpp"     // Task 4: runResultLogger()

using namespace std;

// Forward‐declare the tournament manager entry point:
void runTournamentManager();

void showMainMenu();

int main()
{
    cout << "============================================" << endl;
    cout << "ASIA PACIFIC UNIVERSITY ESPORTS CHAMPIONSHIP" << endl;
    cout << "          MANAGEMENT SYSTEM                " << endl;
    cout << "============================================" << endl;

    int choice = 0;
    do
    {
        showMainMenu();
        cout << "\nEnter your choice (1-5): ";
        if (!(cin >> choice))
        {
            // If the user types a non-number, clear and re-prompt
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }
        // Remove leftover newline from the buffer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
            case 1:
                // Launch your Tournament Manager (MatchScheduler, PlayerRegistration, etc.)
                runTournamentManager();
                break;

            case 2:
                // (If you eventually implement a “PlayerRegistration” module, call it here)
                // e.g. runPlayerRegistration();
                cout << "Player Registration not yet implemented.\n";
                break;

            case 3:
                // (If you eventually implement a “SpectatorManager” module, call it here)
                // e.g. runSpectatorManager();
                cout << "Spectator Management not yet implemented.\n";
                break;

            case 4:
                // Launch the Result Logger submenu from ResultLogger.cpp / ResultLogger.hpp
                runResultLogger("data/results.csv", 10);
                break;

            case 5:
                // Exit
                cout << "Exiting system. Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    while (choice != 5);

    return 0;
}

void showMainMenu()
{
    cout << "\n========== MAIN MENU ==========" << endl;
    cout << "1. Match Scheduling & Player Progression" << endl;
    cout << "2. Tournament Registration & Player Queueing" << endl;
    cout << "3. Live Stream & Spectator Queue Management" << endl;
    cout << "4. Game Result Logging & Performance History" << endl;
    cout << "5. Exit" << endl;
}
