#include <iostream>
#include "Ailurus/String.hpp"
#include "Ailurus/Windows/Console.hpp"

int main()
{
    // Do Not Change Global Locale
    // std::locale::global(std::locale("zh_CN"));

    // Change console output code page to UTF-8
    Infra::Console::SetConsoleOutputUtf8();

    // Change wcout locale to zh_CN
    std::wcout.imbue(std::locale("zh_CN"));

    std::string str = "测试";
    std::cout << str << std::endl;
    std::wstring wstr = Infra::String::StringToWideString(str);
    std::wcout << wstr << std::endl;
    std::string str2 = Infra::String::WideStringToString(wstr);
    std::cout << str2 << std::endl;

    system("pause");

    return 0;
}