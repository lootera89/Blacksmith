#include "pickword.h"
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

const char* pickword::getword(int len) {
    static std::vector<std::string> wordlist = {
        "keyboard", "apple", "imagination", "random", "code",
        "syntax", "debug", "variable", "class", "function",
        "example", "pointer", "loop", "compile", "header"
    };
    static std::string word;
    if (word.empty()) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
    word = wordlist[std::rand() % wordlist.size()];
    return word.c_str();
}
