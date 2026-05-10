cmake_minimum_required(VERSION 3.16)
project(SearchEngineSkeleton VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(searchengine
    src/main.cc
    src/Application.cc
    src/Database.cc
    src/Scanner.cc
    src/TextExtractor.cc
    src/PdfExtractor.cc
    src/ExtractorFactory.cc
    src/Repository.cc
    src/Indexer.cc
    src/IndexService.cc
    src/SearchEngine.cc
    src/RefreshEngine.cc
    src/Hasher.cc
    src/ContextBuilder.cc
)

target_include_directories(searchengine PRIVATE include)
target_link_libraries(searchengine PRIVATE SQLite::SQLite3)
target_link_libraries(searchengine PRIVATE OpenSSL::SSL OpenSSL::Crypto)

enable_testing()
add_subdirectory(tests)
