#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <vector>

using namespace std;

struct MarkerInfo {
    int id;
    vector<int> marks;
    int markCount = 0;
};

extern mutex cs;
extern condition_variable cvStart;
extern condition_variable cvRemove;
extern condition_variable cvBlocked;

extern bool startSignal;
extern bool removeSignal;
extern bool continueSignal;
extern int removeId;

extern vector<int> arr;
extern atomic<int> dim;
extern atomic<int> remainingThreads;
extern atomic<int> blockedCount;
extern vector<bool> isBlocked;
extern vector<bool> isAlive;

void markerThread(MarkerInfo* info);
