#include "ThreadSyncWin.h"
#include "head.h"
#include "marker.h"

CRITICAL_SECTION cs;
HANDLE hStartSignal, hContinueSignal, hRemoveEvent;
HANDLE* hTerminateSignals;
HANDLE* hFinishEvents;
int* arr;
volatile int dim;
volatile int rem;

int main() {
    setlocale(LC_ALL, "RU");
    int dimTemp;
    std::cout << "Введите размерность массива: ";
    std::cin >> dimTemp;
    dim = dimTemp;

    arr = new int[dim] {};

    int num;
    std::cout << "Введите количество потоков marker: ";
    std::cin >> num;

    InitializeCriticalSection(&cs);

    DWORD* IDMarkers = new DWORD[num];
    HANDLE* hMarkers = new HANDLE[num];
    hTerminateSignals = new HANDLE[num];
    hFinishEvents = new HANDLE[num];

    hStartSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hContinueSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
    hRemoveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    for (int i = 0; i < num; i++) {
        hMarkers[i] = CreateThread(NULL, 0, marker, (LPVOID)(i + 1), 0, &IDMarkers[i]);
        if (hMarkers[i] == NULL) {
            std::cerr << "Не удалось создать поток marker по номеру " << (i + 1) << std::endl;
            return 1;
        }

        hTerminateSignals[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hFinishEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    SetEvent(hStartSignal);

    int iteration = num;
    while (iteration > 0) {
        WaitForMultipleObjects(num, hFinishEvents, TRUE, INFINITE);

        EnterCriticalSection(&cs);
        std::cout << "\nНа данный момент массив имеет вид:\n";
        for (int i = 0; i < dim; i++) std::cout << arr[i] << " ";
        std::cout << std::endl;
        LeaveCriticalSection(&cs);

        std::cout << "Введите порядковый номер потока marker, который необходимо завершить: ";
        int delThread;
        std::cin >> delThread;

        if (delThread < 1 || delThread > num || hMarkers[delThread - 1] == NULL) {
            std::cout << "Некорректный номер потока. Повторите попытку.\n";
            SetEvent(hContinueSignal);
            continue;
        }

        rem = delThread;
        PulseEvent(hRemoveEvent);
        WaitForSingleObject(hMarkers[delThread - 1], INFINITE);
        CloseHandle(hMarkers[delThread - 1]);
        hMarkers[delThread - 1] = NULL;

        iteration--;

        EnterCriticalSection(&cs);
        std::cout << "Поток " << delThread << " завершён. Текущее состояние массива:\n";
        for (int i = 0; i < dim; i++) std::cout << arr[i] << " ";
        std::cout << std::endl;
        LeaveCriticalSection(&cs);

        ResetEvent(hRemoveEvent);
        PulseEvent(hContinueSignal);
    }

    std::cout << "\nВсе потоки завершены.\nЗаключительное состояние массива:\n";
    for (int i = 0; i < dim; i++) std::cout << arr[i] << " ";
    std::cout << std::endl;

    for (int i = 0; i < num; i++) {
        if (hMarkers[i]) CloseHandle(hMarkers[i]);
        CloseHandle(hTerminateSignals[i]);
        CloseHandle(hFinishEvents[i]);
    }

    CloseHandle(hStartSignal);
    CloseHandle(hContinueSignal);
    CloseHandle(hRemoveEvent);
    DeleteCriticalSection(&cs);

    return 0;
}

