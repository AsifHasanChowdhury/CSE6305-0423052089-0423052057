#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>

#define MEMORY_SIZE 1024
#define NUM_ACTIONS 3 // Defragment, Delay, Adjust Intensity
#define ALPHA 0.1 // Learning rate
#define GAMMA 0.9 // Discount factor
#define EPSILON 0.2 // Exploration rate

using namespace std;

// Q-table for Reinforcement Learning
double QTable[MEMORY_SIZE][NUM_ACTIONS] = {0};

// Memory Handle Structure
struct Handle {
    void* ptr;
    size_t size;
    bool isPinned;
};

// Memory Manager Class
class MemoryManager {
private:
    unordered_map<int, Handle> handleTable;
    vector<bool> memory;
    
public:
    MemoryManager() : memory(MEMORY_SIZE, false) {}
    
    int allocate(size_t size) {
        for (int i = 0; i < MEMORY_SIZE - size; i++) {
            bool canAllocate = true;
            for (int j = 0; j < size; j++) {
                if (memory[i + j]) {
                    canAllocate = false;
                    break;
                }
            }
            if (canAllocate) {
                for (int j = 0; j < size; j++) memory[i + j] = true;
                handleTable[i] = {malloc(size), size, false};
                return i;
            }
        }
        return -1;
    }
    
    void free(int handleId) {
        if (handleTable.find(handleId) != handleTable.end()) {
            free(handleTable[handleId].ptr);
            for (int i = 0; i < handleTable[handleId].size; i++) memory[handleId + i] = false;
            handleTable.erase(handleId);
        }
    }
    
    void defragment() {
        cout << "Defragmenting Memory..." << endl;
        // Simple compaction logic
        vector<Handle> activeHandles;
        for (auto &entry : handleTable) {
            activeHandles.push_back(entry.second);
        }
        handleTable.clear();
        memory.assign(MEMORY_SIZE, false);
        
        int offset = 0;
        for (auto &handle : activeHandles) {
            handleTable[offset] = handle;
            for (int j = 0; j < handle.size; j++) memory[offset + j] = true;
            offset += handle.size;
        }
    }
};

// Reinforcement Learning-based Defragmentation Decision
int chooseAction(int state) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<> dis(0, 1);
    
    if (dis(gen) < EPSILON) {
        return rand() % NUM_ACTIONS; // Explore
    } else {
        int bestAction = 0;
        for (int i = 1; i < NUM_ACTIONS; i++) {
            if (QTable[state][i] > QTable[state][bestAction]) {
                bestAction = i;
            }
        }
        return bestAction;
    }
}

void updateQTable(int state, int action, int reward, int nextState) {
    double bestNextQ = *max_element(QTable[nextState], QTable[nextState] + NUM_ACTIONS);
    QTable[state][action] = QTable[state][action] + ALPHA * (reward + GAMMA * bestNextQ - QTable[state][action]);
}

int main() {
    MemoryManager memManager;
    int state = MEMORY_SIZE / 2; // Initial memory fragmentation level
    
    for (int i = 0; i < 100; i++) { // Simulate memory operations
        int action = chooseAction(state);
        if (action == 0) {
            memManager.defragment();
            cout << "Defragmentation executed." << endl;
        }
        int reward = -state; // Less fragmentation is better
        int nextState = max(0, state - action);
        updateQTable(state, action, reward, nextState);
        state = nextState;
    }
    return 0;
}
