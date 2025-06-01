#ifndef TOURNAMENT_MANAGER_HPP
#define TOURNAMENT_MANAGER_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <string>

const int MAX_PLAYERS = 32;
const int MAX_MATCHES = 100;
const int GROUP_SIZE = 4;
const int MAX_GROUPS = MAX_PLAYERS / GROUP_SIZE;
#ifndef TOURNAMENT_MANAGER_HPP
#define TOURNAMENT_MANAGER_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <string>

const int MAX_PLAYERS = 32;
const int MAX_MATCHES = 100;
const int GROUP_SIZE = 4;
const int MAX_GROUPS = MAX_PLAYERS / GROUP_SIZE;

enum TournamentStage
{
  QUALIFIERS,
  GROUP_STAGE,
  KNOCKOUT_STAGE,
  TOURNAMENT_OVER
};

struct Participant
{
  char id[10];
  char name[50];
  char university[50];
  int rank;
  char status[10];
  int matchesPlayed;
  int points;
  int group;
};

struct Match
{
  int matchIndex;
  char matchID[10];
  TournamentStage stage;
  int player1Index;
  int player2Index;
  int winnerIndex;
  char status[10];
  int groupID;
  int startTime;
  bool isPlayed;
  float durationInSeconds;
  int date;
  int parentMatchIndex;
};

struct CircularQueue
{
  Match queue[MAX_MATCHES];
  int front, rear, count;

  void init();
  bool isEmpty();
  bool isFull();
  bool enqueue(const Match &m);
  bool dequeue(Match &m);
};

extern Participant players[MAX_PLAYERS];
extern int playerCount;
extern Match matches[MAX_MATCHES];
extern int matchCount;
extern TournamentStage currentStage;
extern Participant advancingPlayers[MAX_PLAYERS];
extern int advancingCount;
extern int groups[MAX_GROUPS][GROUP_SIZE];
extern int groupCounts[MAX_GROUPS];
extern CircularQueue groupMatchQueues[MAX_GROUPS];

bool continueKnockout();
const char *getKnockoutStageName(int numPlayers);
int findPlayerIndexByID(const char *playerID);
void rebuildGroupQueuesFromMatches();
void loadPlayersFromFile(const char *filename);
void savePlayersToFile(const char *filename);
void updatePlayerList();
void saveTournamentStage(const char *filename);
void loadTournamentStage(const char *filename);
void sortPlayersByRank(Participant arr[], int size);
void displayPlayers(Participant *players, int playerCount);
void loadMatchesFromFile(const char *filename);
void saveMatchesToFile(const char *filename);
void shufflePlayers(Participant *players, int playerCount);
void displayMatchBrackets();
void logResult(char matchID[10], int winnerIndex);
void setMatchResult(int matchIndex, int winnerIndex, TournamentStage stage);
bool isLeapYear(int year);
bool isValidDate(int year, int month, int day);
int getDate();
int getTime();
void generateQualifierMatches();
void collectWinners(TournamentStage stage);
void assignPlayersToGroups();
void generateFullKnockoutBracket();
void generateRoundRobinMatches();
bool allMatchesPlayedInStage(TournamentStage stage);
void printMatches(TournamentStage stage);
void generateMatches();
bool getWinnerDone(int matchIndex);
void inputMatchResult();
void updateCurrentStage();
int runTournamentManager();

#endif // TOURNAMENT_MANAGER_HPP

enum TournamentStage
{
  QUALIFIERS,
  GROUP_STAGE,
  KNOCKOUT_STAGE,
  TOURNAMENT_OVER
};

struct Participant
{
  char id[10];
  char name[50];
  char university[50];
  int rank;
  char status[10];
  int matchesPlayed;
  int points;
  int group;
};

struct Match
{
  int matchIndex;
  char matchID[10];
  TournamentStage stage;
  int player1Index;
  int player2Index;
  int winnerIndex;
  char status[10];
  int groupID;
  int startTime;
  bool isPlayed;
  float durationInSeconds;
  int date;
  int parentMatchIndex;
};

struct CircularQueue
{
  Match queue[MAX_MATCHES];
  int front, rear, count;

  void init();
  bool isEmpty();
  bool isFull();
  bool enqueue(const Match &m);
  bool dequeue(Match &m);
};

extern Participant players[MAX_PLAYERS];
extern int playerCount;
extern Match matches[MAX_MATCHES];
extern int matchCount;
extern TournamentStage currentStage;
extern Participant advancingPlayers[MAX_PLAYERS];
extern int advancingCount;
extern int groups[MAX_GROUPS][GROUP_SIZE];
extern int groupCounts[MAX_GROUPS];
extern CircularQueue groupMatchQueues[MAX_GROUPS];

bool continueKnockout();
const char *getKnockoutStageName(int numPlayers);
int findPlayerIndexByID(const char *playerID);
void rebuildGroupQueuesFromMatches();
void loadPlayersFromFile(const char *filename);
void savePlayersToFile(const char *filename);
void updatePlayerList();
void saveTournamentStage(const char *filename);
void loadTournamentStage(const char *filename);
void sortPlayersByRank(Participant arr[], int size);
void displayPlayers(Participant *players, int playerCount);
void loadMatchesFromFile(const char *filename);
void saveMatchesToFile(const char *filename);
void shufflePlayers(Participant *players, int playerCount);
void displayMatchBrackets();
void logResult(char matchID[10], int winnerIndex);
void setMatchResult(int matchIndex, int winnerIndex, TournamentStage stage);
bool isLeapYear(int year);
bool isValidDate(int year, int month, int day);
int getDate();
int getTime();
void generateQualifierMatches();
void collectWinners(TournamentStage stage);
void assignPlayersToGroups();
void generateFullKnockoutBracket();
void generateRoundRobinMatches();
bool allMatchesPlayedInStage(TournamentStage stage);
void printMatches(TournamentStage stage);
void generateMatches();
bool getWinnerDone(int matchIndex);
void inputMatchResult();
void updateCurrentStage();
int runTournamentManager();

#endif // TOURNAMENT_MANAGER_HPP
