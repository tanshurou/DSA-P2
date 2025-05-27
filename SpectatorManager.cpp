#include "SpectatorManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

SpectatorManager::SpectatorManager() {
    vipFront = vipRear = -1;
    genFront = genRear = -1;
    for (int i = 0; i < MAX_STREAMERS; i++) {
        streamerSlots[i] = "EMPTY";
    }
}

bool SpectatorManager::addVIP(const std::string& name) {
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
    if ((genRear + 1) % MAX_GENERAL == genFront) {
        std::cout << "General Queue Full\n";
        return false;
    }
    if (genFront == -1) genFront = 0;
    genRear = (genRear + 1) % MAX_GENERAL;
    generalQueue[genRear] = name;
    return true;
}

bool SpectatorManager::assignStreamerSlot(int slot, const std::string& name) {
    if (slot < 0 || slot >= MAX_STREAMERS) {
        std::cout << "Invalid slot\n";
        return false;
    }
    if (streamerSlots[slot] != "EMPTY") {
        std::cout << "Slot already taken\n";
        return false;
    }
    streamerSlots[slot] = name;
    return true;
}

void SpectatorManager::displayStreamerSlots() {
    std::cout << "[STREAMER SLOTS]\n";
    for (int i = 0; i < MAX_STREAMERS; i++) {
        std::cout << "Slot " << (i + 1) << ": " << streamerSlots[i] << "\n";
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
        file << "STREAMER_SLOT_" << (i + 1) << "," << streamerSlots[i] << "\n";
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
    for (int i = 0; i < MAX_STREAMERS; i++) streamerSlots[i] = "EMPTY";

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type, name;
        std::getline(ss, type, ',');
        std::getline(ss, name);

        if (type == "VIP") addVIP(name);
        else if (type == "GENERAL") addGeneralSpectator(name);
        else if (type.rfind("STREAMER_SLOT_", 0) == 0) {
            int slot = std::stoi(type.substr(14)) - 1;
            if (slot >= 0 && slot < MAX_STREAMERS) streamerSlots[slot] = name;
        }
    }

    file.close();
    std::cout << "[INFO] Spectator data loaded from '" << filename << "'\n";
}

void SpectatorManager::run() {
    loadFromFile("data/spectators.csv");

    int choice = 0;
    do {
        std::cout << "\n===== SPECTATOR MANAGEMENT MENU =====\n";
        std::cout << "1. Add VIP to queue\n";
        std::cout << "2. Add general spectator to queue\n";
        std::cout << "3. Assign streamer slot\n";
        std::cout << "4. View current queues\n";
        std::cout << "5. View streamer slots\n";
        std::cout << "6. Exit\n";
        std::cout << "Choose an option: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "\n--------------------------------------------\n";

        switch (choice) {
        case 1: {
            std::string name;
            std::cout << "Enter VIP name: ";
            std::getline(std::cin, name);
            if (addVIP(name))
                std::cout << "[SUCCESS] VIP \"" << name << "\" added to the queue.\n";
            else
                std::cout << "[ERROR] VIP queue is full. \"" << name << "\" was not added.\n";
            break;
        }
        case 2: {
            std::string name;
            std::cout << "Enter general spectator name: ";
            std::getline(std::cin, name);
            if (addGeneralSpectator(name))
                std::cout << "[SUCCESS] Spectator \"" << name << "\" added to the queue.\n";
            else
                std::cout << "[ERROR] General queue is full. \"" << name << "\" was not added.\n";
            break;
        }
        case 3: {
            std::string name;
            int slot;
            std::cout << "Enter streamer name: ";
            std::getline(std::cin, name);
            std::cout << "Enter slot number (1 to " << MAX_STREAMERS << "): ";
            std::cin >> slot;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            slot -= 1;
            if (assignStreamerSlot(slot, name))
                std::cout << "[SUCCESS] Streamer \"" << name << "\" assigned to slot " << (slot + 1) << ".\n";
            else
                std::cout << "[ERROR] Invalid or occupied slot. \"" << name << "\" was not assigned.\n";
            break;
        }
        case 4:
            std::cout << "[CURRENT QUEUES]\n";
            displayQueues();
            break;
        case 5:
            displayStreamerSlots();
            break;
        case 6:
            std::cout << "Exiting Spectator Manager...\n";
            break;
        default:
            std::cout << "[WARNING] Invalid choice. Please try again.\n";
        }

        std::cout << "--------------------------------------------\n";

    } while (choice != 6);

    saveToFile("data/spectators.csv");
}
