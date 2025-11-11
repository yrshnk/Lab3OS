#include <gtest/gtest.h>
#include "head.h"
#include "marker.h"
#include "marker.cpp"
#include <windows.h>
#include <vector>

CRITICAL_SECTION cs;
HANDLE hStartSignal, hContinueSignal, hRemoveEvent;
HANDLE* hFinishEvents;
int* arr;
volatile int dim;
volatile int rem;

TEST(MarkerTest, SingleMarkerMarksArrayAndClearsOnRemove) {
    dim = 10;
    arr = new int[dim] {};

    InitializeCriticalSection(&cs);

    hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinueSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    hFinishEvents = new HANDLE[1];
    hFinishEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);

    DWORD id;
    HANDLE hMarker = CreateThread(NULL, 0, marker, (LPVOID)1, 0, &id);

    SetEvent(hStartSignal);

    Sleep(50);

    bool marked = false;
    EnterCriticalSection(&cs);
    for (int i = 0; i < dim; i++) {
        if (arr[i] == 1) {
            marked = true;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    ASSERT_TRUE(marked) << "Marker должен был пометить хот€ бы один элемент массива";

    rem = 1;
    PulseEvent(hRemoveEvent);
    WaitForSingleObject(hMarker, INFINITE);

    EnterCriticalSection(&cs);
    for (int i = 0; i < dim; i++) {
        ASSERT_EQ(arr[i], 0);
    }
    LeaveCriticalSection(&cs);

    CloseHandle(hMarker);
    CloseHandle(hFinishEvents[0]);
    CloseHandle(hStartSignal);
    CloseHandle(hContinueSignal);
    CloseHandle(hRemoveEvent);
    DeleteCriticalSection(&cs);
    delete[] arr;
    delete[] hFinishEvents;
}

TEST(MarkerTest, MultMarkersDiffElements) {
    dim = 20;
    arr = new int[dim] {};
    InitializeCriticalSection(&cs);

    hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinueSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    int numMarkers = 3;
    hFinishEvents = new HANDLE[numMarkers];
    HANDLE* hMarkers = new HANDLE[numMarkers];

    for (int i = 0; i < numMarkers; i++) {
        hFinishEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        DWORD id;
        hMarkers[i] = CreateThread(NULL, 0, marker, (LPVOID)(i + 1), 0, &id);
    }

    SetEvent(hStartSignal);
    Sleep(100);

    int markedCount = 0;
    EnterCriticalSection(&cs);
    for (int i = 0; i < dim; i++) {
        if (arr[i] != 0) markedCount++;
    }
    LeaveCriticalSection(&cs);

    ASSERT_GT(markedCount, 0);
    ASSERT_LE(markedCount, dim);

    for (int i = 0; i < numMarkers; i++) {
        TerminateThread(hMarkers[i], 0);
        CloseHandle(hMarkers[i]);
        CloseHandle(hFinishEvents[i]);
    }

    CloseHandle(hStartSignal);
    CloseHandle(hContinueSignal);
    CloseHandle(hRemoveEvent);
    DeleteCriticalSection(&cs);
    delete[] arr;
    delete[] hFinishEvents;
}

TEST(MarkerTest, ArrayReus) {
    dim = 10;
    arr = new int[dim] {};
    InitializeCriticalSection(&cs);

    hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinueSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hFinishEvents = new HANDLE[1];
    hFinishEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);

    DWORD id;
    HANDLE hMarker = CreateThread(NULL, 0, marker, (LPVOID)1, 0, &id);
    SetEvent(hStartSignal);
    Sleep(50);

    rem = 1;
    PulseEvent(hRemoveEvent);
    WaitForSingleObject(hMarker, INFINITE);

    for (int i = 0; i < dim; i++) ASSERT_EQ(arr[i], 0);

    arr[3] = 42;
    ASSERT_EQ(arr[3], 42);

    CloseHandle(hMarker);
    CloseHandle(hFinishEvents[0]);
    CloseHandle(hStartSignal);
    CloseHandle(hContinueSignal);
    CloseHandle(hRemoveEvent);
    DeleteCriticalSection(&cs);
    delete[] arr;
    delete[] hFinishEvents;
}

TEST(MarkerTest, MultMarkersTerminate) {
    dim = 15;
    arr = new int[dim] {};
    InitializeCriticalSection(&cs);

    hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinueSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    int numMarkers = 2;
    hFinishEvents = new HANDLE[numMarkers];
    HANDLE* hMarkers = new HANDLE[numMarkers];

    for (int i = 0; i < numMarkers; i++) {
        hFinishEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        DWORD id;
        hMarkers[i] = CreateThread(NULL, 0, marker, (LPVOID)(i + 1), 0, &id);
    }

    SetEvent(hStartSignal);
    Sleep(100);

    for (int i = 0; i < numMarkers; i++) {
        rem = i + 1;
        PulseEvent(hRemoveEvent);
        WaitForSingleObject(hMarkers[i], INFINITE);
        CloseHandle(hMarkers[i]);
        CloseHandle(hFinishEvents[i]);
    }

    for (int i = 0; i < dim; i++) ASSERT_EQ(arr[i], 0);

    CloseHandle(hStartSignal);
    CloseHandle(hContinueSignal);
    CloseHandle(hRemoveEvent);
    DeleteCriticalSection(&cs);
    delete[] arr;
    delete[] hFinishEvents;
}
