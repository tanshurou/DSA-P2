// main.cpp
// Main program for Asia Pacific University Esports Championship Management System

#include <iostream>
#include <string>

// Include all component headers
#include "MatchScheduler.hpp" // Task 1
#include "PlayerRegistration.hpp"     // Task 2
#include "SpectatorManager.hpp"       // Task 3
// #include "ResultLogger.hpp"           // Task 4

// Function prototypes for menu options
void showMainMenu();
void handleMatchScheduling(MatchScheduler &scheduler, PlayerRegistration &playerReg);
void handlePlayerRegistration(PlayerRegistration &playerReg);
void handleSpectatorManagement(SpectatorManager &specManager);
void handleResultLogging(ResultLogger &logger, MatchScheduler &scheduler);

int main()
{
    // Initialize the components
    MatchScheduler matchScheduler;
    PlayerRegistration playerReg;
    SpectatorManager specManager;
    ResultLogger resultLogger;

    // Try to load existing data from files
    playerReg.loadPlayersFromFile("data/players.csv");
    matchScheduler.loadMatchesFromFile("data/matches.csv");
    resultLogger.loadResultsFromFile("data/results.csv");

    std::cout << "============================================" << std::endl;
    std::cout << "ASIA PACIFIC UNIVERSITY ESPORTS CHAMPIONSHIP" << std::endl;
    std::cout << "          MANAGEMENT SYSTEM                " << std::endl;
    std::cout << "============================================" << std::endl;

    int choice = 0;
    do
    {
        showMainMenu();
        std::cout << "\nEnter your choice (1-5): ";
        std::cin >> choice;

        // Clear the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            handleMatchScheduling(matchScheduler, playerReg);
            break;
        case 2:
            handlePlayerRegistration(playerReg);
            break;
        case 3:
            handleSpectatorManagement(specManager);
            break;
        case 4:
            handleResultLogging(resultLogger, matchScheduler);
            break;
        case 5:
            std::cout << "Exiting system. Saving data..." << std::endl;
            // Save data before exiting
            playerReg.savePlayersToFile("data/players.csv");
            matchScheduler.saveMatchesToFile("data/matches.csv");
            resultLogger.saveResultsToFile("data/results.csv");
            std::cout << "Data saved successfully. Goodbye!" << std::endl;
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

// Implementation of menu handlers would go here...
// Each function would provide a submenu for the specific component
// and handle user interactions with that component

void handleMatchScheduling(MatchScheduler &scheduler, PlayerRegistration &playerReg)
{
    // Implementation for match scheduling submenu
    int choice = 0;
    do
    {
        std::cout << "\n===== MATCH SCHEDULING MENU =====" << std::endl;
        std::cout << "1. Schedule a new match" << std::endl;
        std::cout << "2. View upcoming matches" << std::endl;
        std::cout << "3. Generate tournament brackets" << std::endl;
        std::cout << "4. Update tournament progress" << std::endl;
        std::cout << "5. Return to main menu" << std::endl;

        std::cout << "\nEnter your choice (1-5): ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
        {
            // Schedule a new match
            std::string matchID, player1ID, player2ID;

            std::cout << "Enter Match ID: ";
            std::getline(std::cin, matchID);

            // In a real implementation, you would list available players
            playerReg.displayRegisteredPlayers();

            std::cout << "Enter Player 1 ID: ";
            std::getline(std::cin, player1ID);

            std::cout << "Enter Player 2 ID: ";
            std::getline(std::cin, player2ID);

            scheduler.scheduleMatch(matchID, player1ID, player2ID);
            break;
        }
        case 2:
            // View upcoming matches
            scheduler.displayUpcomingMatches();
            break;
        case 3:
        {
            // Generate tournament brackets
            // In a real implementation, you would use actual player data
            std::cout << "Generating tournament brackets..." << std::endl;
            Player *playerList = nullptr; // This would be properly implemented
            int playerCount = playerReg.getPlayerCount();
            scheduler.generateBrackets(playerList, playerCount);
            break;
        }
        case 4:
        {
            // Update tournament progress
            std::string matchID, winnerID;

            scheduler.displayUpcomingMatches();

            std::cout << "Enter Match ID to update: ";
            std::getline(std::cin, matchID);

            std::cout << "Enter Winner ID: ";
            std::getline(std::cin, winnerID);

            scheduler.updateTournamentProgress(matchID, winnerID);
            break;
        }
        case 5:
            std::cout << "Returning to main menu..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    } while (choice != 5);
}

void handleSpectatorManagement(SpectatorManager &specManager)
{
    int choice = 0;
    do
    {
        std::cout << "\n===== SPECTATOR MANAGEMENT MENU =====" << std::endl;
        std::cout << "1. Add VIP to queue" << std::endl;
        std::cout << "2. Add general spectator to queue" << std::endl;
        std::cout << "3. Assign streamer slot" << std::endl;
        std::cout << "4. View current queues" << std::endl;
        std::cout << "5. View streamer slots" << std::endl;
        std::cout << "6. Return to main menu" << std::endl;

        std::cout << "\nEnter your choice (1-6): ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
        {
            std::string name;
            std::cout << "Enter VIP name: ";
            std::getline(std::cin, name);
            specManager.addVIP(name);
            break;
        }
        case 2:
        {
            std::string name;
            std::cout << "Enter general spectator name: ";
            std::getline(std::cin, name);
            specManager.addGeneralSpectator(name);
            break;
        }
        case 3:
        {
            int slot;
            std::string name;
            std::cout << "Enter streamer name: ";
            std::getline(std::cin, name);
            std::cout << "Enter slot number (0-4): ";
            std::cin >> slot;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            specManager.assignStreamerSlot(slot, name);
            break;
        }
        case 4:
            specManager.displayQueues();
            break;
        case 5:
            specManager.displayStreamerSlots();
            break;
        case 6:
            std::cout << "Returning to main menu..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }

    } while (choice != 6);
}


// Additional handler functions would be implemented similarly