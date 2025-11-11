#include "ThreadSyncStdThread.h"

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

struct MarkerInfo {
    int id;
    vector<int> marks;
    int markCount = 0;
};

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
                return (removeSignal && removeId == info->id) || continueSignal;
                });

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

int main() {
    setlocale(LC_ALL, "ru");

    int tmpDim;
    cout << "Введите размерность массива: ";
    cin >> tmpDim;
    dim = tmpDim;

    arr.assign(dim, 0);

    int numThreads;
    cout << "Введите количество потоков marker: ";
    cin >> numThreads;

    isBlocked.assign(numThreads, false);
    isAlive.assign(numThreads, true);

    remainingThreads = numThreads;
    blockedCount = 0;

    vector<thread> threads;
    vector<MarkerInfo> infos(numThreads);

    for (int i = 0; i < numThreads; ++i) {
        infos[i].id = i + 1;
        infos[i].marks.assign(dim, 0);
        threads.emplace_back(markerThread, &infos[i]);
    }

    {
        lock_guard<mutex> lock(cs);
        startSignal = true;
    }
    cvStart.notify_all();

    while (remainingThreads.load() > 0) {
        {
            unique_lock<mutex> lock(cs);
            cvBlocked.wait(lock, [] {
                return (remainingThreads.load() > 0) && (blockedCount.load() == remainingThreads.load());
                });
        }

        {
            lock_guard<mutex> lock(cs);
            cout << "\nНа данный момент массив имеет вид:\n";
            for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
            cout << endl;
        }

        int delId = -1;
        while (true) {
            cout << "Введите порядковый номер потока marker, который необходимо завершить: ";
            cin >> delId;
            if (delId < 1 || delId > numThreads) {
                cout << "Некорректный номер. Повторите.\n";
                continue;
            }
            if (!isAlive[delId - 1]) {
                cout << "Этот поток уже завершён. Выберите другой.\n";
                continue;
            }
            break;
        }

        {
            lock_guard<mutex> lock(cs);
            removeSignal = true;
            removeId = delId;
            continueSignal = false; 
        }
        cvRemove.notify_all();

        if (threads[delId - 1].joinable()) {
            threads[delId - 1].join();
        }

        {
            lock_guard<mutex> lock(cs);
            cout << "Поток " << delId << " завершён. Текущее состояние массива:\n";
            for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
            cout << endl;

            removeSignal = false;
            removeId = -1;
            continueSignal = true;
        }

        cvRemove.notify_all();

        {
            unique_lock<mutex> lock(cs);
            cvBlocked.wait(lock, [] {
                return (blockedCount.load() == 0) || (blockedCount.load() == remainingThreads.load());
                });
            continueSignal = false;
        }

    }

    cout << "\nВсе потоки завершены.\nЗаключительное состояние массива:\n";
    for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
    cout << endl;

    return 0;
}
