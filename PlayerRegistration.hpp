#ifndef PLAYERREGISTRATION_HPP
#define PLAYERREGISTRATION_HPP

struct Player
{
    char playerID[10];
    char name[50];
    char university[50];
    int ranking;
    char status[15];
    int matchesPlayed;
    int points;
    char grouping[10];
    bool isEarlyBird = false;
    bool isWildcard = false;
};

struct QueueNode
{
    Player *player;
    QueueNode *next;
};

class PlayerRegistration
{
private:
    Player players[100];
    int totalPlayers;

    QueueNode *front;
    QueueNode *rear;

public:
    PlayerRegistration();
    ~PlayerRegistration();

    void loadPlayers(const char *filename);
    void savePlayers(const char *filename);

    void displayAllPlayers();
    void checkInPlayer(const char *playerID);
    void processCheckInQueue();
    void withdrawPlayer(const char *playerID);
    void displayCheckInQueue();
    void displayReadyMatches();
    void setPlayerPriority(const char *playerID, char priorityType);

    bool validatePlayerCount();
};

#endif
