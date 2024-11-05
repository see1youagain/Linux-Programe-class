# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ThreadPoolSortingProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(ThreadPoolSortingProject main.cpp ThreadPool.cpp SortMerge.cpp)

# Add the following line to link pthread library
target_link_libraries(ThreadPoolSortingProject pthread)
