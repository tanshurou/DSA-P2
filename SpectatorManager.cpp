#include "SpectatorManager.hpp"
#include <iostream>

SpectatorManager::SpectatorManager() {
    vipFront = vipRear = -1;
    genFront = genRear = -1;
    for (int i = 0; i < MAX_STREAMERS; i++) {
        streamerSlots[i] = "EMPTY";
    }
}

// VIP queue logic
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

// General queue logic
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

// Streamer slot logic
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
    std::cout << "Streamer Slots:\n";
    for (int i = 0; i < MAX_STREAMERS; i++) {
        std::cout << "Slot " << i << ": " << streamerSlots[i] << "\n";
    }
}

void SpectatorManager::displayQueues() {
    std::cout << "VIP Queue:\n";
    for (int i = vipFront; i != vipRear + 1; i = (i + 1) % MAX_VIP)
        std::cout << vipQueue[i] << "\n";

    std::cout << "General Queue:\n";
    for (int i = genFront; i != genRear + 1; i = (i + 1) % MAX_GENERAL)
        std::cout << generalQueue[i] << "\n";
}

void SpectatorManager::run() {
    // You can test your code here for live demo
    addVIP("VIP_Alice");
    addGeneralSpectator("Bob");
    assignStreamerSlot(1, "Streamer_X");
    displayQueues();
    displayStreamerSlots();
}
