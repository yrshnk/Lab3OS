cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ThreadSyncWin")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED) 

# Добавьте источник в исполняемый файл этого проекта.
add_executable (ThreadSyncWin "ThreadSyncWin.cpp" "ThreadSyncWin.h" "marker.cpp" "marker.h" "head.h")

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest)

target_include_directories(ThreadSyncWin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ThreadSyncWin PRIVATE Threads::Threads)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ThreadSyncWin PROPERTY CXX_STANDARD 20)
endif()

add_executable(markerTests "testMarker.cpp" "marker.h" "head.h" "marker.cpp")
target_include_directories(markerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(markerTests PRIVATE gtest gtest_main Threads::Threads)

include(GoogleTest)	
enable_testing()
gtest_discover_tests(markerTests)