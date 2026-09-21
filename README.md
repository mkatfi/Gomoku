# ⛩️ Gomoku AI Engine — Pente Variant

> **A 1337 (42 Network) Student Project** — A high-performance C++ implementation of Gomoku (Pente variant) featuring a custom graphical interface built with **SFML 3** and a heavily optimized AI engine capable of searching multiple turns ahead in **under 0.5 seconds**.

---

<!-- SEO & Discovery Tags -->
<!--
Tags: gomoku, pente, board-game, AI, minimax, alpha-beta-pruning, SFML, C++, C++20,
game-engine, artificial-intelligence, negamax, transposition-table, zobrist-hashing,
iterative-deepening, PVS, LMR, 1337-school, 42-network, student-project, gomoku-ai,
game-tree-search, move-ordering, five-in-a-row, capture-game, 42cursus
-->

![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue?style=flat-square&logo=c%2B%2B)
![SFML 3](https://img.shields.io/badge/Graphics-SFML%203-green?style=flat-square)
![School](https://img.shields.io/badge/School-1337%20%7C%2042%20Network-orange?style=flat-square)
![AI](https://img.shields.io/badge/AI-Minimax%20%2B%20Alpha--Beta-purple?style=flat-square)
![Response Time](https://img.shields.io/badge/AI%20Response-under%200.5s-red?style=flat-square)
![License](https://img.shields.io/badge/License-Academic-lightgrey?style=flat-square)

---

## 📋 Table of Contents

- [About the Project](#-about-the-project)
- [Game Rules](#-game-rules)
- [Artificial Intelligence](#-artificial-intelligence)
- [Architecture](#-architecture)
- [Prerequisites & Dependencies](#️-prerequisites--dependencies)
- [Build & Run](#-build--run)
- [Controls](#-controls-cheat-sheet)
- [Bonus Features](#-bonus-features)
- [Roadmap](#️-roadmap)

---

## 🏫 About the Project

This project was developed as part of the **1337 School curriculum** (a member of the **42 Network**), one of the most rigorous peer-to-peer coding school programs in the world.

The goal was to build a **complete, competitive Gomoku engine** from scratch — no game libraries, no pre-built AI frameworks — enforcing every rule of the professional Pente ruleset, while keeping the AI response time strictly **under 0.5 seconds** regardless of board state or search depth.

**Key achievements:**
- 🧠 AI regularly reaches **search depth 9–10** within the time budget
- ⚡ Worst-case AI move time: **< 0.5 seconds**
- 🎮 Full graphical interface with menus, themes, and in-game overlays
- ✅ Exhaustive rule-validation test suite included

---

## 📜 Game Rules

This engine acts as a **strict referee** for the subject's Pente ruleset. Every
human and AI move goes through the same legality checks before it is accepted.
The rule engine is independent from SFML, so it can be tested on a headless
machine with `make test`.

| Rule | Description |
|---|---|
| **5-in-a-Row** | Align 5 or more stones in any direction (horizontal, vertical, diagonal) to win. |
| **Capture Victory** | Capture **10 of your opponent's stones** (5 pairs) to win — even if they have a 5-in-a-row. |
| **Capture Mechanic** | Flank an **exact pair** of opponent stones on both sides with your stones to capture and remove them. |
| **Breakable Five** | A 5-in-a-row does **not** win immediately if, on the very next turn, the opponent can capture a stone out of that line **or** reach their 10th capture. |
| **Double-Three Rule** | A player **cannot** place a stone that simultaneously creates two or more open-threes — *unless* that same move also captures opponent stones. |

> ⚠️ The Double-Three and Breakable Five rules are the most commonly misimplemented in student projects. This engine handles both correctly.

---

## 🧠 Artificial Intelligence

The AI is designed to be **aggressive, defensive, and extremely fast**. It strictly respects the `< 0.5s` response time constraint through a layered stack of advanced search techniques.

---

### 🔷 Minimax Algorithm (Negamax formulation)

The foundation of the AI. It simulates all possible future moves, always assuming the opponent plays perfectly, and navigates the **game tree** to find the mathematically best move.

This engine uses the **Negamax** variant — a cleaner single-perspective formulation of Min-Max where each node simply **negates the child's score**, removing the need for separate min/max logic.

---

### ✂️ Alpha-Beta Pruning

Attached on top of Negamax, Alpha-Beta pruning **eliminates branches** of the game tree that are provably worse than already-discovered moves. This cuts the effective branching factor dramatically — reducing billions of potential calculations down to thousands — with **zero loss in decision quality**.

---

### 🔑 Zobrist Hashing & Transposition Tables

Every unique board state is **fingerprinted** with a 64-bit integer using XOR-based Zobrist hashing. Scores for previously evaluated positions are stored in a **Transposition Table** (a custom hash cache). If the same board state is reached through a different move order, the result is retrieved in **O(1)** — no re-evaluation needed.

---

### 📊 Move Ordering

Before the AI evaluates candidate moves, it **sorts them** — placing captures, threats, and winning sequences first. Because Alpha-Beta pruning is most effective when the best moves are explored early, good move ordering delivers roughly a **10× speedup** in pruning efficiency.

---

### 🔄 Iterative Deepening

Rather than committing to a fixed search depth, the AI searches at depth 1, then depth 2, then depth 3, and so on. When the **~420ms internal timer** fires, the current search is cleanly aborted (via a managed exception that unwinds safely), and the **best move from the last fully completed depth** is returned — guaranteeing a valid move is always ready well under the 0.5s limit.

---

### 🔬 Principal Variation Search (PVS) & Late Move Reductions (LMR)

These two techniques push the search to its practical limits:

- **PVS:** After finding the best move in a node, later moves are first tested with a **null window** (minimal alpha-beta window). If they fail to beat the current best, they are discarded cheaply. If one unexpectedly exceeds the window, a full re-search is triggered.
- **LMR:** Quiet, low-priority moves are searched at a **reduced depth** first. Only if a reduced-depth result is surprisingly strong does the engine do a full-depth re-search.

Together, PVS + LMR allow the AI to routinely reach **depth 9–10** within the time budget, with tactical or forcing positions (captures, threats) consistently reaching deeper.

---

### 🧩 Summary of AI Techniques

| Technique | Purpose |
|---|---|
| Negamax | Core game tree search |
| Alpha-Beta Pruning | Eliminates provably bad branches |
| Zobrist Hashing | Unique fingerprint per board state |
| Transposition Table | O(1) cache lookup for repeated states |
| Move Ordering | Search best moves first for maximum pruning |
| Iterative Deepening | Time-safe depth search with guaranteed fallback |
| PVS | Efficient null-window probing of later moves |
| LMR | Reduced-depth search for quiet low-priority moves |
| Killer / History Heuristics | Move ordering refinement across sibling nodes |

---

## 🏗️ Architecture

The codebase follows a strict **decoupled layered architecture**, ensuring thread safety and zero state corruption during deep AI simulations. Each layer has a single, well-defined responsibility.

```
src/
├── core/                  # Fundamental shared data types
│   └── Types.hpp          # Cell enums, Point struct, MoveResult struct
│
├── engine/                # Pure game logic — no UI, no turn management
│   ├── Board              # Data container: cell storage, bounds checking, state arrays
│   ├── Rules              # 100% stateless static rule-validation functions
│   ├── GameEngine         # State mutator: applyMove(), undoMove(), win detection
│   └── Zobrist            # 64-bit random key generator for board state hashing
│
├── game/                  # Orchestration layer
│   └── GameSession        # Turn switching, AI scheduling, UI state string generation
│
├── gui/                   # SFML presentation layer (no game logic)
│   ├── GameWindow         # Window lifecycle, event loop, screen routing
│   ├── InputHandler       # Converts raw pixel coordinates to board grid coordinates
│   └── Renderer           # Pure drawing: grid, hoshi points, stones, text, overlays
│
└── ai/                    # Artificial intelligence subsystem
    ├── AI                 # Time management, iterative deepening loop, exception safety
    ├── MoveGenerator      # Reduces 361 candidates to ~30 nearby moves, sorts by priority
    ├── Evaluator          # Scoring function: reads board lines, assigns values (+100,000 for win)
    └── TranspositionTable # Custom array-based cache for Zobrist hash entries
```

**Design principles enforced throughout:**
- The `engine/` layer has **no knowledge** of the UI or AI layers
- The `ai/` layer operates on a **copy of the board** — it never touches live game state
- All rule validation is **stateless** — the same function called with the same inputs always returns the same result

---

## 🛠️ Prerequisites & Dependencies

| Requirement | Version / Details |
|---|---|
| **C++ Compiler** | C++20 or later (`g++` or `clang++`) |
| **Make** | Standard GNU Make |
| **SFML** | Version 3.x (uses SFML 3.0 API — `openFromFile`, `std::optional` event polling) |

> ⚠️ **SFML 2.x is not compatible.** The SFML 3.0 API has breaking changes from 2.x. Make sure you install version 3.

---

### Installing SFML

**Ubuntu / Debian (Linux):**
```bash
sudo apt update
sudo apt install libsfml-dev
```

**macOS (Homebrew):**
```bash
brew install sfml
```

**Arch Linux:**
```bash
sudo pacman -S sfml
```

---

## 🚀 Build & Run

### Compile the project
```bash
make
```

### Launch the game
```bash
./Gomoku
```

**Game flow:**
1. **Title screen** → click **Start Game**
2. Choose **Mode**: Player vs AI, or Hotseat (Player vs Player)
3. If vs AI: select **difficulty** (Easy / Medium / Hard) and **your color**
4. Play!

> During a match, the **bottom bar** shows the AI's last move time in milliseconds. The **top-left** shows whose turn it is (with a colored stone indicator). The most recently placed stone is highlighted with a **red ring**. Press **? CONTROLS** (top-right) to open the in-game controls overlay.

---

### Run the rule-validation test suite
```bash
make test
```
This builds only the engine and AI test binary, so it does not require a
display or SFML installation. The suite covers pair-only captures, the
double-three prohibition and capture exception, capture counters, full-board
handling, forbidden-move rejection, and an AI legal-move smoke test.

---

### Clean build artifacts
```bash
make clean    # Remove compiled object files only
make fclean   # Remove object files AND the compiled executable
make re       # Full clean rebuild from scratch
```

---

## 🎮 Controls Cheat-Sheet

| Input | Action |
|---|---|
| **Left Mouse Click** | Navigate menus / place a stone on the board |
| **Enter** | Confirm / Start (on the title screen) |
| **`H`** | Show a move hint (suggested move is highlighted — not played automatically) |
| **`U`** | Undo last move |
| **`Y`** | Redo last undone move |
| **`T`** | Toggle Dark / Light theme (works on every screen) |
| **`R`** | Return to the main menu |
| **`Esc`** | Go back one screen |
| **`E` / `M` / `D`** | Set difficulty to Easy / Medium / Hard (on the AI setup screen) |
| **? CONTROLS** (top-right) | Open / close the in-game controls overlay |

---

## ✨ Bonus Features

All bonus features are clearly marked in the source code with a `// BONUS (...)` comment.
To find every bonus location, run:

```bash
grep -rn "BONUS" src/
```

---

### 1. 🎯 AI Difficulty Selector
On the AI setup screen, choose **Easy**, **Medium**, or **Hard**. Each maps to a different per-move time budget:

| Difficulty | Time Budget | Typical Search Depth |
|---|---|---|
| Easy | 100 ms | Depth 4–5 |
| Medium | 250 ms | Depth 6–7 |
| Hard | 420 ms | Depth 9–10 |

A larger budget allows iterative deepening to complete more plies — the AI literally **thinks further ahead** on harder difficulties.

**How to verify:** Select Easy vs Hard and compare the `"AI last move: N ms"` display and the `Depth N done` terminal output.

---

### 2. ↩️ Undo (`U`)
Takes back the last move, **fully restoring** captured stones and resetting the capture counter.

- In **vs-AI mode**: rewinds a full round (the AI's reply **and** your move) so it is your turn again
- In **hotseat mode**: rewinds exactly one ply

`GameEngine::undoMove()` returns the full move record so the session can restore board state cleanly.

**How to verify:** Set up a capture, press `U`, confirm the captured pair reappears and the counter decrements.

---

### 3. 🔁 Redo (`Y`)
Re-applies moves that were just undone (mirroring undo's one-ply / one-round behavior).

Undone moves are tracked on a **redo stack**, which is cleared the moment you make a new move — so you cannot redo into an abandoned branch.

**How to verify:** Undo a move, press `Y` to restore it. Then undo, play somewhere else, and confirm `Y` now has no effect.

---

### 4. 🌙 Dark / Light Theme (`T`)
Pressing `T` re-skins the **entire application** — board background, grid lines, hoshi dots, stone outlines, and all text — between a warm **light wood** theme and a cool **dark slate** theme. Works on all menus and screens.

**How to verify:** Press `T` repeatedly on any screen.

---

### 5. 🔴 Last-Move Marker
The most recently placed stone is highlighted with a **red ring**, making it immediately clear where the last move was played — especially useful after the AI moves.

*(The green ring shown with `H` is the separate move-suggestion indicator.)*

**How to verify:** Play any move — a red ring appears on it and updates after every subsequent move.

---

### 6. 🖥️ Quality-of-Life UI
- Multi-screen **gamer-style menu** system with smooth screen transitions
- **Turn indicator** (colored stone + player label) in the top-left corner
- **? CONTROLS** overlay accessible from within a live game
- Real-time **AI move-time display** in milliseconds at the bottom of the screen

---

## 🗺️ Roadmap

- [ ] Network multiplayer (online 1v1)
- [ ] Opening book for early-game AI improvement
- [ ] Move time visualization (per-move thinking graph)
- [ ] Replay system (save and review full games)
- [ ] Custom board sizes (13×13, 15×15 toggle)

---

## 🤝 Contributing && 🏷️ Keywords & Tags

This project is part of an academic program at **1337 School (42 Network)**. Contributions, feedback, and issue reports are welcome — especially regarding rule-edge-case bugs or AI evaluation improvements.


`gomoku` `pente` `five-in-a-row` `board-game-ai` `minimax` `negamax` `alpha-beta-pruning`
`principal-variation-search` `late-move-reduction` `transposition-table` `zobrist-hashing`
`iterative-deepening` `move-ordering` `C++20` `SFML3` `game-engine` `1337-school`
`42-network` `42cursus` `student-project` `artificial-intelligence` `game-tree-search`
`capture-game` `double-three` `breakable-five` `competitive-ai`