# 👻Ghost Hunt Simulator

## 🏠Overview

This project is a full multithreaded simulation of a paranormal investigation inside a haunted house. You control a team of **hunters**, each equipped with different devices, who move through a network of interconnected **rooms** in search of a roaming **ghost**. The ghost wanders randomly, leaves evidence, gets bored, and sometimes scares hunters.  

Hunters make decisions independently in their own threads:  
- They navigate room-to-room,  
- Collect evidence they detect with their current device,  
- Update a shared and synchronized **CaseFile**,  
- Manage their own fear and boredom levels,  
- And eventually decide whether to push forward or retreat back to the van.

The simulation ends when the hunters successfully identify the ghost using the gathered evidence, or when the team gives up and all hunters abandon the house. Logging utilities are provided to track all major actions (moves, evidence events, thread exits), making it easy to visualize the entire investigation

---

## 📁File Overview

- **defs.h** — Central header containing all enums, structs, constants, and shared typedefs.
- **casefile.c** — Manages the shared evidence CaseFile and synchronization for writing evidence.
- **evidence.c** — Utility functions for setting and checking evidence bits.
- **room.c** — Creates rooms, manages occupants, evidence, and room-level synchronization.
- **roomstack.c** — Stack implementation for tracking hunter movement history.
- **house.c** — Builds the house layout, connects rooms, and initializes major structures.
- **ghost.c** — Contains the ghost thread logic: movement, evidence dropping, boredom handling.
- **hunter.c** — Contains hunter thread logic: movement, device use, fear/boredom updates, evidence collection.
- **helpers.c / helpers.h** — Provided logging and utility functions used throughout the simulation.
- **main.c** — Entry point: initializes everything, spawns threads, waits for completion.
- **Makefile** — Compiles all C files into the final executable; includes clean and build rules.

---

## 🛠️How to Build and Run

From the project directory:

```bash
# 1. Build the project
make

# (optional) Rebuild from scratch
make clean
make

# 2. Run the project
./ghost_sim

