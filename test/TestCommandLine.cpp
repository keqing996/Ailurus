#include <Ailurus/CommandLine.hpp>

int main()
{
    int argc = 13;
    char argv[][] {
        "dummy.exe",
        "--name",
        "TestName",
        "-s",
        "-i",
        "input1.txt",
        "input2.txt",
        "input3.txt",
        "input4.txt",
        "--ip",
        "192.168.0.1",
        "-p",
        "6666"
    };

    Infra::CommandLine commandLine;
    commandLine.AddOption<Infra::CmdOptionType::SingleValue>("name", 'n', "name name");
    commandLine.AddOption<Infra::CmdOptionType::NoValue>("stest", 's', "stest stest");
    commandLine.AddOption<Infra::CmdOptionType::MultiValue>("input", 'i', "input input");
    commandLine.AddOption<Infra::CmdOptionType::SingleValue>("ip", 'v', "name name");
    commandLine.AddOption<Infra::CmdOptionType::SingleValue>("port", 'p', "name name");

}