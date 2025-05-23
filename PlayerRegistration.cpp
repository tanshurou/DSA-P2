#include <iostream>
#include <fstream>
#include <cstring>
#include "PlayerRegistration.hpp"

using namespace std;

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

    while (file.peek() != EOF && totalPlayers < 100) {
        Player& p = players[totalPlayers];
        file >> p.playerID >> p.name >> p.university >> p.ranking
             >> p.status >> p.matchesPlayed >> p.points >> p.grouping;
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
            if (!rear) {
                front = rear = newNode;
            } else {
                rear->next = newNode;
                rear = newNode;
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
