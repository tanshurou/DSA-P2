#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include "PlayerRegistration.hpp"

using namespace std;

void displayMenu() {
    cout << "\n========== Tournament Registration System ==========\n";
    cout << "1. Display check-in queue\n";
    cout << "2. Player check-in (existing)\n";
    cout << "3. Withdraw player from tournament\n";
    cout << "4. Process next player in queue\n";
    cout << "5. Display ready matches\n";
    cout << "0. Exit program\n";
    cout << "========================================================\n";
    cout << "Enter your choice: ";
}

bool equalsIgnoreCase(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower(*a) != tolower(*b)) return false;
        a++;
        b++;
    }
    return *a == *b;
}

void PlayerRegistration::setPlayerPriority(const char* playerID, char priorityType) {
    for (int i = 0; i < totalPlayers; i++) {
        if (equalsIgnoreCase(players[i].playerID, playerID)) {
            players[i].isEarlyBird = (priorityType == 'E' || priorityType == 'e');
            players[i].isWildcard = (priorityType == 'W' || priorityType == 'w');
            return;
        }
    }
}

int main() {
    PlayerRegistration reg;
    string playerID;
    int choice;

    // Preload players from file (since option removed)
    reg.loadPlayers("data/players.csv");

    // Optional: ensure valid data before proceeding
    if (!reg.validatePlayerCount()) {
        return 1;
    }

    do {
        displayMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                reg.displayCheckInQueue();
                break;
            }
            case 2: {
                cout << "Enter Player ID to check in: ";
                getline(cin, playerID);

                char priority;
                cout << "Is this player an Early-Bird (E), Wildcard (W), or Regular (R)? ";
                cin >> priority;
                cin.ignore();

                reg.setPlayerPriority(playerID.c_str(), priority);

                reg.checkInPlayer(playerID.c_str());
                break;
            }
            case 3: {
                cout << "Enter Player ID to withdraw: ";
                getline(cin, playerID);
                reg.withdrawPlayer(playerID.c_str());
                break;
            }
            case 4: {
                reg.processCheckInQueue();
                break;
            }
            case 5: {
                reg.displayReadyMatches();
                break;
            }
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid option. Try again.\n";
        }
    } while (choice != 0);

    // Save on exit (optional)
    reg.savePlayers("data/players.csv");

    return 0;
}

PlayerRegistration::PlayerRegistration() {
    totalPlayers = 0;
    front = rear = nullptr;
}

PlayerRegistration::~PlayerRegistration() {
    while (front) {
        QueueNode* temp = front;
        front = front->next;
        delete temp;
    }
}

void PlayerRegistration::loadPlayers(const char* filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Error opening file.\n";
        return;
    }

    string line;
    totalPlayers = 0;

    while (getline(file, line) && totalPlayers < 100) {
        stringstream ss(line);
        string token;
        Player& p = players[totalPlayers];

        getline(ss, token, ','); strcpy(p.playerID, token.c_str());
        getline(ss, token, ','); strcpy(p.name, token.c_str());
        getline(ss, token, ','); strcpy(p.university, token.c_str());
        getline(ss, token, ','); p.ranking = stoi(token);
        getline(ss, token, ','); strcpy(p.status, token.c_str());
        getline(ss, token, ','); p.matchesPlayed = stoi(token);
        getline(ss, token, ','); p.points = stoi(token);
        getline(ss, token, ','); strcpy(p.grouping, token.c_str());

        totalPlayers++;
    }

    file.close();
}

void PlayerRegistration::savePlayers(const char* filename) {
    ofstream file(filename);
    for (int i = 0; i < totalPlayers; i++) {
        Player& p = players[i];
        file << p.playerID << ","
             << p.name << ","
             << p.university << ","
             << p.ranking << ","
             << p.status << ","
             << p.matchesPlayed << ","
             << p.points << ","
             << p.grouping << "\n";
    }
    file.close();
}

void PlayerRegistration::displayAllPlayers() {
    for (int i = 0; i < totalPlayers; i++) {
        cout << players[i].playerID << " - " << players[i].name << " (" << players[i].status << ")\n";
    }
}

void PlayerRegistration::checkInPlayer(const char* playerID) {
    for (int i = 0; i < totalPlayers; i++) {
        if (equalsIgnoreCase(players[i].playerID, playerID)) {
            if (equalsIgnoreCase(players[i].status, "active")) {
                cout << "Player already active.\n";
                return;
            }

            strcpy(players[i].status, "active");
            QueueNode* newNode = new QueueNode{ &players[i], nullptr };

            // Priority insertion: Early-bird > Wildcard > Regular
            if (players[i].isEarlyBird) {
                newNode->next = front;
                front = newNode;
                if (!rear) rear = newNode;
                cout << "[EarlyBird] ";
            } else if (players[i].isWildcard) {
                // Insert after the last early-bird
                QueueNode* prev = nullptr;
                QueueNode* curr = front;
                while (curr && curr->player->isEarlyBird) {
                    prev = curr;
                    curr = curr->next;
                }
                if (!prev) {
                    newNode->next = front;
                    front = newNode;
                    if (!rear) rear = newNode;
                } else {
                    newNode->next = curr;
                    prev->next = newNode;
                    if (!curr) rear = newNode;
                }
                cout << "[Wildcard] ";
            } else {
                if (!rear) {
                    front = rear = newNode;
                } else {
                    rear->next = newNode;
                    rear = newNode;
                }
            }

            cout << "Checked in: " << players[i].playerID << " - " << players[i].name << endl;
            return;
        }
    }

    cout << "Player not found.\n";
}

void PlayerRegistration::processCheckInQueue() {
    if (!front) {
        cout << "Queue is empty.\n";
        return;
    }

    Player* p = front->player;
    cout << "Processing player: " << p->playerID << " - " << p->name << "\n";

    QueueNode* temp = front;
    front = front->next;
    if (!front) rear = nullptr;
    delete temp;
}

void PlayerRegistration::withdrawPlayer(const char* playerID) {
    for (int i = 0; i < totalPlayers; i++) {
        if (equalsIgnoreCase(players[i].playerID, playerID)) {
            strcpy(players[i].status, "eliminated");
            cout << "Player " << playerID << " withdrawn.\n";
            return;
        }
    }
    cout << "Player not found.\n";
}

void PlayerRegistration::displayCheckInQueue() {
    if (!front) {
        cout << "Check-in queue is empty.\n";
        return;
    }

    cout << "Check-in Queue:\n";
    QueueNode* temp = front;
    while (temp) {
        cout << temp->player->playerID << " - " << temp->player->name << endl;
        temp = temp->next;
    }
}

void PlayerRegistration::displayReadyMatches() {
    if (!front || !front->next) {
        cout << "Not enough players to display matches.\n";
        return;
    }

    QueueNode* temp = front;
    int matchNum = 1;

    cout << "\n--- Ready Matches ---\n";
    while (temp && temp->next) {
        cout << "Match " << matchNum++ << ": "
             << temp->player->name << " vs " << temp->next->player->name << "\n";
        temp = temp->next->next;
    }

    if (temp) {
        cout << "Waiting for match: " << temp->player->name << " (no opponent yet)\n";
    }
}

bool PlayerRegistration::validatePlayerCount() {
    if (totalPlayers < 8) {
        cout << "Error: At least 8 players are required.\n";
        return false;
    }
    if (totalPlayers % 4 != 0) {
        cout << "Error: Total number of players must be divisible by 4.\n";
        return false;
    }
    return true;
}
