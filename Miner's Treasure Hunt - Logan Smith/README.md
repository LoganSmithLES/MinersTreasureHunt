# Miner's Treasure Hunt

Miner's Treasure Hunt is a terminal-based mining game written in C. Explore randomly generated mine layouts, collect ores, trade at the shop, and reach the exit with enough money to escape.

## Features

- 3-level progression with increasing map difficulty
- Randomized ore and obstacle placement
- Inventory system with per-ore carry limits
- In-game shop for upgrades and ore selling
- Timer-based pressure and win/loss conditions
- Persistent leaderboard saved to a local text file

## Controls

- `W` move up
- `A` move left
- `S` move down
- `D` move right
- `I` open shop

## Symbols

- `#` Border wall
- `P` Player
- `X` Boulder (impassable unless hammer is active)
- `G` Gold (`$5`)
- `I` Iron (`$3`)
- `C` Cobalt (`$4`)
- `N` Nickel (`$2`)
- `E` Exit door (Level 3, requires enough money and all ores collected)

## Build

### CMake

```bash
cmake -S . -B build
cmake --build build
```

### GCC (example)

```bash
gcc -std=c11 -O2 -o miners_treasure_hunt main.c
```

## Run

Run the executable from the project root (or a directory where the text data files are reachable):

- `HowToPlay.txt.txt`
- `credits.txt.txt`
- `leaderboard.txt.txt`

## Project Files

- `main.c` game logic
- `HowToPlay.txt.txt` in-game how-to instructions
- `credits.txt.txt` credits page content
- `leaderboard.txt.txt` saved leaderboard entries

## Notes

- The game uses ANSI color codes for text effects.
- Screen clearing uses `cls` on Windows and `clear` on non-Windows systems.
