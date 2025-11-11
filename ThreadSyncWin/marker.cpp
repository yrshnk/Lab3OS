#include "marker.h"

DWORD WINAPI marker(LPVOID number) {
    WaitForSingleObject(hStartSignal, INFINITE);

    int n = (int)number;
    srand(n);

    int mark = 0;
    int* marks = new int[dim] {};

    while (true) {
        int tmp = rand();
        int index = tmp % dim;

        EnterCriticalSection(&cs);
        if (arr[index] == 0) {
            Sleep(5);
            arr[index] = n;
            marks[index] = 1;
            mark++;
            Sleep(5);
            LeaveCriticalSection(&cs);
        }
        else {
            std::cout << "Порядковый номер потока: " << n << std::endl;
            std::cout << "Количество помеченных элементов: " << mark << std::endl;
            std::cout << "Индекс элемента массива, который невозможно пометить: " << index << std::endl;
            std::cout << std::endl;
            LeaveCriticalSection(&cs);

            SetEvent(hFinishEvents[n - 1]);

            WaitForSingleObject(hRemoveEvent, INFINITE);

            if (rem == n) {
                EnterCriticalSection(&cs);
                for (int i = 0; i < dim; i++) {
                    if (marks[i] == 1) {
                        arr[i] = 0;
                    }
                }
                LeaveCriticalSection(&cs);
                delete[] marks;
                SetEvent(hFinishEvents[n - 1]);
                return 0;
            }
            else {
                ResetEvent(hFinishEvents[n - 1]);
                WaitForSingleObject(hContinueSignal, INFINITE);
            }
        }
    }
}
