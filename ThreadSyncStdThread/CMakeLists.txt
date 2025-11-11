cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ThreadSyncStdThread")

set(CMAKE_CXX_STANDARD 17)

add_library(markerLib "marker.cpp""marker.h")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (ThreadSyncStdThread "ThreadSyncStdThread.cpp" "ThreadSyncStdThread.h" "marker.cpp" "marker.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ThreadSyncStdThread PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
add_executable (testMarker "testStdThread.cpp" "marker.h")
target_link_libraries(ThreadSyncStdThread PRIVATE markerLib)
target_link_libraries(testMarker PRIVATE markerLib)
