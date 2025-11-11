#include "ThreadSyncStdThread.h"
#include "marker.h"


int main() {
    std::ios::sync_with_stdio(false);
    setlocale(LC_ALL, "ru");

    int tmpDim;
    std::cout << "Введите размерность массива: ";
    std::cin >> tmpDim;
    dim = tmpDim;

    arr.assign(dim, 0);

    int numThreads;
    std::cout << "Введите количество потоков marker: ";
    std::cin >> numThreads;

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
        unique_lock<mutex> lock(cs);
        cvBlocked.wait(lock, [] {
            return (remainingThreads.load() > 0) && (blockedCount.load() == remainingThreads.load());
            });

        cout << "\nНа данный момент массив имеет вид:\n";
        for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
        cout << endl;
        
        lock.unlock();

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
            lock_guard<mutex> guard(cs);
            removeSignal = true;
            removeId = delId;
            continueSignal = false;
        }
        cvRemove.notify_all();

        if (threads[delId - 1].joinable()) {
            threads[delId - 1].join();
        }
        {
            lock_guard<mutex> guard(cs);
            cout << "Поток " << delId << " завершён. Текущее состояние массива:\n";
            for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
            cout << endl;

            removeSignal = false;
            removeId = -1;
            continueSignal = true;
        }

        cvRemove.notify_all();

        {
            unique_lock<mutex> lock2(cs);
            cvBlocked.wait(lock2, [] {
                return (blockedCount.load() == 0) || (blockedCount.load() == remainingThreads.load());
                });continueSignal = false;
        }
    }
    cout << "\nВсе потоки завершены.\nЗаключительное состояние массива:\n";
    for (int i = 0; i < dim; ++i) cout << arr[i] << " ";
    cout << endl;

    return 0;
}
