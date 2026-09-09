cmake_minimum_required(VERSION 3.16)
project(SearchX CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(include)

# Core engine as a library so both the CLI and the benchmark can link it.
add_library(searchx_core
    src/Tokenizer.cpp
    src/InvertedIndex.cpp
    src/Ranker.cpp
    src/TopK.cpp
    src/Trie.cpp
    src/LRUCache.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(searchx_core PUBLIC Threads::Threads)

add_executable(searchx src/main.cpp)
target_link_libraries(searchx PRIVATE searchx_core)

add_executable(benchmark benchmark/benchmark.cpp)
target_link_libraries(benchmark PRIVATE searchx_core)

add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE searchx_core)
