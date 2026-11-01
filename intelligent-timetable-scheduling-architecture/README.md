# Intelligent Timetable Scheduling Architecture

**Course:** Computer Architecture & Parallel Processing (CCSE0304)  
**Project:** Intelligent Timetable Scheduling Architecture  
**Language:** C++17

This project is a small educational software simulation connecting timetable generation with computer architecture concepts described in the report.

## Features

- Models classes, faculty, rooms, and time slots.
- Checks faculty, room, and class clashes using equality comparisons (ALU-style checks).
- Demonstrates a custom instruction set: `LOAD`, `CHECK`, `ALLOCATE`, `STORE`, and `HALT`.
- Prints a conceptual fetch/decode/execute register trace with `--trace`.
- Uses a five-stage conceptual scheduling pipeline: Fetch, Decode, Conflict Check, Allocate, Write-back.
- Offers sequential and four-worker parallel scheduling modes.
- Protects shared check-and-allocate operations with a mutex to prevent double booking.
- Validates that the final timetable has no clashes.

## Build

Requires a C++17 compiler such as `g++`.

### Linux / macOS / WSL

```bash
g++ -std=c++17 -O2 -pthread src/main.cpp -o timetable
```

### Windows (MinGW-w64)

```bash
g++ -std=c++17 -O2 -pthread src/main.cpp -o timetable.exe
```

## Run

Sequential mode:

```bash
./timetable
```

Parallel mode:

```bash
./timetable --parallel
```

Show the instruction-cycle trace:

```bash
./timetable --trace
```

Parallel mode with trace:

```bash
./timetable --parallel --trace
```

On Windows, replace `./timetable` with `.\timetable.exe`.

## Example output

The exact order of allocation messages and elapsed time can vary in parallel mode.

```text
Intelligent Timetable Scheduling Architecture
Mode: sequential
Allocated class 101 | faculty 1 | room 101 | slot 1
...
Conflict validation: PASS - zero clashes
```

## Architecture mapping

| Report concept | Code demonstration |
|---|---|
| Data representation | `Request` and `Allocation` records |
| ALU-based clash checking | Equality comparisons in `hasConflictUnlocked` |
| Control unit / instruction cycle | `Instruction` and `runInstruction` |
| Pipeline | Five conceptual stages printed in the final summary |
| Parallel processing | `std::thread` workers under `--parallel` |
| Shared-state consistency | Mutex protects the check-and-allocate operation |
| Cache concept | Small illustrative room/slot lookup map and hit/miss counters |

## Important scope note

This is a **software-level educational simulation**, not a hardware CPU emulator. The pipeline is represented conceptually, and the cache-like map is not a real CPU cache or a complete MSI/MESI coherence implementation. Timing output is measured on the machine running the program and is not expected to match any fixed value in the report. If your report includes specific benchmark numbers, rerun tests on your machine and update those numbers to the measurements you actually obtain.

## Suggested GitHub upload

1. Create a new repository on GitHub named `intelligent-timetable-scheduling-architecture`.
2. Extract this project folder.
3. Open a terminal inside the extracted folder and run:

```bash
git init
git add .
git commit -m "Add timetable scheduling architecture simulator"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/intelligent-timetable-scheduling-architecture.git
git push -u origin main
```

Replace `YOUR-USERNAME` with your GitHub username.
