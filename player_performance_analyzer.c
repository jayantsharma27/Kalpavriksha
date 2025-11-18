#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "Players_data.h"

#define NUM_OF_TEAMS 10
#define MIN_ID 1
#define MAX_ID 1000
#define MIN_PLAYERS_PER_TEAM 11
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH 50

typedef struct PlayerNode
{
    int playerId;
    char playerName[51];
    char teamName[51];
    char role[20];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
    struct PlayerNode *nextPlayer;
} PlayerNode;

typedef struct Team
{
    int teamId;
    char teamName[51];
    int totalPlayers;
    float avgStrikeRate;
    float totalBattingStrikeRate;
    int battingPlayerCount;
    PlayerNode *batters; 
    PlayerNode *bowlers;  
    PlayerNode *allRounders;
} Team;

Team gTeams[NUM_OF_TEAMS];

typedef struct
{
    PlayerNode *player;
} HeapNode;
void swapHeapNodes(HeapNode *a, HeapNode *b);
void heapifyDown(HeapNode heap[], int heapSize, int index); 
PlayerNode *heapExtractMax(HeapNode heap[], int *heapSize);          
void heapInsert(HeapNode heap[], int *heapSize, PlayerNode *player); 

int getTeamId(const char *teamName)
{
    for (int i = 0; i < teamCount; i++)
    {
        if (!strcmp(teamName, teams[i]))
        {
            return i + 1;
        }
    }
    return -1;
}

float calculatePerformanceIndex(PlayerNode *player)
{
    if (strcmp(player->role, "Batsman") == 0)
    {
        return (player->battingAverage * player->strikeRate) / 100.0;
    }
    if (strcmp(player->role, "Bowler") == 0)
    {
        return (player->wickets * 2.0) + (100.0 - player->economyRate);
    }
    if (strcmp(player->role, "All-rounder") == 0)
    {
        return (player->battingAverage * player->strikeRate) / 100.0 + (player->wickets * 2.0);
    }
    return 0.0;
}

void insertPlayerSorted(PlayerNode **listHead, PlayerNode *newPlayer) 
{
    if (*listHead == NULL || newPlayer->performanceIndex > (*listHead)->performanceIndex)
    {
        newPlayer->nextPlayer = *listHead;
        *listHead = newPlayer;
    }
    else
    {
        PlayerNode *current = *listHead;
        while (current->nextPlayer != NULL && current->nextPlayer->performanceIndex >= newPlayer->performanceIndex)
        {
            current = current->nextPlayer;
        }
        newPlayer->nextPlayer = current->nextPlayer;
        current->nextPlayer = newPlayer;
    }
}

int compareTeams(const void *a, const void *b)
{
    Team *teamA = (Team *)a;
    Team *teamB = (Team *)b;
    if (teamA->avgStrikeRate < teamB->avgStrikeRate)
        return 1;
    if (teamA->avgStrikeRate > teamB->avgStrikeRate)
        return -1;
    return 0;
}

bool isPlayerIdUnique(int playerId)
{
    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        PlayerNode *current = gTeams[i].batters;
        while (current != NULL)
        {
            if (current->playerId == playerId)
                return false;
            current = current->nextPlayer;
        }
        current = gTeams[i].bowlers;
        while (current != NULL)
        {
            if (current->playerId == playerId)
                return false;
            current = current->nextPlayer;
        }
        current = gTeams[i].allRounders;
        while (current != NULL)
        {
            if (current->playerId == playerId)
                return false;
            current = current->nextPlayer;
        }
    }
    return true;
}

void initializeData()
{
    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        gTeams[i].teamId = i + 1;
        strcpy(gTeams[i].teamName, teams[i]);
        gTeams[i].totalPlayers = 0;
        gTeams[i].avgStrikeRate = 0;
        gTeams[i].totalBattingStrikeRate = 0.0;
        gTeams[i].battingPlayerCount = 0;
        gTeams[i].batters = NULL;
        gTeams[i].bowlers = NULL;
        gTeams[i].allRounders = NULL;
    }

    for (int i = 0; i < playerCount; i++)
    {
        const Player *playerData = &players[i]; 
        int teamId = getTeamId(playerData->team); 
        int teamIndex = teamId - 1;
        PlayerNode *newPlayer = (PlayerNode *)malloc(sizeof(PlayerNode));
        if (newPlayer == NULL)
        {
            printf("Error: malloc failed during initialization.\n");
            exit(1);
        }

        newPlayer->playerId = playerData->id;            
        strcpy(newPlayer->playerName, playerData->name); 
        strcpy(newPlayer->teamName, playerData->team);  
        strcpy(newPlayer->role, playerData->role);
        newPlayer->totalRuns = playerData->totalRuns;
        newPlayer->battingAverage = playerData->battingAverage;
        newPlayer->strikeRate = playerData->strikeRate;
        newPlayer->wickets = playerData->wickets;
        newPlayer->economyRate = playerData->economyRate;
        newPlayer->nextPlayer = NULL;

        newPlayer->performanceIndex = calculatePerformanceIndex(newPlayer);

        gTeams[teamIndex].totalPlayers++;

        if (strcmp(newPlayer->role, "Batsman") == 0)
        {
            insertPlayerSorted(&gTeams[teamIndex].batters, newPlayer);
            gTeams[teamIndex].totalBattingStrikeRate += newPlayer->strikeRate;
            gTeams[teamIndex].battingPlayerCount++;
        }
        else if (strcmp(newPlayer->role, "Bowler") == 0)
        {
            insertPlayerSorted(&gTeams[teamIndex].bowlers, newPlayer);
        }
        else if (strcmp(newPlayer->role, "All-rounder") == 0)
        {
            insertPlayerSorted(&gTeams[teamIndex].allRounders, newPlayer);
            gTeams[teamIndex].totalBattingStrikeRate += newPlayer->strikeRate;
            gTeams[teamIndex].battingPlayerCount++;
        }
    }
}

void clearInputBuffer()
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void printPlayerHeader(void)
{
    printf("\n========================================================================================================\n");
    printf("ID   | Name                 | Role        | Runs  | Avg   | SR    | Wkts | ER    | Perf. Index\n");
    printf("========================================================================================================\n");
}

void printPlayerRow(PlayerNode *player) 
{
    if (player == NULL)
        return;
    printf("%-4d | %-20s | %-11s | %-5d | %-5.1f | %-5.1f | %-4d | %-5.1f | %.2f\n",
           player->playerId,
           player->playerName,
           player->role,
           player->totalRuns,
           player->battingAverage,
           player->strikeRate,
           player->wickets,
           player->economyRate,
           player->performanceIndex);
}

int getRoleChoice()
{
    int roleChoice = 0;
    while (roleChoice < 1 || roleChoice > 3)
    {
        printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
        if (scanf("%d", &roleChoice) != 1)
        {
            printf("\nInvalid input. Please enter 1, 2, or 3.\n");
            clearInputBuffer();
            continue;
        }
        if (roleChoice < 1 || roleChoice > 3)
        {
            printf("\nInvalid role. Please enter 1, 2, or 3.\n");
        }
    }
    clearInputBuffer();
    return roleChoice;
}

void addPlayer()
{
    int teamId, teamIndex;
    while (true)
    {
        printf("Enter Team ID (1-10): ");
        if (scanf("%d", &teamId) == 1 && teamId >= 1 && teamId <= NUM_OF_TEAMS)
        {
            clearInputBuffer();
            teamIndex = teamId - 1;
            break;
        }
        else
        {
            printf("\nInvalid team id. Enter a number between 1-10.\n");
            clearInputBuffer();
        }
    }

    if (gTeams[teamIndex].totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("Error: Team %s is full (max %d players).\n", gTeams[teamIndex].teamName, MAX_PLAYERS_PER_TEAM);
        return;
    }

    PlayerNode *newPlayer = (PlayerNode *)malloc(sizeof(PlayerNode));
    if (newPlayer == NULL)
    {
        printf("Error: Malloc failed!\n");
        return;
    }
    strcpy(newPlayer->teamName, gTeams[teamIndex].teamName);
    newPlayer->nextPlayer = NULL;

    printf("Enter Player Details:\n");
    while (true)
    {
        printf("Player ID (1-1000): ");
        if (scanf("%d", &newPlayer->playerId) == 1 && newPlayer->playerId >= MIN_ID && newPlayer->playerId <= MAX_ID)
        {
            if (isPlayerIdUnique(newPlayer->playerId))
            {
                clearInputBuffer();
                break;
            }
            else
            {
                printf("Error: Player ID %d already exists.\n", newPlayer->playerId);
                clearInputBuffer();
            }
        }
        else
        {
            printf("Invalid ID. Enter a number between 1-1000.\n");
            clearInputBuffer();
        }
    }

    printf("Name: ");
    fgets(newPlayer->playerName, sizeof(newPlayer->playerName), stdin);
    newPlayer->playerName[strcspn(newPlayer->playerName, "\n")] = 0;

    int roleChoice = getRoleChoice();

    while (true)
    {
        printf("Total Runs: ");
        if (scanf("%d", &newPlayer->totalRuns) == 1)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }
    while (true)
    {
        printf("Batting Average: ");
        if (scanf("%f", &newPlayer->battingAverage) == 1)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }
    while (true)
    {
        printf("Strike Rate: ");
        if (scanf("%f", &newPlayer->strikeRate) == 1)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }
    while (true)
    {
        printf("Wickets: ");
        if (scanf("%d", &newPlayer->wickets) == 1)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }
    while (true)
    {
        printf("Economy Rate: ");
        if (scanf("%f", &newPlayer->economyRate) == 1)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }

    switch (roleChoice)
    {
    case 1:
        strcpy(newPlayer->role, "Batsman");
        break;
    case 2:
        strcpy(newPlayer->role, "Bowler");
        break;
    case 3:
        strcpy(newPlayer->role, "All-rounder");
        break;
    }
    newPlayer->performanceIndex = calculatePerformanceIndex(newPlayer);
    gTeams[teamIndex].totalPlayers++;
    if (roleChoice == 1)
    {
        insertPlayerSorted(&gTeams[teamIndex].batters, newPlayer);
        gTeams[teamIndex].totalBattingStrikeRate += newPlayer->strikeRate;
        gTeams[teamIndex].battingPlayerCount++;
    }
    else if (roleChoice == 2)
    {
        insertPlayerSorted(&gTeams[teamIndex].bowlers, newPlayer);
    }
    else
    {
        insertPlayerSorted(&gTeams[teamIndex].allRounders, newPlayer);
        gTeams[teamIndex].totalBattingStrikeRate += newPlayer->strikeRate;
        gTeams[teamIndex].battingPlayerCount++;
    }

    printf("Player added successfully to %s!\n", gTeams[teamIndex].teamName);
}
void displayTeamPlayers()
{
    printf("\nEnter Team ID (1-10): ");
    int id;
    if (scanf("%d", &id) != 1 || id < 1 || id > NUM_OF_TEAMS)
    {
        printf("\nInvalid team id. Enter a number between 1-10.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int teamIndex = id - 1;
    Team *team = &gTeams[teamIndex];

    printf("\nPlayers of Team %s:\n", team->teamName);
    printPlayerHeader();

    PlayerNode *current = team->batters;
    while (current != NULL)
    {
        printPlayerRow(current);
        current = current->nextPlayer;
    }
    current = team->bowlers;
    while (current != NULL)
    {
        printPlayerRow(current);
        current = current->nextPlayer;
    }
    current = team->allRounders;
    while (current != NULL)
    {
        printPlayerRow(current);
        current = current->nextPlayer;
    }
    printf("========================================================================================================\n");

    if (team->battingPlayerCount > 0)
    {
        team->avgStrikeRate = team->totalBattingStrikeRate / team->battingPlayerCount;
    }
    else
    {
        team->avgStrikeRate = 0.0;
    }

    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->avgStrikeRate);
}

void displayTeamsByAvgStrikeRate(void)
{
    printf("\n--- Teams Sorted by Average Batting Strike Rate ---\n\n");

    Team tempTeams[NUM_OF_TEAMS];

    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        if (gTeams[i].battingPlayerCount > 0)
        {
            gTeams[i].avgStrikeRate = gTeams[i].totalBattingStrikeRate / gTeams[i].battingPlayerCount;
        }
        else
        {
            gTeams[i].avgStrikeRate = 0.0;
        }
        tempTeams[i] = gTeams[i];
    }

    qsort(tempTeams, NUM_OF_TEAMS, sizeof(Team), compareTeams);

    printf("====================================================\n");
    printf("ID | Team Name      | Avg Bat SR | Total Players\n");
    printf("====================================================\n");
    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        printf("%-2d | %-14s | %-10.2f | %d\n",
               tempTeams[i].teamId,
               tempTeams[i].teamName,
               tempTeams[i].avgStrikeRate,
               tempTeams[i].totalPlayers);
    }
    printf("====================================================\n");
}

void displayTopKPlayers()
{
    int teamId;
    printf("Enter Team ID (1-10): ");
    if (scanf("%d", &teamId) != 1 || teamId < 1 || teamId > NUM_OF_TEAMS)
    {
        printf("\nInvalid team id. Enter a number between 1-10.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    Team *team = &gTeams[teamId - 1];
    int roleChoice = getRoleChoice();
    int k;
    printf("Enter number of players (K): ");
    if (scanf("%d", &k) != 1 || k < 1)
    {
        printf("\nInvalid number for K. Must be > 0.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    PlayerNode *listHead = NULL;
    const char *roleName = "";
    if (roleChoice == 1)
    {
        listHead = team->batters;
        roleName = "Batsmen";
    }
    else if (roleChoice == 2)
    {
        listHead = team->bowlers;
        roleName = "Bowlers";
    }
    else
    {
        listHead = team->allRounders;
        roleName = "All-rounders";
    }

    printf("\nTop %d %s of %s:\n", k, roleName, team->teamName);
    printPlayerHeader();
    PlayerNode *current = listHead;
    int count = 0;
    for (int i = 0; i < k && current != NULL; i++)
    {
        printPlayerRow(current);
        current = current->nextPlayer;
        count++;
    }
    if (count == 0)
    {
        printf("No players found for this role.\n");
    }
    printf("========================================================================================================\n");
}

void swapHeapNodes(HeapNode *a, HeapNode *b)
{
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyDown(HeapNode heap[], int heapSize, int index) 
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heapSize && heap[left].player->performanceIndex > heap[largest].player->performanceIndex)
    {
        largest = left;
    }
    if (right < heapSize && heap[right].player->performanceIndex > heap[largest].player->performanceIndex)
    {
        largest = right;
    }
    if (largest != index)
    {
        swapHeapNodes(&heap[index], &heap[largest]); 
        heapifyDown(heap, heapSize, largest);
    }
}

PlayerNode *heapExtractMax(HeapNode heap[], int *heapSize) 
{
    if (*heapSize <= 0)
        return NULL;

    PlayerNode *maxPlayer = heap[0].player;
    heap[0] = heap[*heapSize - 1];
    (*heapSize)--;
    heapifyDown(heap, *heapSize, 0);

    return maxPlayer;
}

void heapInsert(HeapNode heap[], int *heapSize, PlayerNode *player)
{
    (*heapSize)++;
    int i = *heapSize - 1;
    heap[i].player = player;

    while (i != 0 && heap[(i - 1) / 2].player->performanceIndex < heap[i].player->performanceIndex)
    {
        swapHeapNodes(&heap[i], &heap[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void displayAllPlayersByRole()
{
    int roleChoice = getRoleChoice();
    const char *roleName = (roleChoice == 1) ? "Batsmen" : (roleChoice == 2) ? "Bowlers" : "All-rounders";

    HeapNode maxHeap[NUM_OF_TEAMS];
    int heapSize = 0;

    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        PlayerNode *listHead = NULL;
        if (roleChoice == 1)
            listHead = gTeams[i].batters;
        else if (roleChoice == 2)
            listHead = gTeams[i].bowlers;
        else
            listHead = gTeams[i].allRounders;

        if (listHead != NULL)
        {
            heapInsert(maxHeap, &heapSize, listHead);
        }
    }
    printPlayerHeader();

    int count = 0;
    while (heapSize > 0)
    {
        PlayerNode *bestPlayer = heapExtractMax(maxHeap, &heapSize);

        printPlayerRow(bestPlayer);
        count++;

        if (bestPlayer->nextPlayer != NULL)
        {
            heapInsert(maxHeap, &heapSize, bestPlayer->nextPlayer);
        }
    }

    if (count == 0)
    {
        printf("No players found for this role.\n");
    }
    printf("========================================================================================================\n");
}

void cleanupMemory()
{
    printf("\nFreeing memory...\n");
    for (int i = 0; i < NUM_OF_TEAMS; i++)
    {
        PlayerNode *current = gTeams[i].batters;
        while (current != NULL)
        {
            PlayerNode *temp = current;
            current = current->nextPlayer;
            free(temp);
        }
        gTeams[i].batters = NULL;

        current = gTeams[i].bowlers;
        while (current != NULL)
        {
            PlayerNode *temp = current;
            current = current->nextPlayer;
            free(temp);
        }
        gTeams[i].bowlers = NULL;

        current = gTeams[i].allRounders;
        while (current != NULL)
        {
            PlayerNode *temp = current;
            current = current->nextPlayer;
            free(temp);
        }
        gTeams[i].allRounders = NULL;
    }
}

int main()
{
    initializeData();
    printf("==============================================================================\n\n ICC ODI Player Performance Analyzer\n\n==============================================================================\n\n");

    printf("1. Add Player to Team\n2. Display Players of a Specific Team\n3. Display Teams by Average Batting Strike Rate\n4. Display Top K Players of a Specific Team by Role\n5. Display all Players of specific role Across All Teams by performance index\n6. Exit\n\n==============================================================================\n\n");

    int choice;
    do
    {
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid choice. Enter a number between 1-6\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice)
        {
        case 1:
            printf("\nChoice %d -> Add Player to Team\n", choice);
            addPlayer();
            break;
        case 2:
            printf("\nChoice %d -> Display All Players of a Specific Team\n", choice);
            displayTeamPlayers();
            break;
        case 3:
            printf("\nChoice %d -> Display Teams by Average Batting Strike Rate\n", choice);
            displayTeamsByAvgStrikeRate();
            break;
        case 4:
            printf("\nChoice %d -> Display Top K Players of a Specific Team by Role\n", choice);
            displayTopKPlayers();
            break;
        case 5:
            printf("\nChoice %d -> Display all Players of specific role Across All Teams\n", choice);
            displayAllPlayersByRole();
            break;
        case 6:
            printf("\nChoice %d -> Exit\n", choice);
            break;
        default:
            printf("\nInvalid choice. Enter a number between 1-6\n");
        }
    } while (choice != 6);

    cleanupMemory();
    return 0;
}