#include <iostream>
#include <string>
#include <limits>

#include "MatchScheduler.hpp" // Task 1
// #include "PlayerRegistration.hpp" // Task 2
// #include "SpectatorManager.hpp"   // Task 3
// #include "ResultLogger.hpp"       // Task 4

using namespace std;

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
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            runTournamentManager();
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 5);

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