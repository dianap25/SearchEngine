// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of PdfExtractor. Calls pdftotext as a
// child process via fork + execvp; the file path is placed in argv
// directly so it never reaches a shell.
//
// No shell -- argv passed directly to pdftotext to avoid command
// injection.

#include "PdfExtractor.h"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kReadEnd = 0;
constexpr int kWriteEnd = 1;
constexpr std::size_t kBufferSize = 4096;

ExtractResult readChildStdout(int fd) {
    std::string content;
    char buffer[kBufferSize];

    while (true) {
        ssize_t bytes = ::read(fd, buffer, sizeof(buffer));
        if (bytes > 0) {
            content.append(buffer, static_cast<std::size_t>(bytes));
            continue;
        }
        if (bytes == 0) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        return ExtractResult::fail(
            std::string("read from pdftotext failed: ") + std::strerror(errno));
    }

    if (content.empty()) {
        return ExtractResult::fail("PDF extraction returned empty content");
    }

    return ExtractResult::ok(std::move(content));
}

} // namespace

ExtractResult PdfExtractor::extract(const std::string& file_path) const {
    if (!fs::exists(file_path)) {
        return ExtractResult::fail("File does not exist");
    }

    int pipe_fds[2];
    if (::pipe(pipe_fds) != 0) {
        return ExtractResult::fail(
            std::string("pipe() failed: ") + std::strerror(errno));
    }

    pid_t pid = ::fork();
    if (pid < 0) {
        ::close(pipe_fds[kReadEnd]);
        ::close(pipe_fds[kWriteEnd]);
        return ExtractResult::fail(
            std::string("fork() failed: ") + std::strerror(errno));
    }

    if (pid == 0) {
        // Child: redirect stdout to the pipe and exec pdftotext.
        ::close(pipe_fds[kReadEnd]);
        if (::dup2(pipe_fds[kWriteEnd], STDOUT_FILENO) < 0) {
            ::_exit(127);
        }
        ::close(pipe_fds[kWriteEnd]);

        // pdftotext <file_path> -   (write text to stdout)
        std::string mutable_path = file_path;
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>("pdftotext"));
        argv.push_back(mutable_path.data());
        argv.push_back(const_cast<char*>("-"));
        argv.push_back(nullptr);

        ::execvp("pdftotext", argv.data());
        ::_exit(127);
    }

    // Parent: read child's stdout, then wait.
    ::close(pipe_fds[kWriteEnd]);
    ExtractResult result = readChildStdout(pipe_fds[kReadEnd]);
    ::close(pipe_fds[kReadEnd]);

    int status = 0;
    while (::waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return ExtractResult::fail(
                std::string("waitpid() failed: ") + std::strerror(errno));
        }
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code != 0) {
            return ExtractResult::fail(
                "pdftotext exited with status " + std::to_string(code));
        }
    } else if (WIFSIGNALED(status)) {
        return ExtractResult::fail(
            "pdftotext killed by signal " + std::to_string(WTERMSIG(status)));
    }

    return result;
}
