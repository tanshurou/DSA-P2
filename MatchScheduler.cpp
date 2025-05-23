#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

const int MAX_PLAYERS = 32;
const int MAX_MATCHES = 100;

enum TournamentStage
{
  QUALIFIERS,
  GROUP_STAGE,
  KNOCKOUT_STAGE,
  TOURNAMENT_OVER
};

struct Player
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
  int winnerIndex; // -1 if undecided
  char status[10];
  int groupID;
  int startTime;
  bool isPlayed;
};

Player players[MAX_PLAYERS];
int playerCount = 0;

Match matches[MAX_MATCHES];
int matchCount = 0;

TournamentStage currentStage = QUALIFIERS;

int advancingPlayers[MAX_PLAYERS];
int advancingCount = 0;

const int GROUP_SIZE = 4;
const int MAX_GROUPS = MAX_PLAYERS / GROUP_SIZE;

int groups[MAX_GROUPS][GROUP_SIZE];
int groupCounts[MAX_GROUPS];

struct CircularQueue
{
  Match queue[MAX_MATCHES];
  int front, rear, count;

  void init()
  {
    front = 0;
    rear = -1;
    count = 0;
  }

  bool isEmpty() { return count == 0; }
  bool isFull() { return count == MAX_MATCHES; }

  bool enqueue(const Match &m)
  {
    if (isFull())
      return false;
    rear = (rear + 1) % MAX_MATCHES;
    queue[rear] = m;
    count++;
    return true;
  }

  bool dequeue(Match &m)
  {
    if (isEmpty())
      return false;
    m = queue[front];
    front = (front + 1) % MAX_MATCHES;
    count--;
    return true;
  }
};

CircularQueue groupMatchQueue;

// --- Player & match loading/saving, sorting, and display functions ---

void loadPlayersFromFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    printf("Failed to open file %s\n", filename);
    exit(1);
  }

  char line[256];
  playerCount = 0;
  while (fgets(line, sizeof(line), file))
  {
    line[strcspn(line, "\r\n")] = 0;
    if (playerCount >= MAX_PLAYERS)
    {
      printf("Max player limit reached.\n");
      break;
    }

    char *token = strtok(line, ",");
    if (!token)
      continue;
    strcpy(players[playerCount].id, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    strcpy(players[playerCount].name, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    strcpy(players[playerCount].university, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    players[playerCount].rank = atoi(token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    strcpy(players[playerCount].status, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    players[playerCount].matchesPlayed = atoi(token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    players[playerCount].points = atoi(token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    players[playerCount].group = atoi(token);

    playerCount++;
  }
  fclose(file);
  printf("%d players loaded from %s\n\n", playerCount, filename);
}

void savePlayersToFile(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
  {
    printf("Failed to open %s for writing.\n", filename);
    return;
  }

  for (int i = 0; i < playerCount; i++)
  {
    fprintf(file, "%s,%s,%s,%d,%s,%d,%d,%d\n",
            players[i].id,
            players[i].name,
            players[i].university,
            players[i].rank,
            players[i].status,
            players[i].matchesPlayed,
            players[i].points,
            players[i].group);
  }

  fclose(file);
  printf("Players saved to %s\n", filename);
}

void saveTournamentStage(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
  {
    printf("Failed to save tournament stage.\n");
    return;
  }
  fprintf(file, "%d\n", (int)currentStage);
  fclose(file);
}

void loadTournamentStage(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    printf("No previous stage file found, starting new tournament.\n");
    currentStage = QUALIFIERS; // default start stage
    return;
  }
  int stage;
  if (fscanf(file, "%d", &stage) == 1)
  {
    if (stage >= QUALIFIERS && stage <= TOURNAMENT_OVER)
    {
      currentStage = (TournamentStage)stage;
    }
    else
    {
      currentStage = QUALIFIERS; // fallback
    }
  }
  else
  {
    currentStage = QUALIFIERS; // fallback
  }
  fclose(file);
}

void sortPlayersByRank()
{
  for (int i = 0; i < playerCount - 1; i++)
  {
    for (int j = 0; j < playerCount - i - 1; j++)
    {
      if (players[j].rank > players[j + 1].rank)
      {
        Player temp = players[j];
        players[j] = players[j + 1];
        players[j + 1] = temp;
      }
    }
  }
}

void displayPlayers()
{
  cout << "Players List:\n";
  cout << "ID\tName\tUniversity\tRank\tStatus\tMatchesPlayed\tPoints\tGroup\n";
  for (int i = 0; i < playerCount; i++)
  {
    cout << players[i].id << "\t"
         << players[i].name << "\t"
         << players[i].university << "\t"
         << players[i].rank << "\t"
         << players[i].status << "\t"
         << players[i].matchesPlayed << "\t\t"
         << players[i].points << "\t"
         << players[i].group << "\n";
  }
  cout << "\n";
}

void loadMatchesFromFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    printf("No existing match file found (%s). Starting fresh.\n", filename);
    matchCount = 0;
    return;
  }

  char line[256];
  matchCount = 0;

  while (fgets(line, sizeof(line), file))
  {
    line[strcspn(line, "\r\n")] = 0;

    if (matchCount >= MAX_MATCHES)
    {
      printf("Max matches limit reached.\n");
      break;
    }

    char *token = strtok(line, ",");
    if (!token)
      continue;
    strcpy(matches[matchCount].matchID, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    if (strcmp(token, "Qualifiers") == 0)
      matches[matchCount].stage = QUALIFIERS;
    else if (strcmp(token, "Group") == 0)
      matches[matchCount].stage = GROUP_STAGE;
    else if (strcmp(token, "Knockout") == 0)
      matches[matchCount].stage = KNOCKOUT_STAGE;
    else
      matches[matchCount].stage = TOURNAMENT_OVER;

    token = strtok(NULL, ",");
    if (!token)
      continue;
    int i;
    for (i = 0; i < playerCount; i++)
    {
      if (strcmp(players[i].id, token) == 0)
      {
        matches[matchCount].player1Index = i;
        break;
      }
    }
    if (i == playerCount)
      matches[matchCount].player1Index = -1;

    token = strtok(NULL, ",");
    if (!token)
      continue;
    for (i = 0; i < playerCount; i++)
    {
      if (strcmp(players[i].id, token) == 0)
      {
        matches[matchCount].player2Index = i;
        break;
      }
    }
    if (i == playerCount)
      matches[matchCount].player2Index = -1;

    token = strtok(NULL, ",");
    if (!token)
      continue;
    if (strcmp(token, "-1") == 0)
      matches[matchCount].winnerIndex = -1;
    else
    {
      for (i = 0; i < playerCount; i++)
      {
        if (strcmp(players[i].id, token) == 0)
        {
          matches[matchCount].winnerIndex = i;
          break;
        }
      }
      if (i == playerCount)
        matches[matchCount].winnerIndex = -1;
    }

    token = strtok(NULL, ",");
    if (!token)
      continue;
    strcpy(matches[matchCount].status, token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    matches[matchCount].groupID = atoi(token);

    token = strtok(NULL, ",");
    if (!token)
      continue;
    matches[matchCount].startTime = atoi(token);

    matches[matchCount].isPlayed = (strcmp(matches[matchCount].status, "Completed") == 0);

    matches[matchCount].matchIndex = matchCount;

    matchCount++;
  }

  fclose(file);
  printf("%d matches loaded from %s\n\n", matchCount, filename);
}

void saveMatchesToFile(const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
  {
    printf("Failed to open %s for writing.\n", filename);
    return;
  }

  for (int i = 0; i < matchCount; i++)
  {
    fprintf(file, "%s,", matches[i].matchID);

    const char *stageStr = "";
    switch (matches[i].stage)
    {
    case QUALIFIERS:
      stageStr = "Qualifiers";
      break;
    case GROUP_STAGE:
      stageStr = "Group";
      break;
    case KNOCKOUT_STAGE:
      stageStr = "Knockout";
      break;
    default:
      stageStr = "Unknown";
      break;
    }
    fprintf(file, "%s,", stageStr);

    fprintf(file, "%s,", players[matches[i].player1Index].id);
    fprintf(file, "%s,", players[matches[i].player2Index].id);

    if (matches[i].winnerIndex == -1)
      fprintf(file, "-1,");
    else
      fprintf(file, "%s,", players[matches[i].winnerIndex].id);

    fprintf(file, "%s,", matches[i].status);
    fprintf(file, "%d,", matches[i].groupID);
    fprintf(file, "%d\n", matches[i].startTime);
  }

  fclose(file);
  printf("Matches saved to %s\n", filename);
}

void assignPlayersToGroups()
{
  for (int i = 0; i < MAX_GROUPS; i++)
    groupCounts[i] = 0;
  int groupIndex = 0;
  for (int i = 0; i < advancingCount; i++)
  {
    int g = groupIndex % MAX_GROUPS;
    groups[g][groupCounts[g]] = advancingPlayers[i];
    players[advancingPlayers[i]].group = g + 1; // update player's group
    groupCounts[g]++;
    groupIndex++;
  }

  savePlayersToFile("data/players.csv");
}

void logResult(int matchID, int winnerIndex)
{
  cout << "[LOG] Match " << matchID << " result: Winner is " << players[winnerIndex].name << "\n";
}

void setMatchResult(int matchID, int winnerIndex)
{
  matches[matchID].winnerIndex = winnerIndex;
  matches[matchID].isPlayed = true;
  strcpy(matches[matchID].status, "Completed");
  logResult(matchID, winnerIndex);

  int loserIndex = (matches[matchID].player1Index == winnerIndex) ? matches[matchID].player2Index : matches[matchID].player1Index;
  if (strcmp(players[loserIndex].status, "eliminated") != 0)
  {
    strcpy(players[loserIndex].status, "eliminated");
  }

  saveMatchesToFile("data/matches.csv");
  savePlayersToFile("data/players.csv");
}

// --- The new simple qualifier matches generator ---
void generateQualifierMatches()
{
  matchCount = 0;

  int left = 0;
  int right = playerCount - 1;

  while (left < right)
  {
    matches[matchCount].matchIndex = matchCount;
    sprintf(matches[matchCount].matchID, "M%03d", matchCount + 1);
    matches[matchCount].stage = QUALIFIERS;
    matches[matchCount].player1Index = left;
    matches[matchCount].player2Index = right;
    matches[matchCount].winnerIndex = -1;
    strcpy(matches[matchCount].status, "Scheduled");
    matches[matchCount].groupID = 0;

    int startTime = 1400;

    if (matchCount * 30 == 60)
    {
      startTime += 100;
    }
    else if (matchCount * 30 % 60 == 0)
    {
      startTime += matchCount * 30;
    }
    else
    {
      startTime += ((matchCount - 1) / 2 * 100) + 30;
    }
    matches[matchCount].startTime = startTime;
    matches[matchCount].isPlayed = false;

    matchCount++;
    left++;
    right--;
  }

  saveMatchesToFile("data/matches.csv");
}

void generateRoundRobinMatches()
{
  groupMatchQueue.init();

  // Reset matchCount to qualifiers count, or zero if starting fresh
  // Here assume we start fresh group stage, reset to 0:
  matchCount = 0;

  for (int g = 0; g < MAX_GROUPS; g++)
  {
    int size = groupCounts[g];
    int matchIdx = 0; // reset per group
    for (int i = 0; i < size - 1; i++)
    {
      for (int j = i + 1; j < size; j++)
      {
        Match m;
        m.matchIndex = matchCount;
        sprintf(m.matchID, "G%dM%02d", g + 1, matchIdx + 1);
        m.stage = GROUP_STAGE;
        m.player1Index = groups[g][i];
        m.player2Index = groups[g][j];
        m.winnerIndex = -1;
        strcpy(m.status, "Scheduled");
        m.groupID = g;
        m.startTime = 0;
        m.isPlayed = false;

        matches[matchCount] = m;
        groupMatchQueue.enqueue(m);
        matchCount++;
        matchIdx++;
      }
    }
  }
  saveMatchesToFile("data/matches.csv");
}

bool allMatchesPlayedInStage(TournamentStage stage)
{
  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == stage && !matches[i].isPlayed)
    {
      return false;
    }
  }
  return true;
}

void printMatches(TournamentStage stage)
{
  cout << "Matches in stage: ";
  switch (stage)
  {
  case QUALIFIERS:
    cout << "Qualifiers\n";
    break;
  case GROUP_STAGE:
    cout << "Group Stage\n";
    break;
  case KNOCKOUT_STAGE:
    cout << "Knockout Stage\n";
    break;
  default:
    cout << "Unknown\n";
    break;
  }
  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == stage)
    {
      cout << i << ": " << players[matches[i].player1Index].name
           << " vs " << players[matches[i].player2Index].name
           << " | Status: " << matches[i].status
           << "| Start TIme: " << matches[i].startTime;
      if (matches[i].isPlayed)
      {
        cout << " | Winner: " << players[matches[i].winnerIndex].name;
      }
      cout << "\n";
    }
  }
  cout << "\n";
}

void collectWinners(TournamentStage stage)
{
  advancingCount = 0;
  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == stage && matches[i].isPlayed)
    {
      advancingPlayers[advancingCount++] = matches[i].winnerIndex;
      int idx = matches[i].winnerIndex;
      advancingPlayers[advancingCount++] = idx;
      cout << "Advancing player: " << players[idx].name << "\n";
    }
  }
}

void generateKnockoutMatches()
{
  matchCount = 0;
  int n = advancingCount;
  for (int i = 0; i < n / 2; i++)
  {
    matches[matchCount].matchIndex = matchCount;
    sprintf(matches[matchCount].matchID, "M%03d", matchCount + 1);
    matches[matchCount].stage = KNOCKOUT_STAGE;
    matches[matchCount].player1Index = advancingPlayers[i];
    matches[matchCount].player2Index = advancingPlayers[n - 1 - i];
    matches[matchCount].winnerIndex = -1;
    strcpy(matches[matchCount].status, "Scheduled");
    matches[matchCount].groupID = 0;
    matches[matchCount].startTime = 1800 + i * 30;
    matches[matchCount].isPlayed = false;
    matchCount++;
  }
  saveMatchesToFile("data/matches.csv");
}

void generateMatches()
{
  if (currentStage == QUALIFIERS)
  {
    if (matchCount == 0)
    {
      generateQualifierMatches();
      printMatches(QUALIFIERS);
    }
    else if (allMatchesPlayedInStage(QUALIFIERS))
    {
      collectWinners(QUALIFIERS);
      currentStage = GROUP_STAGE;
      assignPlayersToGroups();
      generateRoundRobinMatches();
      cout << "Group Stage matches generated.\n";
    }
    else
    {
      cout << "Please finish all qualifier matches first.\n";
    }
  }
  else if (currentStage == GROUP_STAGE)
  {
    if (groupMatchQueue.isEmpty())
    {
      cout << "All group matches played.\n";
      // Here you should calculate group rankings and advance players
      // For simplicity, directly advance all for knockout
      collectWinners(GROUP_STAGE); // You’ll need to implement collecting winners based on group points
      currentStage = KNOCKOUT_STAGE;
      generateKnockoutMatches();
      printMatches(KNOCKOUT_STAGE);
    }
    else
    {
      cout << "There are still group matches remaining to be played.\n";
    }
  }
  else if (currentStage == KNOCKOUT_STAGE)
  {
    if (allMatchesPlayedInStage(KNOCKOUT_STAGE))
    {
      currentStage = TOURNAMENT_OVER;
      cout << "Tournament finished! Congratulations to the winner!\n";
    }
    else
    {
      cout << "Please finish all knockout matches first.\n";
    }
  }
}

void inputMatchResult()
{
  if (currentStage == GROUP_STAGE)
  {
    cout << "Input result for Group stage match or Solo stage match?\n";
    cout << "1. Group Stage Match\n2. Solo Stage Match (Qualifier/Knockout)\nChoice: ";
    int stageChoice;
    cin >> stageChoice;

    if (stageChoice == 1)
    {
      if (groupMatchQueue.isEmpty())
      {
        cout << "No group matches left to input result for.\n";
        return;
      }
      Match m;
      groupMatchQueue.dequeue(m);
      cout << "Group " << (m.groupID + 1) << " Match: "
           << players[m.player1Index].name << " vs " << players[m.player2Index].name << "\n";
      cout << "Who won? Enter 1 for " << players[m.player1Index].name
           << ", 2 for " << players[m.player2Index].name << ": ";
      int winnerChoice;
      cin >> winnerChoice;
      if (winnerChoice == 1)
        setMatchResult(m.matchIndex, m.player1Index);
      else if (winnerChoice == 2)
        setMatchResult(m.matchIndex, m.player2Index);
      else
        cout << "Invalid choice, match skipped.\n";
    }
    else if (stageChoice == 2)
    {
      // Process solo matches input
      int matchID, winnerChoice;
      cout << "Enter match ID to input result: ";
      cin >> matchID;
      if (matchID < 0 || matchID >= matchCount || matches[matchID].isPlayed)
      {
        cout << "Invalid match ID or already played.\n";
        return;
      }
      cout << "Who won? Enter 1 for " << players[matches[matchID].player1Index].name
           << ", 2 for " << players[matches[matchID].player2Index].name << ": ";
      cin >> winnerChoice;
      if (winnerChoice == 1)
        setMatchResult(matchID, matches[matchID].player1Index);
      else if (winnerChoice == 2)
        setMatchResult(matchID, matches[matchID].player2Index);
      else
        cout << "Invalid choice.\n";
    }
    else
    {
      cout << "Invalid stage choice.\n";
    }
  }
  else
  {
    // Not group stage, process solo matches only
    int matchID, winnerChoice;
    cout << "Enter match ID to input result: ";
    cin >> matchID;
    if (matchID < 0 || matchID >= matchCount || matches[matchID].isPlayed)
    {
      cout << "Invalid match ID or already played.\n";
      return;
    }
    cout << "Who won? Enter 1 for " << players[matches[matchID].player1Index].name
         << ", 2 for " << players[matches[matchID].player2Index].name << ": ";
    cin >> winnerChoice;
    if (winnerChoice == 1)
      setMatchResult(matchID, matches[matchID].player1Index);
    else if (winnerChoice == 2)
      setMatchResult(matchID, matches[matchID].player2Index);
    else
      cout << "Invalid choice.\n";
  }
}

void updateCurrentStage()
{
  if (matchCount == 0)
  {
    currentStage = QUALIFIERS;
    return;
  }

  bool qualifiersIncomplete = false;
  bool knockoutIncomplete = false;
  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == QUALIFIERS && !matches[i].isPlayed)
    {
      qualifiersIncomplete = true;
      break;
    }
  }
  if (qualifiersIncomplete)
  {
    currentStage = QUALIFIERS;
    return;
  }
  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == KNOCKOUT_STAGE && !matches[i].isPlayed)
    {
      knockoutIncomplete = true;
      break;
    }
  }
  if (knockoutIncomplete)
  {
    currentStage = KNOCKOUT_STAGE;
  }
  else
  {
    currentStage = TOURNAMENT_OVER;
  }
}

int main()
{
  loadPlayersFromFile("data/players.csv");
  sortPlayersByRank();
  displayPlayers();

  loadMatchesFromFile("data/matches.csv");
  loadTournamentStage("data/stage.txt"); // Load saved stage

  while (true)
  {
    cout << "\nCurrent Stage: ";
    switch (currentStage)
    {
    case QUALIFIERS:
      cout << "Qualifiers\n";
      break;
    case GROUP_STAGE:
      cout << "Group Stage\n";
      break;
    case KNOCKOUT_STAGE:
      cout << "Knockout Stage\n";
      break;
    case TOURNAMENT_OVER:
      cout << "Tournament Over\n";
      break;
    default:
      cout << "Unknown\n";
      break;
    }
    cout << "1. Generate Matches\n2. Input Match Result\n3. Show Matches\n4. Exit\nChoice: ";
    int choice;
    cin >> choice;

    if (choice == 1)
    {
      generateMatches();
    }
    else if (choice == 2)
    {
      inputMatchResult();
    }
    else if (choice == 3)
    {
      printMatches(currentStage);
    }
    else if (choice == 4)
    {
      saveTournamentStage("data/stage.txt"); // Save before exit
      cout << "Exiting...\n";
      break;
    }
    else
    {
      cout << "Invalid choice.\n";
    }
  }
  return 0;
}
