#pragma once
#include <vector>

void initArray(std::vector<int>& arr);
bool tryMarkElement(std::vector<int>& arr, size_t index, int id);
void clearMarks(std::vector<int>& arr, int id);
int countMarks(const std::vector<int>& arr, int id);
