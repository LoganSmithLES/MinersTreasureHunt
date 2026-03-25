#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#endif

#define SIZE 12
#define WALL '#'
#define PLAYER 'P'
#define BLOCK 'X'
#define EMPTY ' '
#define GOLD 'G'
#define IRON 'I'
#define COBALT 'C'
#define NICKEL 'N'

#define GOLD_VAL 5
#define IRON_VAL 3
#define COBALT_VAL 4
#define NICKEL_VAL 2

#define MAX_INPUT 100
#define MAX_ORES 10  // Maximum number of ores per type in the inventory

typedef struct {
    int level;
    int hammer;
    char textColor[10];
    time_t start;
    char exitDoor;
    int exitDoorX;
    int exitDoorY;
    int oresLeft;
    int money;
    int playerX;
    int playerY;
    int inventory[4];  // Array to store the count of each type of ore in inventory
    int totalOresBroughtOut;  // Total ores brought out to be displayed in leaderboard
} GameState;

#define LEADERBOARD_FILE_PATH "leaderboard.txt.txt"
#define CREDITS_FILE_PATH "credits.txt.txt"
#define HOW_TO_PLAY_FILE_PATH "HowToPlay.txt.txt"

void initGrid(char grid[SIZE][SIZE], GameState *state);
void placeBlocks(char grid[SIZE][SIZE], int wallPercent, int orePercent, GameState *state);
void showLoading();
void showGrid(char grid[SIZE][SIZE], GameState *state);
void playGame(GameState *state);
void shop(GameState *state);
void movePlayer(char grid[SIZE][SIZE], GameState *state, char move);
void sellOre(GameState *state);
void saveToLeaderboard(const char *playerName, int timePassed, int money, int totalOresBroughtOut);
void displayLeaderboard();
void displayCredits();
void howToPlay();
FILE *openDataFile(const char *fileName, const char *mode);
/////////////////////////////////////////////////////////////////
FILE *openDataFile(const char *fileName, const char *mode) {
    FILE *file = fopen(fileName, mode);
    if (file != NULL) {
        return file;
    }

    const char *fallbackPrefixes[] = {"../", "../../", "..\\", "..\\..\\", NULL};
    char pathBuffer[260];
    for (int i = 0; fallbackPrefixes[i] != NULL; i++) {
        snprintf(pathBuffer, sizeof(pathBuffer), "%s%s", fallbackPrefixes[i], fileName);
        file = fopen(pathBuffer, mode);
        if (file != NULL) {
            return file;
        }
    }

    return NULL;
}

void showLoading() {
    printf("Loading");
    for (int i = 0; i < 10; i++) {
        printf(".");
        fflush(stdout);
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    printf("\n");
}

void showGrid(char grid[SIZE][SIZE], GameState *state) {
    system(CLEAR_SCREEN);
    printf("%s", state->textColor);
    printf("Level: %d\n", state->level);
    long timeRemaining = (long)(state->start + 300 - time(NULL));
    if (timeRemaining < 0) {
        timeRemaining = 0;
    }
    printf("Time Remaining: %ld seconds\n", timeRemaining); // Print remaining time
    printf("Money: $%d\n", state->money);
    printf("Ores left: %d\n", state->oresLeft);
    printf("Inventory - Iron: %d/10, Gold: %d/10, Cobalt: %d/10, Nickel: %d/10\n", state->inventory[1], state->inventory[0], state->inventory[2], state->inventory[3]);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (i == state->playerY && j == state->playerX)
                printf("%c ", PLAYER);
            else if (state->level == 3 && i == state->exitDoorY && j == state->exitDoorX)
                printf("%c ", state->exitDoor); // Print exit door for level 3
            else
                printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
}

void initGrid(char grid[SIZE][SIZE], GameState *state) {
    int wallPercent = 15 + (state->level == 2 ? 2 : (state->level == 3 ? 5 : 0));
    int orePercent = 20 + (state->level - 1) * 5;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = EMPTY;
        }
        grid[i][0] = WALL;
        grid[i][SIZE-1] = WALL;
    }
    for (int j = 0; j < SIZE; j++) {
        grid[0][j] = WALL;
        grid[SIZE-1][j] = WALL;
    }
    if (state->level == 3) {
        do {
            state->exitDoorX = rand() % (SIZE - 2) + 1;
            state->exitDoorY = rand() % (SIZE - 2) + 1;
        } while (grid[state->exitDoorY][state->exitDoorX] != EMPTY);  // Ensure the exit is placed in an empty space
    }
    placeBlocks(grid, wallPercent, orePercent, state);
}

void placeBlocks(char grid[SIZE][SIZE], int wallPercent, int orePercent, GameState *state) {
    srand(time(NULL) + state->level);
    state->oresLeft = 0;
    for (int i = 1; i < SIZE - 1; i++) {
        for (int j = 1; j < SIZE - 1; j++) {
            if ((i <= 2 && j <= 2) || (i == state->exitDoorY && j == state->exitDoorX)) continue;
            int chance = rand() % 100;
            if (chance < wallPercent) {
                grid[i][j] = BLOCK;
            } else if (chance < wallPercent + orePercent) {
                int type = rand() % 4;
                grid[i][j] = type == 0 ? GOLD : type == 1 ? IRON : type == 2 ? COBALT : NICKEL;
                state->oresLeft++;
            } else {
                grid[i][j] = EMPTY;
            }
        }
    }
}

void shop(GameState *state) {
    int choice;
    time_t shopEnterTime = time(NULL);
    printf("\nWelcome to the Shop!\n");
    printf("1. Stop Watch - $25 - Reduces Timer by 30 Seconds\n");
    printf("2. SledgeHammer - $50 - Removes next impassable wall (except border)\n");
    printf("3. BLOOD! - $1 - Changes the color of all text into red\n");
    printf("4. Sell Ores - Sell your collected ores for money\n");
    printf("5. Sell All Ores - Sell all your collected ores for money\n");
    printf("6. Exit Shop\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    getchar();  // Clear the input buffer after reading
    switch (choice) {
        case 1:
            if (state->money >= 25) {
                state->start += 30;  // Add 30 seconds to the timer
                state->money -= 25;
                printf("Timer increased by 30 seconds!\n");
            } else {
                printf("Not enough money!\n");
            }
            break;
        case 2:
            if (state->money >= 50) {
                state->money -= 50;
                state->hammer = 1;
                printf("SledgeHammer purchased! Use it on your next impassable wall!\n");
            } else {
                printf("Not enough money!\n");
            }
            break;
        case 3:
            if (state->money >= 1) {
                state->money -= 1;
                strcpy(state->textColor, "\033[1;31m");
                printf("%sBLOOD purchased! Enjoy the new look!\033[0m\n", state->textColor);
            } else {
                printf("Not enough money!\n");
            }
            break;
        case 4:
            sellOre(state);
            break;
        case 5:
            printf("You sold all your ores for $%d\n", (state->inventory[0] * GOLD_VAL) + (state->inventory[1] * IRON_VAL) +
                                                           (state->inventory[2] * COBALT_VAL) + (state->inventory[3] * NICKEL_VAL));
            state->money += (state->inventory[0] * GOLD_VAL) + (state->inventory[1] * IRON_VAL) +
                            (state->inventory[2] * COBALT_VAL) + (state->inventory[3] * NICKEL_VAL);
            memset(state->inventory, 0, sizeof(state->inventory));  // Clear inventory
            break;
        case 6:
            printf("Exiting shop.\n");
            break;
        default:
            printf("Invalid option.\n");
            break;
    }
    getchar();  // Wait for user to press a key to return to the game
    state->start += difftime(time(NULL), shopEnterTime);  // Adjust the timer for time spent in the shop
}


void sellOre(GameState *state) {
    printf("\nSelling Ores:\n");
    printf("1. Sell Gold (Value: $%d)\n", GOLD_VAL);
    printf("2. Sell Iron (Value: $%d)\n", IRON_VAL);
    printf("3. Sell Cobalt (Value: $%d)\n", COBALT_VAL);
    printf("4. Sell Nickel (Value: $%d)\n", NICKEL_VAL);
    printf("5. Cancel\n");
    printf("Enter choice: ");
    int choice;
    scanf("%d", &choice);
    getchar(); // Clear the input buffer after reading
    switch (choice) {
        case 1:
            if (state->inventory[0] > 0) {
                state->money += GOLD_VAL * state->inventory[0];
                state->inventory[0] = 0;
                printf("Sold Gold.\n");
            } else {
                printf("You don't have any Gold to sell.\n");
            }
            break;
        case 2:
            if (state->inventory[1] > 0) {
                state->money += IRON_VAL * state->inventory[1];
                state->inventory[1] = 0;
                printf("Sold Iron.\n");
            } else {
                printf("You don't have any Iron to sell.\n");
            }
            break;
        case 3:
            if (state->inventory[2] > 0) {
                state->money += COBALT_VAL * state->inventory[2];
                state->inventory[2] = 0;
                printf("Sold Cobalt.\n");
            } else {
                printf("You don't have any Cobalt to sell.\n");
            }
            break;
        case 4:
            if (state->inventory[3] > 0) {
                state->money += NICKEL_VAL * state->inventory[3];
                state->inventory[3] = 0;
                printf("Sold Nickel.\n");
            } else {
                printf("You don't have any Nickel to sell.\n");
            }
            break;
        case 5:
            printf("Cancelled.\n");
            break;
        default:
            printf("Invalid option.\n");
            break;
    }
}

void movePlayer(char grid[SIZE][SIZE], GameState *state, char move) {
    int newX = state->playerX, newY = state->playerY;
    switch (move) {
        case 'w': newY--; break;
        case 's': newY++; break;
        case 'a': newX--; break;
        case 'd': newX++; break;
        case 'i': case 'I':
            shop(state);
            return;
        default:
            printf("Invalid input! Please use WASD keys or I for Shop.\n");
            getchar();
            return;
    }

    if (newX >= 1 && newX < SIZE - 1 && newY >= 1 && newY < SIZE - 1) {
        if (grid[newY][newX] == WALL || (grid[newY][newX] == BLOCK && !state->hammer)) {
            printf("Can't move there!\n");
            getchar();
            return;
        }
        if (grid[newY][newX] == BLOCK && state->hammer) {
            grid[newY][newX] = EMPTY;
            state->hammer = 0;
        }

        // Check if player reached the exit door
        if (state->level == 3 && newX == state->exitDoorX && newY == state->exitDoorY) {
            if (state->oresLeft == 0 && state->money >= 100) {
                printf("You have enough money and all ores are collected. Do you want to exit? (Y/N): ");
                char decision;
                scanf(" %c", &decision);
                if (tolower(decision) == 'y') {
                    printf("Exiting with $100. You win!\n");
                    saveToLeaderboard("Player", (int)difftime(time(NULL), state->start), state->money, state->totalOresBroughtOut);
                    exit(0);
                }
            } else if (state->oresLeft == 0 && state->money < 100) {
                printf("You don't have enough money and no ores left to collect. You lose!\n");
                exit(0);
            } else {
                printf("You cannot exit yet. Collect more ores or earn enough money.\n");
                getchar();
            }
            return;
        }

        // Check inventory before picking up ore
        if ((grid[newY][newX] == GOLD && state->inventory[0] < MAX_ORES) ||
            (grid[newY][newX] == IRON && state->inventory[1] < MAX_ORES) ||
            (grid[newY][newX] == COBALT && state->inventory[2] < MAX_ORES) ||
            (grid[newY][newX] == NICKEL && state->inventory[3] < MAX_ORES)) {
            int oreIndex = (grid[newY][newX] == GOLD ? 0 :
                            grid[newY][newX] == IRON ? 1 :
                            grid[newY][newX] == COBALT ? 2 : 3);
            state->inventory[oreIndex]++;
            state->totalOresBroughtOut++;
            grid[newY][newX] = EMPTY;  // Remove ore from grid
        } else if (strchr("GICN", grid[newY][newX]) != NULL) {
            printf("Inventory full for this ore type, cannot collect more.\n");
            getchar();  // Allow player to read the message before clearing
        }

        state->playerX = newX;
        state->playerY = newY;
    } else {
        printf("Move out of bounds!\n");
        getchar();  // Allow player to read the message before clearing
    }
}

void saveToLeaderboard(const char *playerName, int timePassed, int money, int totalOresBroughtOut) {
    FILE *file = openDataFile(LEADERBOARD_FILE_PATH, "a");
    if (file != NULL) {
        fprintf(file, "%s %d %d %d\n", playerName, timePassed, money, totalOresBroughtOut);
        fclose(file);
    } else {
        perror("Error: Couldn't save to leaderboard");
    }
}

void displayLeaderboard() {
    FILE *file = openDataFile(LEADERBOARD_FILE_PATH, "r");
    if (file != NULL) {
        printf("\n\nLeaderboard:\n");
        char playerName[50];
        int timePassed, money, totalOresBroughtOut;
        while (fscanf(file, "%s %d %d %d", playerName, &timePassed, &money, &totalOresBroughtOut) != EOF) {
            printf("%s\t\t%d\t%d\t%d\n", playerName, timePassed, money, totalOresBroughtOut);
        }
        fclose(file);
        printf("\n");
    } else {
        perror("Error: Leaderboard file not found");
    }
}

void displayCredits() {
    FILE *file = openDataFile(CREDITS_FILE_PATH, "r");
    if (file != NULL) {
        printf("\n\n");
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        printf("\n");
        fclose(file);
    } else {
        perror("Error: Credits file not found");
    }
}

void howToPlay() {
    FILE *file = openDataFile(HOW_TO_PLAY_FILE_PATH, "r");
    if (file != NULL) {
        printf("\n\nHow to Play:\n");
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        printf("\n");
        fclose(file);
    } else {
        perror("Error: How to Play file not found");
    }
}

void playGame(GameState *state) {
    char grid[SIZE][SIZE];
    state->start = time(NULL);  // Start the timer at the beginning of the game

    while (state->level <= 3) {
        initGrid(grid, state);  // Initialize the grid for the current level
        while (1) {
            if (difftime(time(NULL), state->start) >= 300) {
                showGrid(grid, state);
                printf("You ran out of time and suffocated in the mines. Game over!\n");
                getchar();
                return;
            }
            showGrid(grid, state);  // Display the current state of the grid
            char move = getchar();
            getchar();  // Clear buffer after getting input
            movePlayer(grid, state, move);  // Move player based on input
            state->oresLeft = 0;  // Reset ores count for check
            for (int i = 1; i < SIZE - 1; i++) {
                for (int j = 1; j < SIZE - 1; j++) {
                    if (strchr("GINC", grid[i][j]) != NULL) {
                        state->oresLeft++;  // Count remaining ores
                    }
                }
            }
            if (state->oresLeft == 0 && state->level < 3) {
                state->level++;  // Advance to the next level if all ores are collected
                printf("Level completed. Moving to level %d!\n", state->level);
                getchar();  // Wait for user input before moving to next level
                break;
            }
        }
        if (state->level > 3) {
            printf("Congratulations! You've explored all levels.\n");
            saveToLeaderboard("Player", (int)difftime(time(NULL), state->start), state->money, state->totalOresBroughtOut);  // Include total ores brought out
            printf("Performance saved successfully!\n");
            break;
        }
    }
}

int main() {
    GameState state = {
        .level = 1,
        .hammer = 0,
        .textColor = "\033[0m",
        .exitDoor = 'E',
        .exitDoorX = -1,
        .exitDoorY = -1,
        .oresLeft = 0,
        .money = 0,
        .playerX = 1,
        .playerY = 1,
        .totalOresBroughtOut = 0  // Initialize total ores brought out
    };

    int menuChoice = 0;
    char input[MAX_INPUT];
    do {
        printf("#################################################################################\n");
        printf("|   ,               p       ,---.                               .  .         .   \n");
        printf("|\\ /| o                       |                                 |  |         |  \n");
        printf("| V | . ;-. ,-. ;-.   ,-.     |   ;-. ,-. ,-: ,-. . . ;-. ,-.   |--| . . ;-. |-\n");
        printf("|   | | | | |-' |     `-.     |   |   |-' | | `-. | | |   |-'   |  | | | | | |\n");
        printf("'   ' ' ' ' `-' '     `-'     '   '   `-' `-` `-' `-` '   `-'   '  ' `-` ' ' `-\n");
        printf("#################################################################################\n");
        printf("\n                              1 ~ Start Game                                     \n");
        printf("                              2 ~ Leaderboard                                    \n");
        printf("                              3 ~ Credits                                        \n");
        printf("                              4 ~ How to Play                                    \n");
        printf("                              5 ~ Exit                                           \n\n");
        printf("                  Please Enter The Number You Wish to Explore:");
        if (fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = 0; // Remove newline character if present
            if (sscanf(input, "%d", &menuChoice) == 1) {
                switch (menuChoice) {
                    case 1:
                        showLoading();
                        playGame(&state);
                        break;
                    case 2:
                        displayLeaderboard();
                        break;
                    case 3:
                        displayCredits();
                        break;
                    case 4:
                        howToPlay();
                        break;
                    case 5:
                        printf("Exiting...\n");
                        break;
                    default:
                        printf("Invalid choice! Please enter a number between 1 and 5.\n");
                }
            } else {
                printf("Invalid input! Please enter a number.\n");
            }
        }
    } while (menuChoice != 5);

    return 0;
}
