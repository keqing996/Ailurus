#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

#if AILURUS_PLATFORM_MAC
#include <libproc.h>
#endif

#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Ailurus/System/Process.h"

namespace Ailurus
{
    static std::optional<size_t> ReadPipe(int pipeFd, char* buffer, size_t maxSize)
    {
        ssize_t count = ::read(pipeFd, buffer, maxSize);
        if (count > 0)
            return count;

        // EOF
        if (count == 0)
            return 0;

        // No data
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;

        return std::nullopt;
    }

    bool Process::WriteStdin(const char* buffer, size_t size) const
    {
        if (::write(static_cast<int>(stdinPipe), buffer, size) == -1)
            return false;

        return true;
    }

    bool Process::IsRunning() const
    {
        pid_t pid = static_cast<pid_t>(handle);
        if (pid < 0)
            return false;

        int status;
        // Check progress status by non-block WNOHANG
        int result = ::waitpid(pid, &status, WNOHANG);
        if (result == 0)
            return true;

        return false;
    }

    int Process::WaitFinish() const
    {
        int status;
        ::waitpid(static_cast<pid_t>(handle), &status, 0);
        return WEXITSTATUS(status);
    }

    std::optional<size_t> Process::ReadStdout(char* buffer, size_t maxSize) const
    {
        return ReadPipe(static_cast<int>(stdoutPipe), buffer, maxSize);
    }

    std::optional<size_t> Process::ReadStderr(char* buffer, size_t maxSize) const
    {
        return ReadPipe(static_cast<int>(stderrPipe), buffer, maxSize);
    }

    int32_t Process::GetCurrentProcessId()
    {
        return ::getpid();
    }

    std::string Process::GetProcessName(int32_t pid)
    {
#if AILURUS_PLATFORM_MAC
        char name[PROC_PIDPATHINFO_MAXSIZE] = {0};
        if (::proc_name(pid, name, sizeof(name)) <= 0)
            return "";

        return std::string(name);
#else
        std::string proc_status_path = "/proc/" + std::to_string(pid) + "/status";
        std::ifstream status_file(proc_status_path);

        // PID not exist, or it is no auth to access it.
        if (!status_file.is_open())
            return "";

        std::string line;
        while (std::getline(status_file, line))
        {
            if (line.find("Name:") == 0)
            {
                std::istringstream iss(line);
                std::string key, value;
                iss >> key >> value;
                return value;
            }
        }

        return "";
#endif
    }

    std::optional<Process> Process::Create(const std::string& exeName, const std::vector<std::string>& argv)
    {
        if (exeName.empty())
            return std::nullopt;

        int stdinPipe[2];
        int stdoutPipe[2];
        int stderrPipe[2];
        if (::pipe(stdinPipe) == -1)
            return std::nullopt;

        if (::pipe(stdoutPipe) == -1)
        {
            ::close(stdinPipe[0]);
            ::close(stdinPipe[1]);
            return std::nullopt;
        }

        if (::pipe(stderrPipe) == -1)
        {
            ::close(stdinPipe[0]);
            ::close(stdinPipe[1]);
            ::close(stdoutPipe[0]);
            ::close(stdoutPipe[1]);
            return std::nullopt;
        }

        pid_t pid = ::fork();
        if (pid == -1)
        {
            ::close(stdinPipe[0]);
            ::close(stdinPipe[1]);
            ::close(stdoutPipe[0]);
            ::close(stdoutPipe[1]);
            ::close(stderrPipe[0]);
            ::close(stderrPipe[1]);
            return std::nullopt;
        }

        if (pid == 0) // Child process
        {
            // redirect std in/out/err
            if (::dup2(stdinPipe[0], STDIN_FILENO) == -1 ||
                ::dup2(stdoutPipe[1], STDOUT_FILENO) == -1 ||
                ::dup2(stderrPipe[1], STDERR_FILENO) == -1)
            {
                ::exit(EXIT_FAILURE);
            }

            // close all parent pipe
            ::close(stdinPipe[0]);
            ::close(stdinPipe[1]);
            ::close(stdoutPipe[0]);
            ::close(stdoutPipe[1]);
            ::close(stderrPipe[0]);
            ::close(stderrPipe[1]);

            // prepare parameters
            std::vector<char*> argvRaw;
            argvRaw.push_back(const_cast<char*>(exeName.c_str()));
            for (const auto& arg : argv)
                argvRaw.push_back(const_cast<char*>(arg.c_str()));
            argvRaw.push_back(nullptr);

            ::execvp(exeName.c_str(), argvRaw.data());
            exit(EXIT_FAILURE);
        }

        // Parent process
        ::close(stdinPipe[0]);   // close stdin read
        ::close(stdoutPipe[1]);  // close stdout write
        ::close(stderrPipe[1]);  // close stderr write

        // Set non-block read
        ::fcntl(stdoutPipe[0], F_SETFL, O_NONBLOCK);
        ::fcntl(stderrPipe[0], F_SETFL, O_NONBLOCK);

        Process result {};
        result.handle = static_cast<int64_t>(pid);
        result.stdinPipe = stdinPipe[1];
        result.stdoutPipe = stdoutPipe[0];
        result.stderrPipe = stderrPipe[0];

        return result;
    }
}

#endif
