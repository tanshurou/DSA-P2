#include "SpectatorManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>

SpectatorManager::SpectatorManager() {
    vipFront = vipRear = -1;
    genFront = genRear = -1;
    for (int i = 0; i < MAX_STREAMERS; i++) {
        streamerSlots[i] = "EMPTY";
        streamerMatchIDs[i] = "";
    }
}

bool SpectatorManager::addVIP(const std::string& name) {
    if (name.empty()) {
        std::cout << "[ERROR] Name cannot be empty.\n";
        return false;
    }
    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP; i = (i + 1) % MAX_VIP) {
        if (vipQueue[i] == name) {
            std::cout << "[WARNING] Duplicate name in VIP queue.\n";
            return false;
        }
    }
    if ((vipRear + 1) % MAX_VIP == vipFront) {
        std::cout << "VIP Queue Full\n";
        return false;
    }
    if (vipFront == -1) vipFront = 0;
    vipRear = (vipRear + 1) % MAX_VIP;
    vipQueue[vipRear] = name;
    return true;
}

bool SpectatorManager::addGeneralSpectator(const std::string& name) {
    if (name.empty()) {
        std::cout << "[ERROR] Name cannot be empty.\n";
        return false;
    }
    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL; i = (i + 1) % MAX_GENERAL) {
        if (generalQueue[i] == name) {
            std::cout << "[WARNING] Duplicate name in General queue.\n";
            return false;
        }
    }
    if ((genRear + 1) % MAX_GENERAL == genFront) {
        std::cout << "General Queue Full\n";
        return false;
    }
    if (genFront == -1) genFront = 0;
    genRear = (genRear + 1) % MAX_GENERAL;
    generalQueue[genRear] = name;
    return true;
}

bool SpectatorManager::assignStreamerSlot(int slot, const std::string& name, const std::string& matchID) {
    if (slot < 0 || slot >= MAX_STREAMERS) {
        std::cout << "Invalid slot\n";
        return false;
    }
    if (streamerSlots[slot] != "EMPTY") {
        std::cout << "Slot already taken\n";
        return false;
    }
    streamerSlots[slot] = name;
    streamerMatchIDs[slot] = matchID;
    return true;
}

bool SpectatorManager::removeStreamerSlot(int slot) {
    if (slot < 0 || slot >= MAX_STREAMERS) return false;
    streamerSlots[slot] = "EMPTY";
    streamerMatchIDs[slot] = "";
    return true;
}

void SpectatorManager::displayStreamerSlots() {
    std::cout << "[STREAMER SLOTS]\n";
    for (int i = 0; i < MAX_STREAMERS; i++) {
        std::cout << "Slot " << (i + 1) << ": " << streamerSlots[i];
        if (!streamerMatchIDs[i].empty())
            std::cout << " (Match " << streamerMatchIDs[i] << ")";
        std::cout << "\n";
    }
}

void SpectatorManager::displayStreamerSchedule() {
    int index[MAX_STREAMERS];
    for (int i = 0; i < MAX_STREAMERS; ++i) index[i] = i;

    for (int i = 0; i < MAX_STREAMERS - 1; ++i) {
        for (int j = 0; j < MAX_STREAMERS - i - 1; ++j) {
            if (streamerMatchIDs[index[j]] > streamerMatchIDs[index[j + 1]]) {
                std::swap(index[j], index[j + 1]);
            }
        }
    }

    std::cout << "[STREAMER SCHEDULE BY MATCH ORDER]\n";
    for (int i = 0; i < MAX_STREAMERS; ++i) {
        int idx = index[i];
        if (streamerSlots[idx] != "EMPTY") {
            std::cout << "Match " << streamerMatchIDs[idx] << ": " << streamerSlots[idx] << "\n";
        }
    }
}

void SpectatorManager::displayQueues() {
    std::cout << "VIP Queue:\n";
    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP; i = (i + 1) % MAX_VIP)
        std::cout << vipQueue[i] << "\n";

    std::cout << "General Queue:\n";
    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL; i = (i + 1) % MAX_GENERAL)
        std::cout << generalQueue[i] << "\n";
}

void SpectatorManager::displayNextSpectators() {
    std::cout << "\n[Next in VIP Queue]:\n";
    int count = 0;
    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP && count < 3; i = (i + 1) % MAX_VIP, count++) {
        std::cout << "- " << vipQueue[i] << "\n";
    }

    std::cout << "\n[Next in General Queue]:\n";
    count = 0;
    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL && count < 3; i = (i + 1) % MAX_GENERAL, count++) {
        std::cout << "- " << generalQueue[i] << "\n";
    }
}

bool SpectatorManager::removeSpectatorFromVIP() {
    if (vipFront == -1) return false;
    if (vipFront == vipRear) {
        vipFront = vipRear = -1;
    } else {
        vipFront = (vipFront + 1) % MAX_VIP;
    }
    return true;
}

bool SpectatorManager::removeSpectatorFromGeneral() {
    if (genFront == -1) return false;
    if (genFront == genRear) {
        genFront = genRear = -1;
    } else {
        genFront = (genFront + 1) % MAX_GENERAL;
    }
    return true;
}

void SpectatorManager::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "[ERROR] Could not open file to save: " << filename << "\n";
        return;
    }

    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP; i = (i + 1) % MAX_VIP)
        file << "VIP," << vipQueue[i] << "\n";

    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL; i = (i + 1) % MAX_GENERAL)
        file << "GENERAL," << generalQueue[i] << "\n";

    for (int i = 0; i < MAX_STREAMERS; i++)
        file << "STREAMER_SLOT_" << (i + 1) << "," << streamerSlots[i] << "," << streamerMatchIDs[i] << "\n";

    file.close();
    std::cout << "[INFO] Spectator data saved to '" << filename << "'\n";
}

void SpectatorManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[WARNING] No spectator file found. Starting fresh.\n";
        return;
    }

    vipFront = vipRear = -1;
    genFront = genRear = -1;
    for (int i = 0; i < MAX_STREAMERS; i++) {
        streamerSlots[i] = "EMPTY";
        streamerMatchIDs[i] = "";
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type, name, matchID;
        std::getline(ss, type, ',');
        std::getline(ss, name, ',');
        std::getline(ss, matchID);

        if (type == "VIP") addVIP(name);
        else if (type == "GENERAL") addGeneralSpectator(name);
        else if (type.rfind("STREAMER_SLOT_", 0) == 0) {
            int slot = std::stoi(type.substr(14)) - 1;
            if (slot >= 0 && slot < MAX_STREAMERS) {
                streamerSlots[slot] = name;
                streamerMatchIDs[slot] = matchID;
            }
        }
    }

    file.close();
    std::cout << "[INFO] Spectator data loaded from '" << filename << "'\n";
}

void SpectatorManager::displayUpcomingMatches() {
    std::ifstream file("data/matches.csv");
    if (!file.is_open()) {
        std::cout << "[WARNING] Unable to load matches.csv.\n";
        return;
    }
    std::string line;
    std::cout << "[UPCOMING MATCHES]\n";
    while (std::getline(file, line)) {
        std::cout << line << "\n";
    }
    file.close();
}

void SpectatorManager::run() {
    loadFromFile("data/spectators.csv");

    int choice = 0;
    do {
        std::cout << "\n===== SPECTATOR MANAGEMENT MENU =====\n";
        std::cout << "1. Add VIP to queue\n";
        std::cout << "2. Add general spectator to queue\n";
        std::cout << "3. Assign streamer slot\n";
        std::cout << "4. Remove VIP\n";
        std::cout << "5. Remove general spectator\n";
        std::cout << "6. Remove streamer slot\n";
        std::cout << "7. View next 3 spectators\n";
        std::cout << "8. View current queues\n";
        std::cout << "9. View streamer slots\n";
        std::cout << "10. View upcoming matches\n";
        std::cout << "11. View streamer match schedule\n";
        std::cout << "12. Exit\n";
        std::cout << "Choose an option: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[WARNING] Invalid input. Please enter a number.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\n--------------------------------------------\n";

        switch (choice) {
        case 1: {
            std::string name;
            std::cout << "Enter VIP name: ";
            std::getline(std::cin, name);
            addVIP(name);
            break;
        }
        case 2: {
            std::string name;
            std::cout << "Enter general spectator name: ";
            std::getline(std::cin, name);
            addGeneralSpectator(name);
            break;
        }
        case 3: {
            std::string name, matchID;
            int slot;
            std::cout << "Enter streamer name: ";
            std::getline(std::cin, name);
            std::cout << "Enter match ID: ";
            std::getline(std::cin, matchID);
            std::cout << "Enter slot number (1 to " << MAX_STREAMERS << "): ";
            std::cin >> slot;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            slot -= 1;
            assignStreamerSlot(slot, name, matchID);
            break;
        }
        case 4:
            removeSpectatorFromVIP();
            break;
        case 5:
            removeSpectatorFromGeneral();
            break;
        case 6: {
            int slot;
            std::cout << "Enter slot number to remove (1 to " << MAX_STREAMERS << "): ";
            std::cin >> slot;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            removeStreamerSlot(slot - 1);
            break;
        }
        case 7:
            displayNextSpectators();
            break;
        case 8:
            displayQueues();
            break;
        case 9:
            displayStreamerSlots();
            break;
        case 10:
            displayUpcomingMatches();
            break;
        case 11:
            displayStreamerSchedule();
            break;
        case 12:
            std::cout << "Exiting Spectator Manager...\n";
            break;
        default:
            std::cout << "[WARNING] Invalid choice. Please try again.\n";
        }

        std::cout << "--------------------------------------------\n";

    } while (choice != 12);

    saveToFile("data/spectators.csv");
}
