#include "arrays.h"
#include <algorithm>

void initArray(std::vector<int>& arr) {
    std::fill(arr.begin(), arr.end(), 0);
}

bool tryMarkElement(std::vector<int>& arr, size_t index, int id) {
    if (index >= arr.size()) return false;
    if (arr[index] == 0) {
        arr[index] = id;
        return true;
    }
    return false;
}

void clearMarks(std::vector<int>& arr, int id) {
    for (int& x : arr) {
        if (x == id) x = 0;
    }
}

int countMarks(const std::vector<int>& arr, int id) {
    int c = 0;
    for (int x : arr) if (x == id) ++c;
    return c;
}

