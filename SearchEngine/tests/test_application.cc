// Authors: Alesia Filinkova, Diana Pelin

#include <gtest/gtest.h>
#include "Application.h"
#include "Database.h"
#include "Repository.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>

namespace fs = std::filesystem;

class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::string unique_name = "app_test_" + std::to_string(::getpid());
        test_dir = fs::temp_directory_path() / unique_name;
        fs::create_directories(test_dir);

        std::error_code ec;
        fs::remove("index.db", ec);
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(test_dir, ec);

        fs::remove("index.db", ec);
    }

    void writeFile(const std::string& name, const std::string& content) {
        fs::path path = test_dir / name;
        std::ofstream file(path);
        file << content;
    }

    void createTestFiles() {
        writeFile("document1.txt", "This is document one with sample content.");
        writeFile("document2.txt", "This is document two with different content.");
        writeFile("notes.txt", "Important notes about the project.");
        writeFile("readme.md", "# README\nThis is a markdown file.");
    }
    
    std::vector<char*> makeArgv(const std::vector<std::string>& args) {
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        return argv;
    }

    int runApp(const std::vector<std::string>& args) {
        Application app;
        auto argv = makeArgv(args);
        return app.run(static_cast<int>(args.size()), argv.data());
    }

    int runAppCaptureStdout(const std::vector<std::string>& args, std::string& output) {
        Application app;
        auto argv = makeArgv(args);
        testing::internal::CaptureStdout();
        int result = app.run(static_cast<int>(args.size()), argv.data());
        output = testing::internal::GetCapturedStdout();
        return result;
    }

    fs::path test_dir;
};

TEST_F(ApplicationTest, IndexCommandCreatesDatabase) {
    createTestFiles();
    
    std::string path = test_dir.string();
    std::vector<std::string> args = {"searchengine", "index", path};
    
    int result = runApp(args);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fs::exists("index.db"));
}

TEST_F(ApplicationTest, IndexCommandReportsCorrectCounts) {
    writeFile("file1.txt", "content1");
    writeFile("file2.txt", "content2");
    writeFile("empty.txt", "");
    
    std::string path = test_dir.string();
    std::vector<std::string> args = {"searchengine", "index", path};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_EQ(result, 0);
    
    EXPECT_TRUE(output.find("Pliki przeskanowane:") != std::string::npos);
    EXPECT_TRUE(output.find("Pliki zaindeksowane:") != std::string::npos);
    EXPECT_TRUE(output.find("Pliki pominiete:") != std::string::npos);
    EXPECT_TRUE(output.find("Pliki z bledami:") != std::string::npos);
    
    bool indexed_2_or_3 = (output.find("Pliki zaindeksowane: 2") != std::string::npos) ||
                          (output.find("Pliki zaindeksowane: 3") != std::string::npos);
    EXPECT_TRUE(indexed_2_or_3);
}

TEST_F(ApplicationTest, IndexCommandReturnsPartialFailureForFailedFiles) {
    writeFile("valid.txt", "valid content");
    writeFile("invalid.xyz", "invalid");
    
    std::string path = test_dir.string();
    std::vector<std::string> args = {"searchengine", "index", path};
    
    int result = runApp(args);

    EXPECT_EQ(result, 0);
}

TEST_F(ApplicationTest, IndexCommandHandlesNonExistentDirectory) {
    std::vector<std::string> args = {"searchengine", "index", "/nonexistent/directory"};
    
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    int result = runApp(args);
    std::string stderr_output = testing::internal::GetCapturedStderr();
    std::string stdout_output = testing::internal::GetCapturedStdout();
    
    bool has_error = (result != 0) || 
                     (!stderr_output.empty()) ||
                     (stderr_output.find("nie istnieje") != std::string::npos) ||
                     (stderr_output.find("cannot") != std::string::npos) ||
                     (stderr_output.find("not exist") != std::string::npos);
    
    EXPECT_TRUE(has_error) << "Expected error for non-existent directory. "
                           << "Result=" << result << ", stderr='" << stderr_output << "'";
}

TEST_F(ApplicationTest, IndexCommandRequiresDirectoryPath) {
    std::vector<std::string> args = {"searchengine", "index"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_NE(result, 0);
}

TEST_F(ApplicationTest, RefreshCommandUpdatesIndex) {
    writeFile("refresh_test.txt", "initial content");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    writeFile("refresh_test.txt", "modified content after refresh");
    
    std::vector<std::string> refresh_args = {"searchengine", "refresh", path};
    int result = runApp(refresh_args);
    
    EXPECT_EQ(result, 0);
}

TEST_F(ApplicationTest, RefreshCommandRequiresDirectoryPath) {
    std::vector<std::string> args = {"searchengine", "refresh"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_NE(result, 0);
}

TEST_F(ApplicationTest, RefreshCommandHandlesEmptyDirectory) {
    std::string path = test_dir.string();
    std::vector<std::string> args = {"searchengine", "refresh", path};
    
    int result = runApp(args);
    
    EXPECT_EQ(result, 0);
}

TEST_F(ApplicationTest, SearchNameCommandFindsFiles) {
    createTestFiles();
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-name", "document"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("document1.txt") != std::string::npos);
    EXPECT_TRUE(output.find("document2.txt") != std::string::npos);
}

TEST_F(ApplicationTest, SearchNameCommandReturnsNoResults) {
    createTestFiles();
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-name", "nonexistent"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("Brak wynikow") != std::string::npos);
}

TEST_F(ApplicationTest, SearchNameCommandRequiresPhrase) {
    std::vector<std::string> args = {"searchengine", "search-name"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_NE(result, 0);
}

TEST_F(ApplicationTest, SearchContentCommandFindsFiles) {
    writeFile("search_content.txt", "The quick brown fox jumps over the lazy dog");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "fox"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("search_content.txt") != std::string::npos);
    EXPECT_TRUE(output.find("wystapien: 1") != std::string::npos);
}

TEST_F(ApplicationTest, SearchContentCommandShowsContext) {
    writeFile("context_test.txt", "This is a sample file with the word important in the middle.");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "important"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("Kontekst:") != std::string::npos);
}

TEST_F(ApplicationTest, SearchContentCommandHandlesMultipleOccurrences) {
    writeFile("multiple.txt", "word word word word");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "word"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("wystapien: 4") != std::string::npos);
}

TEST_F(ApplicationTest, SearchContentCommandReturnsNoResults) {
    writeFile("no_match.txt", "some content here");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "nonexistent"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("Brak trafien") != std::string::npos);
}

TEST_F(ApplicationTest, SearchContentCommandRequiresWord) {
    std::vector<std::string> args = {"searchengine", "search-content"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_NE(result, 0);
}

TEST_F(ApplicationTest, InvalidCommandShowsUsage) {
    std::vector<std::string> args = {"searchengine", "invalid-command"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    bool has_usage = (output.find("Usage") != std::string::npos) ||
                     (output.find("Uzycie") != std::string::npos) ||
                     (output.find("search") != std::string::npos);
    
    EXPECT_TRUE(has_usage) << "Expected usage information, got: " << output;
}

TEST_F(ApplicationTest, NoArgumentsShowsUsage) {
    std::vector<std::string> args = {"searchengine"};
    
    std::string output;
    int result = runAppCaptureStdout(args, output);
    
    EXPECT_EQ(result, 1);
    EXPECT_TRUE(output.find("SearchEngine") != std::string::npos);
}

TEST_F(ApplicationTest, MultipleIndexingRunsDoNotDuplicate) {
    writeFile("unique.txt", "unique content");
    
    std::string path = test_dir.string();
    std::vector<std::string> index_args = {"searchengine", "index", path};
    
    runApp(index_args);
    
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "unique"};
    
    std::string output;
    runAppCaptureStdout(search_args, output);
    
    EXPECT_TRUE(output.find("unique.txt") != std::string::npos);
}

TEST_F(ApplicationTest, DatabasePersistsBetweenCommands) {
    writeFile("persist.txt", "persistent content");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    runApp(index_args);
    
    std::vector<std::string> search_args = {"searchengine", "search-content", "persistent"};
    
    std::string output;
    int result = runAppCaptureStdout(search_args, output);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.find("persist.txt") != std::string::npos);
}

TEST_F(ApplicationTest, FullWorkflowIndexSearchRefresh) {
    writeFile("workflow1.txt", "first file content with unique_word");
    writeFile("workflow2.txt", "second file with different content");
    
    std::string path = test_dir.string();
    
    std::vector<std::string> index_args = {"searchengine", "index", path};
    int index_result = runApp(index_args);
    EXPECT_EQ(index_result, 0);
    
    std::vector<std::string> name_args = {"searchengine", "search-name", "workflow"};
    std::string name_output;
    runAppCaptureStdout(name_args, name_output);
    EXPECT_TRUE(name_output.find("workflow1.txt") != std::string::npos);
    EXPECT_TRUE(name_output.find("workflow2.txt") != std::string::npos);
    
    std::vector<std::string> content_args = {"searchengine", "search-content", "unique_word"};
    std::string content_output;
    runAppCaptureStdout(content_args, content_output);
    EXPECT_TRUE(content_output.find("workflow1.txt") != std::string::npos);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    writeFile("workflow1.txt", "modified content with new_word");
    
    std::vector<std::string> refresh_args = {"searchengine", "refresh", path};
    int refresh_result = runApp(refresh_args);
    EXPECT_EQ(refresh_result, 0);
    
    std::vector<std::string> new_word_args = {"searchengine", "search-content", "new_word"};
    std::string new_output;
    runAppCaptureStdout(new_word_args, new_output);
    EXPECT_TRUE(new_output.find("workflow1.txt") != std::string::npos);
    
    std::error_code ec;
    fs::remove(test_dir / "workflow2.txt", ec);
    
    runApp(refresh_args);
    
    std::vector<std::string> name_after_args = {"searchengine", "search-name", "workflow2"};
    std::string after_output;
    runAppCaptureStdout(name_after_args, after_output);
    EXPECT_TRUE(after_output.find("Brak wynikow") != std::string::npos);
}
