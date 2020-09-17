#include <iostream>
#include <fstream>
#include <cstring>

#include "TPatricia.hpp"

TVector<unsigned char> strToVec(const char* str) {
    TVector<unsigned char> result;
    for (int i = 0; str[i] != '\0'; ++i) {
        result.PushBack(std::tolower(str[i]));
    }
    return result;
}


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    TPatricia<unsigned long long> ptree;
    char command[257];
    while (std::cin >> command) {
        if (command[0] == '+') {
            std::cin >> command;
            TVector<unsigned char> str = strToVec(command);
            unsigned long long data;
            std::cin >> data;
            if (ptree.Insert(std::move(str), data)) {
                std::cout << "OK\n";
            } else {
                std::cout << "Exist\n";
            }
        } else if (command[0] == '-') {
            std::cin >> command;
            TVector<unsigned char> str = strToVec(command);
            if (ptree.Erase(str)) {
                std::cout << "OK\n";
            } else {
                std::cout << "NoSuchWord\n";
            }
        } else if (command[0] == '!') {
            std::cin >> command;
            if (strcmp(command,"Save") == 0) {
                std::cin >> command;
                ptree.PrintToFile(command);
                std::cout << "OK" << "\n";
            } else if (strcmp(command,"Load") == 0) {
                std::cin >> command;
                ptree.ScanFromFile(command);
                std::cout << "OK" << "\n";
            } else if (strcmp(command, "Print") == 0) {
                ptree.Print(std::cout);
            }
        } else {
            TVector<unsigned char> str = strToVec(command);
            TErr<unsigned long long> opt = ptree[str];
            if (opt) {
                std::cout << "OK: " << *opt << "\n";
            } else {
                std::cout << "NoSuchWord\n";
            }
        }
    }
    return 0;
}