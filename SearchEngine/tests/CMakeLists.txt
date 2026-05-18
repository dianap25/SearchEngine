find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(unit_tests
    test_application.cc
    test_database.cc
    test_extractor.cc
    test_scanner.cc
    test_repository.cc
    test_indexer.cc
    test_index_service.cc
    test_context_builder.cc
    test_hasher.cc
    test_refresh_engine.cc
    test_search_result.cc

    ../src/IndexService.cc
    ../src/Database.cc
    ../src/TextExtractor.cc
    ../src/PdfExtractor.cc
    ../src/ExtractorFactory.cc
    ../src/Scanner.cc
    ../src/Repository.cc
    ../src/Indexer.cc
    ../src/ContextBuilder.cc
    ../src/Hasher.cc
    ../src/RefreshEngine.cc
    ../src/Application.cc
)

target_include_directories(unit_tests PRIVATE ../include)
target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main SQLite::SQLite3 OpenSSL::SSL OpenSSL::Crypto)

include(GoogleTest)
gtest_discover_tests(unit_tests)

target_compile_definitions(unit_tests PRIVATE
    TEST_SAMPLE_DATA_DIR="${CMAKE_SOURCE_DIR}/sample_data"
)
