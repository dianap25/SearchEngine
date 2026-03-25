find_package(GTest REQUIRED)

add_executable(unit_tests
    test_database.cpp
    test_extractor.cpp
    ../src/Database.cpp
    ../src/Extractor.cpp
)

target_include_directories(unit_tests PRIVATE ../include)
target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main SQLite::SQLite3)

include(GoogleTest)
gtest_discover_tests(unit_tests)

target_compile_definitions(unit_tests PRIVATE
    TEST_SAMPLE_DATA_DIR="${CMAKE_SOURCE_DIR}/sample_data"
)