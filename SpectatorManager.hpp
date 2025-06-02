#ifndef SPECTATOR_MANAGER_HPP
#define SPECTATOR_MANAGER_HPP

#include <string>
#include <map>
#include <iostream>

class SpectatorManager {
private:
    static const int MAX_VIP = 10;
    static const int MAX_GENERAL = 20;
    static const int MAX_STREAMERS = 5;

    std::string vipQueue[MAX_VIP];
    std::string generalQueue[MAX_GENERAL];
    std::string streamerSlots[MAX_STREAMERS];
    std::string streamerMatchIDs[MAX_STREAMERS];

    int vipFront, vipRear;
    int genFront, genRear;

public:
    SpectatorManager();
    bool addVIP(const std::string& name);
    bool addGeneralSpectator(const std::string& name);
    bool removeSpectatorFromVIP();
    bool removeSpectatorFromGeneral();
    void displayNextSpectators();
    void displayVIPQueue();
    void displayGeneralQueue();
    void displayQueues();
    bool searchSpectator(const std::string& name);
    bool assignStreamerSlot(int slot, const std::string& name, const std::string& matchID, const std::map<std::string, std::pair<std::string, std::string>>& matchTimes);
    bool removeStreamerSlot(int slot);
    void displayStreamerSlots();
    void displayStreamerSchedule();
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
    void runSpectatorManager();
};

#endif
