#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include "PlayerRegistration.hpp"

using namespace std;

int main() {
    PlayerRegistration reg;

    // Load players from CSV
    reg.loadPlayers("data/players.csv");

    // Validate player count (must be at least 8 and divisible by 4)
    if (!reg.validatePlayerCount()) {
        return 1;
    }

    cout << "\n--- All Registered Players ---\n";
    reg.displayAllPlayers();

    // Simulate some check-ins
    cout << "\n--- Player Check-Ins ---\n";
    reg.checkInPlayer("P002");  // assume early-bird
    reg.checkInPlayer("P003");  // assume wildcard
    reg.checkInPlayer("P005");  // regular
    reg.checkInPlayer("P006");
    reg.checkInPlayer("P001");

    // Display current queue
    cout << "\n--- Current Check-In Queue ---\n";
    reg.displayCheckInQueue();

    // Process one check-in (simulate queue advancement)
    cout << "\n--- Processing One Check-In ---\n";
    reg.processCheckInQueue();

    // Show queue after processing
    cout << "\n--- Queue After Processing ---\n";
    reg.displayCheckInQueue();

    // Withdraw a player
    cout << "\n--- Withdraw Player P003 ---\n";
    reg.withdrawPlayer("P003");

    // Display players after withdrawal
    cout << "\n--- Players After Withdrawal ---\n";
    reg.displayAllPlayers();

    // Save updated player list
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
        file << p.playerID << " " << p.name << " " << p.university << " "
             << p.ranking << " " << p.status << " "
             << p.matchesPlayed << " " << p.points << " " << p.grouping << "\n";
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
        if (strcmp(players[i].playerID, playerID) == 0) {
            if (strcmp(players[i].status, "Active") == 0) {
                cout << "Player already active.\n";
                return;
            }

            strcpy(players[i].status, "Active");
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
        if (strcmp(players[i].playerID, playerID) == 0) {
            strcpy(players[i].status, "Eliminated");
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
