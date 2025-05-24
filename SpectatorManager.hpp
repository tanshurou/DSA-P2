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

    int vipFront, vipRear;
    int genFront, genRear;

public:
    SpectatorManager();

    bool addVIP(const std::string& name);
    bool addGeneralSpectator(const std::string& name);
    bool assignStreamerSlot(int slot, const std::string& name);
    void displayStreamerSlots();
    void displayQueues();

    void run(); // test interface
};

#endif
