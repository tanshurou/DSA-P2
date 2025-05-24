#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdlib> // For rand()
#include <ctime>   // For time()
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <string>

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
  float durationInSeconds;
  int date;
};

Player players[MAX_PLAYERS];
int playerCount = 0;

Match matches[MAX_MATCHES];
int matchCount = 0;

TournamentStage currentStage = QUALIFIERS;

Player advancingPlayers[MAX_PLAYERS];
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

// Utilities function
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

void updatePlayerList()
{
  for (int i = 0; i < playerCount; i++)
  {
    for (int j = 0; j < playerCount; j++)
    {
      if (strcmp(players[i].id, advancingPlayers[j].id) == 0)
      {
        players[i].group = advancingPlayers[j].group;
        break; // Exit the loop once the player is found
      }
    }
  }
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

void displayPlayers(Player *players, int playerCount)
{
  for (int i = 0; i < playerCount; i++)
  {
    cout << i + 1 << ".\t"
         << players[i].id << "\t"
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

    // Read the new fields
    token = strtok(NULL, ",");
    if (!token)
      continue;
    matches[matchCount].durationInSeconds = atoi(token); // Store duration in seconds

    token = strtok(NULL, ",");
    if (!token)
      continue;
    matches[matchCount].date = atoi(token); // Store date as an integer in YYYYMMDD format

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

    // Write the match stage as string
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

    // Write player 1 and player 2 IDs
    fprintf(file, "%s,", players[matches[i].player1Index].id);
    fprintf(file, "%s,", players[matches[i].player2Index].id);

    // Write winner (if any) or -1 if undecided
    if (matches[i].winnerIndex == -1)
      fprintf(file, "-1,");
    else
      fprintf(file, "%s,", players[matches[i].winnerIndex].id);

    // Write the match status
    fprintf(file, "%s,", matches[i].status);

    // Write group ID
    fprintf(file, "%d,", matches[i].groupID);

    // Write start time
    fprintf(file, "%d,", matches[i].startTime);

    // Write duration in seconds
    fprintf(file, "%d,", static_cast<int>(matches[i].durationInSeconds));

    // Write match date in YYYYMMDD format
    fprintf(file, "%d\n", matches[i].date);
  }

  fclose(file);
  printf("Matches saved to %s\n", filename);
}

void shufflePlayers(Player *players, int playerCount)
{
  srand(time(0)); // Use current time as the seed for randomness

  for (int i = advancingCount - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);
    swap(players[i], players[j]);
  }
}

void displayMatchBrackets()
{
  cout << "============================================" << endl;
  cout << "Assigning Players to Group" << endl;
  cout << "============================================" << endl;

  for (int group = 0; group < advancingCount / 4; group++)
  {
    cout << "Group " << group + 1 << endl;
    int index = 1;

    for (int i = 0; i < advancingCount; i++)
    {
      if (advancingPlayers[i].group == group + 1)
      {
        cout << index++ << ".\t"
             << advancingPlayers[i].id << "\t"
             << advancingPlayers[i].name << "\t"
             << advancingPlayers[i].university << "\t"
             << advancingPlayers[i].rank << "\t"
             << advancingPlayers[i].status << "\t"
             << advancingPlayers[i].matchesPlayed << "\t\t"
             << advancingPlayers[i].points << "\t"
             << advancingPlayers[i].group << "\n";
      }
    }
    cout << endl;
  }
}

void logResult(char matchID[10], int winnerIndex)
{
  cout << "[LOG] Match " << matchID << " result: Winner is " << players[winnerIndex].name << "\n";
}

void setMatchResult(int matchIndex, int winnerIndex)
{
  matches[matchIndex].winnerIndex = winnerIndex;
  matches[matchIndex].isPlayed = true;
  strcpy(matches[matchIndex].status, "Completed");
  logResult(matches[matchIndex].matchID, winnerIndex);

  int loserIndex = (matches[matchIndex].player1Index == winnerIndex) ? matches[matchIndex].player2Index : matches[matchIndex].player1Index;
  if (strcmp(players[loserIndex].status, "eliminated") != 0)
  {
    strcpy(players[loserIndex].status, "eliminated");
  }
}

bool isLeapYear(int year)
{
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
    return true;
  return false;
}

bool isValidDate(int year, int month, int day)
{
  // Check if the month is between 1 and 12
  if (month < 1 || month > 12)
    return false;

  // Days in each month
  int daysInMonth[] = {31, (isLeapYear(year) ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Check if the day is valid for the given month
  if (day < 1 || day > daysInMonth[month - 1])
    return false;

  return true;
}

int getDate()
{
  bool invalidInput = true;
  int date;

  while (invalidInput)
  {
    cout << "Enter date (YYYYMMDD format): " << flush;
    cin >> date;

    if (cin.fail())
    {
      cin.clear();
      cin.ignore();
      cout << "Invalid input. Please enter a valid date.\n";
      continue;
    }

    // Extract year, month, and day from the input
    int year = date / 10000;
    int month = (date / 100) % 100;
    int day = date % 100;

    // Check if the year, month, and day are valid
    if (year >= 1000 && isValidDate(year, month, day))
    {
      invalidInput = false;
    }
    else
    {
      cout << "Invalid date. Please ensure the format is YYYYMMDD and the date is valid.\n";
    }
  }

  return date; // return the valid date
}

// --- The new simple qualifier matches generator ---
void generateQualifierMatches()
{
  matchCount = 0;

  int left = 0;
  int right = playerCount - 1;
  int date = getDate();

  while (left < right)
  {
    matches[matchCount].date = date;
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
    matches[matchCount].durationInSeconds = 0; // Initialize duration

    matchCount++;
    left++;
    right--;
  }

  saveMatchesToFile("data/matches.csv");
}

void collectWinners(TournamentStage stage)
{
  advancingCount = 0;

  // keep players with active status in a list
  for (int i = 0; i < playerCount; i++)
  {
    if (string(players[i].status) == "active")
    {
      advancingPlayers[advancingCount] = players[i];
      advancingCount += 1;
    }
    else
    {
      cout << players[i].name << " is eliminated "
           << players[i].status << endl;
    }
  }

  // shuffle players
  shufflePlayers(advancingPlayers, advancingCount);

  cout << "\nPlayers Advancing from Qualifiers: \n";
  displayPlayers(advancingPlayers, advancingCount);
}

void assignPlayersToGroups()
{
  int grpCount = advancingCount / 4;

  for (int group = 0; group < grpCount; group++)
  {
    for (int player = 0; player < 4; player++)
    {
      advancingPlayers[group * 4 + player].group = group + 1;
    }
  }

  updatePlayerList();
  savePlayersToFile("data/players.csv");
  displayMatchBrackets();
}

// void generateKnockoutMatches()
// {
//   matchCount = 0;
//   int n = advancingCount;
//   for (int i = 0; i < n / 2; i++)
//   {
//     matches[matchCount].matchIndex = matchCount;
//     sprintf(matches[matchCount].matchID, "M%03d", matchCount + 1);
//     matches[matchCount].stage = KNOCKOUT_STAGE;
//     matches[matchCount].player1Index = advancingPlayers[i];
//     matches[matchCount].player2Index = advancingPlayers[n - 1 - i];
//     matches[matchCount].winnerIndex = -1;
//     strcpy(matches[matchCount].status, "Scheduled");
//     matches[matchCount].groupID = 0;
//     matches[matchCount].startTime = 1800 + i * 30;
//     matches[matchCount].isPlayed = false;
//     matchCount++;
//   }
//   saveMatchesToFile("data/matches.csv");
// }

void generateRoundRobinMatches()
{
  groupMatchQueue.init(); // Reset the queue

  // Iterate through each group
  for (int g = 0; g < MAX_GROUPS; g++)
  {
    int size = groupCounts[g]; // Get the size of the current group
    int matchIdx = 0;          // Start match indexing from 0 for each group

    // Generate matches for all player pairs in this group
    for (int i = 0; i < size - 1; i++)
    {
      for (int j = i + 1; j < size; j++)
      {
        Match m;
        m.matchIndex = matchCount;                           // Match index as current matchCount
        sprintf(m.matchID, "G%dM%02d", g + 1, matchIdx + 1); // Create match ID like G1M01, G1M02, ...
        m.stage = GROUP_STAGE;                               // It's a group stage match
        m.player1Index = groups[g][i];                       // Player 1 from the group
        m.player2Index = groups[g][j];                       // Player 2 from the group
        m.winnerIndex = -1;                                  // Winner is undecided
        strcpy(m.status, "Scheduled");                       // Match status as scheduled
        m.groupID = g;                                       // Store the group ID
        m.startTime = 0;                                     // Optional: Can set start time if needed
        m.isPlayed = false;                                  // Mark match as not played

        // Add match to the global matches array
        matches[matchCount] = m;
        // Enqueue the match for scheduling
        groupMatchQueue.enqueue(m);

        matchCount++; // Increment match count
        matchIdx++;   // Increment match index for next match
      }
    }
  }

  // Save the matches to the file after generating all matches
  saveMatchesToFile("data/matches.csv");

  cout << "Round-robin group stage matches generated and saved.\n";
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

#include <iomanip> // For std::setw

#include <iomanip> // For std::setw

#include <iostream>
#include <iomanip> // For std::setw, std::fixed, std::setprecision
#include <cmath>   // For floor()

void printMatches(TournamentStage stage)
{
  // Print the header with the updated column names
  cout << left << setw(12) << "Match ID"
       << setw(12) << "Date"
       << setw(12) << "Start Time"
       << setw(12) << "Status"
       << setw(20) << "Player 1"
       << setw(20) << "Player 2"
       << setw(17) << "Duration (min)"
       << "Winner\n";
  cout << "-------------------------------------------------------------------------------------------------------------------\n";

  for (int i = 0; i < matchCount; i++)
  {
    if (matches[i].stage == stage)
    {
      // Calculate duration in minutes with seconds as decimal
      float oriTime = matches[i].durationInSeconds / 60;
      float min = floor(matches[i].durationInSeconds / 60);
      float finalTime = (oriTime - min) * 60 / 100 + min;
      float durationInMinutes = (matches[i].isPlayed) ? finalTime : -1;

      // Format the date (YYYY-MM-DD)
      int year = matches[i].date / 10000;
      int month = (matches[i].date / 100) % 100;
      int day = matches[i].date % 100;

      // Print the match details
      cout << left << setw(12) << matches[i].matchID
           << setw(12) << to_string(year) + "-" + (month < 10 ? "0" : "") + to_string(month) + "-" + (day < 10 ? "0" : "") + to_string(day)
           << setw(12) << matches[i].startTime // Print the start time
           << setw(12) << matches[i].status
           << setw(20) << players[matches[i].player1Index].name
           << setw(20) << players[matches[i].player2Index].name
           << setw(15); // for duration

      // Print duration (or - if match is not completed)
      if (durationInMinutes >= 0)
        cout << setw(17) << finalTime; // Print the duration in minutes
      else
        cout << setw(17) << "-"; // Print "-" for incomplete matches

      // Print winner (or - if match is not completed)
      if (matches[i].isPlayed)
      {
        cout << players[matches[i].winnerIndex].name;
      }
      else
      {
        cout << "-"; // Print "-" if match is not completed
      }

      cout << "\n";
    }
  }
  cout << "\n";
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
      // currentStage = KNOCKOUT_STAGE;
      // generateKnockoutMatches();
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

bool getWinnerDone(int matchIndex)
{
  if (matchIndex > matchCount - 1)
  {
    return true;
  }

  // get winner id
  bool invalidInput = true;
  int winnerChoice;

  while (invalidInput)
  {
    if (matches[matchIndex].isPlayed)
    {
      return false;
    }
    cout << matches[matchIndex].matchID << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Who won? Enter 1 for "
         << players[matches[matchIndex].player1Index].name
         << ", 2 for "
         << players[matches[matchIndex].player2Index].name
         << " (0 to quit): ";

    cin >> winnerChoice;
    if (cin.fail())
    {
      cin.clear();
      cin.ignore();
    }

    if (winnerChoice == 0)
    {
      return true;
    }
    else if (winnerChoice == 1)
    {
      setMatchResult(matches[matchIndex].matchIndex, matches[matchIndex].player1Index);
      invalidInput = false;
    }
    else if (winnerChoice == 2)
    {
      setMatchResult(matches[matchIndex].matchIndex, matches[matchIndex].player1Index);
      invalidInput = false;
    }
    else
    {
      cout << "Invalid choice.\n\n";
      return false;
    }
  }

  // get match duration
  invalidInput = true;
  int time;
  while (invalidInput)
  {
    cout << "Enter match duration in seconds (max 1800s) : ";
    cin >> time;
    if (cin.fail())
    {
      cin.clear();
      cin.ignore();
    }
    if (time > 0 && time <= 1800)
    {
      invalidInput = false;
      matches[matchIndex].durationInSeconds = time;
      cout << "here" << matches[matchIndex].durationInSeconds << endl;
    }
  }
  saveMatchesToFile("date/matches.csv");
  savePlayersToFile("date/players.csv");
  return false;
}

void inputMatchResult()
{
  if (currentStage == QUALIFIERS)
  {
    bool done = false;
    int inputtingMatch = -1;
    while (!done && !allMatchesPlayedInStage(QUALIFIERS))
    {
      inputtingMatch++;
      printMatches(QUALIFIERS);
      done = getWinnerDone(inputtingMatch);
    }

    if (allMatchesPlayedInStage(QUALIFIERS))
    {
      cout << "All matches have been completed. Please move on to the next stage.\n";
    }
  }

  saveMatchesToFile("data/matches.csv");
}

void inputMatchResultCopy()
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
      {
        cout << "Invalid choice, match skipped.\n";
        return;
      }

      // Get match duration
      int durationInSeconds;
      cout << "Enter match duration in seconds (max 1800): ";
      cin >> durationInSeconds;
      if (durationInSeconds < 0 || durationInSeconds > 1800)
      {
        cout << "Invalid duration. It must be between 0 and 1800 seconds.\n";
        return;
      }

      // Save the duration for the match
      matches[m.matchIndex].durationInSeconds = durationInSeconds;
      cout << "Hello";
      cout << matches[m.matchIndex].durationInSeconds;
      // Save the result to file
      saveMatchesToFile("data/matches.csv");
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
      {
        cout << "Invalid choice.\n";
        return;
      }

      // Get match duration
      int durationInSeconds;
      cout << "Enter match duration in seconds (max 1800): ";
      cin >> durationInSeconds;
      if (durationInSeconds < 0 || durationInSeconds > 1800)
      {
        cout << "Invalid duration. It must be between 0 and 1800 seconds.\n";
        return;
      }

      // Save the duration for the match
      matches[matchID].durationInSeconds = durationInSeconds;
      cout << "hello";
      cout << matches[matchID].durationInSeconds;
      // Save the result to file
      saveMatchesToFile("data/matches.csv");
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
    {
      cout << "Invalid choice.\n";
      return;
    }

    // Get match duration
    int durationInSeconds;
    cout << "Enter match duration in seconds (max 1800): ";
    cin >> durationInSeconds;
    if (durationInSeconds < 0 || durationInSeconds > 1800)
    {
      cout << "Invalid duration. It must be between 0 and 1800 seconds.\n";
      return;
    }

    // Save the duration for the match
    matches[matchID].durationInSeconds = durationInSeconds;

    // Save the result to file
    saveMatchesToFile("data/matches.csv");
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
  displayPlayers(players, playerCount);

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
      savePlayersToFile("data/players.csv");
      saveMatchesToFile("data/matches.csv");
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
