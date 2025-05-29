#ifndef SPECTATOR_MANAGER_HPP
#define SPECTATOR_MANAGER_HPP

#include <string>

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

    // Queue operations
    bool addVIP(const std::string& name);
    bool addGeneralSpectator(const std::string& name);
    bool removeSpectatorFromVIP();
    bool removeSpectatorFromGeneral();
    void displayNextSpectators();
    void displayVIPQueue();        
    void displayGeneralQueue();           

    // Streamer operations
    bool assignStreamerSlot(int slot, const std::string& name, const std::string& matchID);
    bool removeStreamerSlot(int slot);
    void displayStreamerSlots();
    void displayStreamerSchedule();

    // Display operations
    void displayQueues();

    // File operations
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

    // Main interface
    void run();
};

#endif
