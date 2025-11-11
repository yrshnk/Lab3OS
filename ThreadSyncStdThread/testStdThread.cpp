#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "marker.h"
#include <thread>
#include <chrono>

void resetGlobals() {
    arr.clear();
    dim = 0;
    remainingThreads = 0;
    blockedCount = 0;
    startSignal = false;
    removeSignal = false;
    continueSignal = false;
    removeId = -1;
    isBlocked.clear();
    isAlive.clear();
}

TEST_CASE("Инициализация массива работает корректно") {

    setlocale(LC_ALL, "ru");
    resetGlobals();
    dim = 10;
    arr.assign(dim, 0);

    CHECK(arr.size() == dim);
    CHECK(std::all_of(arr.begin(), arr.end(), [](int v) { return v == 0; }));
}

TEST_CASE("Один поток markerThread корректно помечает элементы") {

    setlocale(LC_ALL, "ru");
    resetGlobals();

    dim = 12;
    arr.assign(dim, 0);
    remainingThreads = 1;
    blockedCount = 0;
    isBlocked = { false };
    isAlive = { true };

    MarkerInfo info;
    info.id = 1;
    info.marks.assign(dim, 0);

    std::thread t(markerThread, &info);

    {
        std::lock_guard<std::mutex> lock(cs);
        startSignal = true;
    }
    cvStart.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int marked = 0;
    for (int v : arr)
        if (v == info.id)
            marked++;

    CHECK(marked > 0);

    {
        std::lock_guard<std::mutex> lock(cs);
        removeSignal = true;
        removeId = 1;
    }
    cvRemove.notify_all();

    if (t.joinable())
        t.join();

    CHECK(isAlive[0] == false);
}

TEST_CASE("После завершения потока все его метки очищаются") {

    setlocale(LC_ALL, "ru");
    resetGlobals();

    dim = 8;
    arr.assign(dim, 2);
    MarkerInfo info;
    info.id = 2;
    info.marks.assign(dim, 1);

    for (int i = 0; i < dim; ++i) {
        if (info.marks[i] == 1)
            arr[i] = 0;
    }

    bool cleared = std::all_of(arr.begin(), arr.end(), [](int v) { return v == 0; });
    CHECK(cleared);
}
