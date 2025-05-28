#include "SpectatorManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <map>

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

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void SpectatorManager::displayStreamerSchedule() {
    std::map<std::string, std::pair<std::string, std::string>> matchTimes;

    // Load match times from matches.csv
    std::ifstream file("data/matches.csv");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string matchID, stage, p1, p2, winner, status, round, time, streamerID, date;

            std::getline(ss, matchID, ',');
            std::getline(ss, stage, ',');
            std::getline(ss, p1, ',');
            std::getline(ss, p2, ',');
            std::getline(ss, winner, ',');
            std::getline(ss, status, ',');
            std::getline(ss, round, ',');
            std::getline(ss, time, ',');
            std::getline(ss, streamerID, ',');
            std::getline(ss, date, ',');

            matchTimes[trim(matchID)] = {trim(time), trim(date)};
        }
        file.close();
    }

    // Sort streamer slots by match ID
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
        const std::string& matchID = trim(streamerMatchIDs[idx]);
        if (streamerSlots[idx] != "EMPTY") {
            auto it = matchTimes.find(matchID);
            if (it != matchTimes.end()) {
                const std::string& time = it->second.first;
                const std::string& date = it->second.second;
                std::cout << "Match " << matchID << " (" << date << " " << time << "): " << streamerSlots[idx] << "\n";
            } else {
                std::cout << "Match " << matchID << " (Unknown time): " << streamerSlots[idx] << "\n";
            }
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
    if (vipFront == -1) {
        std::cout << "[INFO] VIP queue is empty.\n";
        return false;
    }

    std::cout << "Current VIP Queue:\n";
    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP; i = (i + 1) % MAX_VIP)
        std::cout << "- " << vipQueue[i] << "\n";

    std::string name;
    std::cout << "Enter the VIP name to remove: ";
    std::getline(std::cin, name);

    int index = -1;
    for (int i = vipFront; i != -1 && i != (vipRear + 1) % MAX_VIP; i = (i + 1) % MAX_VIP) {
        if (vipQueue[i] == name) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        std::cout << "[ERROR] VIP name not found in queue.\n";
        return false;
    }

    for (int i = index; i != vipRear; i = (i + 1) % MAX_VIP)
        vipQueue[i] = vipQueue[(i + 1) % MAX_VIP];

    vipRear = (vipRear - 1 + MAX_VIP) % MAX_VIP;
    if (vipFront == (vipRear + 1) % MAX_VIP)
        vipFront = vipRear = -1;

    std::cout << "[SUCCESS] VIP \"" << name << "\" removed from the queue.\n";
    return true;
}

bool SpectatorManager::removeSpectatorFromGeneral() {
    if (genFront == -1) {
        std::cout << "[INFO] General queue is empty.\n";
        return false;
    }

    std::cout << "Current General Spectator Queue:\n";
    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL; i = (i + 1) % MAX_GENERAL)
        std::cout << "- " << generalQueue[i] << "\n";

    std::string name;
    std::cout << "Enter the spectator name to remove: ";
    std::getline(std::cin, name);

    int index = -1;
    for (int i = genFront; i != -1 && i != (genRear + 1) % MAX_GENERAL; i = (i + 1) % MAX_GENERAL) {
        if (generalQueue[i] == name) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        std::cout << "[ERROR] Name not found in general queue.\n";
        return false;
    }

    for (int i = index; i != genRear; i = (i + 1) % MAX_GENERAL)
        generalQueue[i] = generalQueue[(i + 1) % MAX_GENERAL];

    genRear = (genRear - 1 + MAX_GENERAL) % MAX_GENERAL;
    if (genFront == (genRear + 1) % MAX_GENERAL)
        genFront = genRear = -1;

    std::cout << "[SUCCESS] General spectator \"" << name << "\" removed from the queue.\n";
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

    int mainChoice = 0;
    do {
        std::cout << "\n===== SPECTATOR MANAGEMENT MAIN MENU =====\n";
        std::cout << "1. VIP Management\n";
        std::cout << "2. General Spectator Management\n";
        std::cout << "3. Streamer Slot Management\n";
        std::cout << "4. Display Information\n";
        std::cout << "5. Exit\n";
        std::cout << "Choose a section: ";

        if (!(std::cin >> mainChoice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[WARNING] Invalid input. Please enter a number.\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "--------------------------------------------\n";

        switch (mainChoice) {
            case 1: { // VIP Management
                int vipChoice;
                std::cout << "\n[VIP MANAGEMENT]\n";
                std::cout << "1. Add VIP\n";
                std::cout << "2. Remove VIP\n";
                std::cout << "Choose an action: ";
                std::cin >> vipChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (vipChoice == 1) {
                    std::string name;
                    std::cout << "Enter VIP name: ";
                    std::getline(std::cin, name);
                    if (addVIP(name))
                        std::cout << "[SUCCESS] VIP \"" << name << "\" added to the queue.\n";
                    else
                        std::cout << "[ERROR] Failed to add VIP \"" << name << "\".\n";

                } else if (vipChoice == 2) {
                    removeSpectatorFromVIP();
                } else {
                    std::cout << "[WARNING] Invalid VIP action.\n";
                }
                break;
            }
            case 2: { // General Spectator Management
                int genChoice;
                std::cout << "\n[GENERAL SPECTATOR MANAGEMENT]\n";
                std::cout << "1. Add General Spectator\n";
                std::cout << "2. Remove General Spectator\n";
                std::cout << "Choose an action: ";
                std::cin >> genChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (genChoice == 1) {
                    std::string name;
                    std::cout << "Enter general spectator name: ";
                    std::getline(std::cin, name);
                    if (addGeneralSpectator(name))
                        std::cout << "[SUCCESS] Spectator \"" << name << "\" added to the queue.\n";
                    else
                        std::cout << "[ERROR] Failed to add spectator \"" << name << "\".\n";

                } else if (genChoice == 2) {
                    removeSpectatorFromGeneral();
                } else {
                    std::cout << "[WARNING] Invalid spectator action.\n";
                }
                break;
            }
            case 3: { // Streamer Slot Management
                int streamChoice;
                std::cout << "\n[STREAMER SLOT MANAGEMENT]\n";
                std::cout << "1. Assign streamer slot\n";
                std::cout << "2. Remove streamer slot\n";
                std::cout << "Choose an action: ";
                std::cin >> streamChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (streamChoice == 1) {
                    std::string name, matchID;
                    int slot;
                    std::cout << "Enter streamer name: ";
                    std::getline(std::cin, name);
                    std::cout << "Enter match ID: ";
                    std::getline(std::cin, matchID);
                    std::cout << "Enter slot number (1 to " << MAX_STREAMERS << "): ";
                    std::cin >> slot;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    assignStreamerSlot(slot - 1, name, matchID);
                } else if (streamChoice == 2) {
                    int slot;
                    displayStreamerSlots();
                    std::cout << "Enter slot number to remove (1 to " << MAX_STREAMERS << "): ";
                    std::cin >> slot;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    if (removeStreamerSlot(slot - 1)) {
                        std::cout << "[SUCCESS] Streamer slot " << slot << " cleared.\n";
                    } else {
                        std::cout << "[ERROR] Invalid slot number or slot already empty.\n";
                    }

                } else {
                    std::cout << "[WARNING] Invalid streamer action.\n";
                }
                break;
            }
            case 4: { // Display Info
                int displayChoice;
                std::cout << "\n[DISPLAY INFORMATION]\n";
                std::cout << "1. View next 3 spectators in each queue\n";
                std::cout << "2. View all queues\n";
                std::cout << "3. View streamer slots\n";
                std::cout << "4. View upcoming matches\n";
                std::cout << "5. View streamer match schedule\n";
                std::cout << "Choose an option: ";
                std::cin >> displayChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                switch (displayChoice) {
                    case 1: displayNextSpectators(); break;
                    case 2: displayQueues(); break;
                    case 3: displayStreamerSlots(); break;
                    case 4: displayUpcomingMatches(); break;
                    case 5: displayStreamerSchedule(); break;
                    default: std::cout << "[WARNING] Invalid display option.\n";
                }
                break;
            }
            case 5:
                std::cout << "Exiting Spectator Manager...\n";
                break;
            default:
                std::cout << "[WARNING] Invalid main menu option.\n";
        }

        std::cout << "--------------------------------------------\n";

    } while (mainChoice != 5);

    saveToFile("data/spectators.csv");
}
