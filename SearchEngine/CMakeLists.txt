cmake_minimum_required(VERSION 3.16)
project(SearchEngineSkeleton VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

find_package(SQLite3 REQUIRED)

add_executable(searchengine
    src/main.cpp
    src/Application.cpp
    src/Database.cpp
    src/Extractor.cpp
    src/Scanner.cpp
    src/SearchEngine.cpp
    src/Repository.cpp
    src/Indexer.cpp
)

target_include_directories(searchengine PRIVATE include)
target_link_libraries(searchengine PRIVATE SQLite::SQLite3)

enable_testing()
add_subdirectory(tests)