#include "marker.h"
#include <iostream>
#include <cstdlib>

using namespace std;

mutex cs;
condition_variable cvStart;
condition_variable cvRemove;
condition_variable cvBlocked;

bool startSignal = false;
bool removeSignal = false;
bool continueSignal = false;
int removeId = -1;

vector<int> arr;
atomic<int> dim{ 0 };
atomic<int> remainingThreads{ 0 };
atomic<int> blockedCount{ 0 };
vector<bool> isBlocked;
vector<bool> isAlive;

void markerThread(MarkerInfo* info) {
    {
        unique_lock<mutex> lock(cs);
        cvStart.wait(lock, [] { return startSignal; });
    }

    srand(info->id);
    info->markCount = 0;
    info->marks.assign(static_cast<size_t>(dim.load()), 0);

    while (true) {
        int localDim = dim.load();
        int index = rand() % localDim;
        {
            unique_lock<mutex> lock(cs);

            if (arr[index] == 0) {
                this_thread::sleep_for(chrono::milliseconds(5));
                arr[index] = info->id;
                info->marks[index] = 1;
                info->markCount++;
                this_thread::sleep_for(chrono::milliseconds(5));
                continue;
            }

            cout << "Порядковый номер потока: " << info->id << endl;
            cout << "Количество помеченных элементов: " << info->markCount << endl;
            cout << "Индекс элемента массива, который невозможно пометить: " << index << endl;
            cout << endl;

            if (!isBlocked[info->id - 1]) {
                isBlocked[info->id - 1] = true;
                blockedCount.fetch_add(1);
            }
        }
        cvBlocked.notify_one();
        {
            unique_lock<mutex> lock(cs);
            cvRemove.wait(lock, [info] {
                return (removeSignal && removeId == info->id) || continueSignal;});

            if (removeSignal && removeId == info->id) {
                for (int i = 0; i < dim; ++i) {
                    if (info->marks[i] == 1) arr[i] = 0;
                }

                isAlive[info->id - 1] = false;
                remainingThreads.fetch_sub(1);
                if (isBlocked[info->id - 1]) {
                    isBlocked[info->id - 1] = false;
                    blockedCount.fetch_sub(1);
                }

                cvBlocked.notify_one();

                return; 
            }

            if (continueSignal) {
                if (isBlocked[info->id - 1]) {
                    isBlocked[info->id - 1] = false;
                    blockedCount.fetch_sub(1);
                }
            }
        }
    }
}
