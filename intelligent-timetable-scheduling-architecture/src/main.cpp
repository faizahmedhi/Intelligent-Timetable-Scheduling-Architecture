#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;

// Project: Intelligent Timetable Scheduling Architecture
// Course: Computer Architecture & Parallel Processing (CCSE0304)
//
// This is a software simulation of selected architecture concepts:
// 1. Encoded timetable records and ALU-style equality checks.
// 2. A small instruction set and fetch-decode-execute trace.
// 3. Five conceptual pipeline stages for each allocation instruction.
// 4. Parallel workers for scheduling requests.
// 5. A mutex-protected shared timetable to prevent conflicting writes.
//
// Note: This educational simulator models these concepts in software.
// It is not a cycle-accurate CPU, hardware cache, or real cache-coherence protocol.

struct Request {
    int classId;
    int facultyId;
    vector<int> preferredRooms;
    vector<int> candidateSlots;
};

struct Allocation {
    int classId;
    int facultyId;
    int roomId;
    int slotId;
};

struct Instruction {
    string opcode;
    int classId = -1;
    int facultyId = -1;
    int roomId = -1;
    int slotId = -1;
};

class TimetableScheduler {
private:
    vector<Allocation> timetable;
    mutex timetableMutex;
    mutex outputMutex;
    unordered_map<string, int> slotCache; // Illustrative cache-like lookup counter.
    size_t cacheHits = 0;
    size_t cacheMisses = 0;

    bool hasConflictUnlocked(int classId, int facultyId, int roomId, int slotId,
                             string &reason) const {
        for (const auto &entry : timetable) {
            if (entry.slotId != slotId) continue;

            // ALU-style equality comparisons for the three clash rules.
            if (entry.facultyId == facultyId) {
                reason = "Faculty clash";
                return true;
            }
            if (entry.roomId == roomId) {
                reason = "Room clash";
                return true;
            }
            if (entry.classId == classId) {
                reason = "Class clash";
                return true;
            }
        }
        reason.clear();
        return false;
    }

    string cacheKey(int roomId, int slotId) const {
        return to_string(roomId) + ":" + to_string(slotId);
    }

    bool tryAllocate(const Request &request, Allocation &result, string &failure) {
        lock_guard<mutex> lock(timetableMutex);

        for (int slotId : request.candidateSlots) {
            for (int roomId : request.preferredRooms) {
                const string key = cacheKey(roomId, slotId);
                auto found = slotCache.find(key);
                if (found != slotCache.end()) {
                    ++cacheHits;
                } else {
                    ++cacheMisses;
                    slotCache[key] = 1;
                }

                string reason;
                if (!hasConflictUnlocked(request.classId, request.facultyId,
                                         roomId, slotId, reason)) {
                    result = {request.classId, request.facultyId, roomId, slotId};
                    timetable.push_back(result);
                    // Invalidate the illustrative entry after a shared write.
                    slotCache.erase(key);
                    return true;
                }
                failure = reason + " for class " + to_string(request.classId) +
                          " at slot " + to_string(slotId);
            }
        }
        if (failure.empty()) failure = "No room/slot combination available";
        return false;
    }

public:
    bool hasConflict(int classId, int facultyId, int roomId, int slotId,
                     string &reason) const {
        lock_guard<mutex> lock(const_cast<mutex &>(timetableMutex));
        return hasConflictUnlocked(classId, facultyId, roomId, slotId, reason);
    }

    void runInstruction(const Instruction &instruction, bool showTrace) {
        // Conceptual register set for the CPU instruction-cycle demonstration.
        static int PC = 0;
        int MAR = PC;
        int MDR = PC;
        string IR = instruction.opcode;
        int ACC = instruction.classId;
        ++PC;

        if (showTrace) {
            lock_guard<mutex> outLock(outputMutex);
            cout << "\nInstruction: " << IR << "\n"
                 << "  T1 MAR <- PC       : MAR = " << MAR << "\n"
                 << "  T2 MDR <- M[MAR]   : MDR = " << MDR << "\n"
                 << "  T3 IR <- MDR       : IR  = " << IR << "\n"
                 << "  T4 Decode           : opcode selected = " << IR << "\n"
                 << "  T5 ALU/check        : class register ACC = " << ACC << "\n"
                 << "  T6 Write-back       : instruction simulation complete\n";
        }
    }

    bool scheduleOne(const Request &request, bool showTrace = false) {
        runInstruction({"LOAD", request.classId, request.facultyId}, showTrace);
        runInstruction({"CHECK", request.classId, request.facultyId}, showTrace);

        Allocation allocation{};
        string failure;
        bool allocated = tryAllocate(request, allocation, failure);

        if (allocated) {
            runInstruction({"ALLOCATE", allocation.classId, allocation.facultyId,
                            allocation.roomId, allocation.slotId}, showTrace);
            runInstruction({"STORE", allocation.classId, allocation.facultyId,
                            allocation.roomId, allocation.slotId}, showTrace);
            lock_guard<mutex> outLock(outputMutex);
            cout << "Allocated class " << allocation.classId
                 << " | faculty " << allocation.facultyId
                 << " | room " << allocation.roomId
                 << " | slot " << allocation.slotId << "\n";
        } else {
            lock_guard<mutex> outLock(outputMutex);
            cout << "Could not allocate class " << request.classId
                 << ": " << failure << "\n";
        }
        return allocated;
    }

    void printTimetable() const {
        lock_guard<mutex> lock(const_cast<mutex &>(timetableMutex));
        vector<Allocation> sorted = timetable;
        sort(sorted.begin(), sorted.end(), [](const Allocation &a, const Allocation &b) {
            if (a.slotId != b.slotId) return a.slotId < b.slotId;
            if (a.roomId != b.roomId) return a.roomId < b.roomId;
            return a.classId < b.classId;
        });

        cout << "\n========== FINAL TIMETABLE ==========\n";
        cout << left << setw(10) << "Class" << setw(12) << "Faculty"
             << setw(10) << "Room" << setw(10) << "Slot" << "\n";
        cout << string(42, '-') << "\n";
        for (const auto &entry : sorted) {
            cout << left << setw(10) << entry.classId
                 << setw(12) << entry.facultyId
                 << setw(10) << entry.roomId
                 << setw(10) << entry.slotId << "\n";
        }
        cout << "Total allocations: " << sorted.size() << "\n";
        cout << "Illustrative cache-like lookup hits: " << cacheHits
             << ", misses: " << cacheMisses << "\n";
    }

    bool validateNoConflicts() const {
        lock_guard<mutex> lock(const_cast<mutex &>(timetableMutex));
        for (size_t i = 0; i < timetable.size(); ++i) {
            for (size_t j = i + 1; j < timetable.size(); ++j) {
                if (timetable[i].slotId == timetable[j].slotId &&
                    (timetable[i].facultyId == timetable[j].facultyId ||
                     timetable[i].roomId == timetable[j].roomId ||
                     timetable[i].classId == timetable[j].classId)) {
                    return false;
                }
            }
        }
        return true;
    }
};

vector<Request> sampleRequests() {
    return {
        {101, 1, {101, 102}, {1, 2, 3}},
        {102, 2, {101, 102}, {1, 2, 3}},
        {103, 1, {101, 102}, {1, 2, 3}},
        {104, 3, {101, 102}, {1, 2, 3}},
        {105, 4, {101, 102}, {1, 2, 3}},
        {106, 5, {101, 102}, {1, 2, 3}}
    };
}

int main(int argc, char *argv[]) {
    bool parallelMode = false;
    bool showTrace = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--parallel") parallelMode = true;
        if (arg == "--trace") showTrace = true;
        if (arg == "--help") {
            cout << "Usage: timetable [--parallel] [--trace]\n"
                 << "Default runs sequentially; --parallel uses four worker threads.\n"
                 << "--trace prints a conceptual CPU instruction-cycle trace.\n";
            return 0;
        }
    }

    vector<Request> requests = sampleRequests();
    TimetableScheduler scheduler;

    cout << "Intelligent Timetable Scheduling Architecture\n";
    cout << "Mode: " << (parallelMode ? "parallel (up to 4 workers)" : "sequential") << "\n";

    auto start = Clock::now();
    if (parallelMode) {
        // Parallel workers process independent requests. The shared timetable
        // is protected by a mutex so check-and-allocate is one atomic operation.
        const unsigned workerCount = min(4u, static_cast<unsigned>(requests.size()));
        vector<thread> workers;
        vector<size_t> nextIndex(workerCount);
        for (unsigned w = 0; w < workerCount; ++w) {
            workers.emplace_back([&, w]() {
                for (size_t i = w; i < requests.size(); i += workerCount) {
                    scheduler.scheduleOne(requests[i], showTrace);
                }
            });
        }
        for (auto &worker : workers) worker.join();
    } else {
        for (const auto &request : requests) {
            scheduler.scheduleOne(request, showTrace);
        }
    }
    auto end = Clock::now();

    double elapsedMs = chrono::duration<double, milli>(end - start).count();
    scheduler.runInstruction({"HALT"}, showTrace);
    scheduler.printTimetable();

    cout << fixed << setprecision(3)
         << "\nElapsed time (this run): " << elapsedMs << " ms\n";
    cout << "Conflict validation: "
         << (scheduler.validateNoConflicts() ? "PASS - zero clashes" : "FAIL - clash found")
         << "\n";
    cout << "\nPipeline model: Fetch -> Decode -> Conflict Check -> Allocate -> Write-back\n";
    cout << "Note: measured time depends on your computer and run; do not treat it as a fixed benchmark.\n";

    return scheduler.validateNoConflicts() ? 0 : 1;
}
