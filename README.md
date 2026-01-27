# Drone Swarm Simulation

An autonomous drone swarm simulation developed as a project for the Operating Systems (SO) course. The system utilizes multi-process architecture and various Inter-Process Communication (IPC) mechanisms to simulate a lifecycle of a drone swarm.

## Features

- **Multi-process Architecture**: Separate processes for the Main Controller, Commander, Operator, and each Drone.
- **IPC Mechanisms**:
  - **Shared Memory**: Synchronized storage for drone states and base information.
  - **Semaphores**: Coordination of base capacity, narrow passages (one-way traffic), and thread-safe logging.
  - **Signals**: Real-time communication for platform management and "suicide attack" commands.
  - **Pipes**: Communication between the UI and the Commander manager.
- **TUI (Text User Interface)**: Real-time visualization using the `termbox2` library.
- **Comprehensive Logging**: Detailed event history saved to `simulation.log`.

## Architecture

- **Main Process**: Handles the UI, event loop, and resource cleanup.
- **Commander**: Listens for user input and dispatches signals to the Operator or specific Drones.
- **Operator**: Manages the base. Restocks missing drones every $T_k$ and adjusts base capacity.
- **Drone Worker**: Simulates flight cycles, battery drain, and charging. Drones retire after a set number of visits to the base or if destroyed by a command.

## Prerequisites

- C11 compatible compiler (gcc/clang)
- CMake 3.10+
- Linux environment (required for System V IPC and RT signals)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the simulation from the project root:

```bash
./build/bin/drone_swarm [initial_drone_count]
```

### Controls

- `q`: Quit simulation and cleanup resources.
- `a`: Add a platform (increase base capacity and target drone count).
- `r`: Remove a platform (decrease base capacity and target drone count).
- `k`: Trigger a suicide attack (prompts for a Drone ID to destroy).
- `Arrows / PgUp / PgDn`: Scroll through the drone list.

## Project Structure

- `src/main.c`: Entry point and UI logic.
- `src/drone_worker.c`: Individual drone simulation logic.
- `src/operator_manager.c`: Base restocking and platform management.
- `src/commander_manager.c`: Command signal dispatching.
- `src/utils.c`: IPC helper functions and thread-safe logging.
- `include/types.h`: Shared data structures and IPC definitions.

## Logging

All simulation events (spawn, flight, charging, signals, and destruction) are recorded with timestamps and PIDs in `simulation.log`.
