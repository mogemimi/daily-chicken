#include "editdistance.h"
#include "spellcheck.h"
#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <cassert>
#include <vector>
#include <fstream>

namespace {

#define UNUSED_CODE 0
#if UNUSED_CODE

void tryMatch(const std::string& a, const std::string& b)
{
    std::printf("%9s %9s", a.c_str(), b.c_str());
    std::printf(" | %2d", somera::EditDistance::levenshteinDistance(a, b));
    std::printf(" | %1.4f", somera::EditDistance::closestMatchFuzzyDistance(a, b));
    std::printf(" | %1.4f", somera::EditDistance::jaroWinklerDistance(a, b));
    std::cout << std::endl;
}

void checkSpell(const std::string& word)
{
    auto correction = somera::NativeSpellChecker::correctSpelling(word);
    auto result = somera::NativeSpellChecker::findClosestWords(word);

    std::cout << word << " => ";

    if (correction.empty()) {
        std::cout << "(is not misspelling)";
    }
    else {
        std::cout << correction;
    }

    {
        std::cout << " (";
        bool spacing = false;
        for (auto & s : result) {
            if (spacing) {
                std::cout << " ";
            }
            std::cout << s;
            spacing = true;
        }
        std::cout << ")" << std::endl;
    }
}

void test()
{
    std::printf("%9s %9s | Ls | fast-M | Jw \n", "s1", "s2");

    tryMatch("LISP", "LISP");
    tryMatch("LISP", "LISp");
    tryMatch("LISP", "LIsp");
    tryMatch("LISP", "Lisp");
    tryMatch("LISP", "lisp");
    tryMatch("LISP", "lis");
    tryMatch("LISP", "li");
    tryMatch("LISP", "l");
    tryMatch("LISP", "");
    tryMatch("abcd", "abcd");
    tryMatch("abcd", "abcD");
    tryMatch("abcd", "Abcd");
    tryMatch("abc", "abc");
    tryMatch("abc", "Abc");
    tryMatch("abc", "aBc");
    tryMatch("abc", "abC");
    tryMatch("abc", "ABc");
    tryMatch("abc", "aBC");
    tryMatch("abc", "AbC");
    tryMatch("abc", "acb");
    tryMatch("abc", "bac");
    tryMatch("abc", "bca");
    tryMatch("abc", "acB");
    tryMatch("A", "A");
    tryMatch("A", "a");
    tryMatch("A", "B");
    tryMatch("Google", "google");
    tryMatch("Typo", "Type");
    tryMatch("Hoge", "Foo");
    tryMatch("unkown", "unknown");

    std::cout << "------" << std::endl;

    checkSpell("unknown");
    checkSpell("unkown");
    checkSpell("exisiting");
    checkSpell("volid");
    tryMatch("volid", "solid");
    tryMatch("volid", "valid");
    tryMatch("volid", "void");
    tryMatch("volid", "voile");
    tryMatch("volid", "vilid");
    tryMatch("volid", "vllid");
    checkSpell("charater");
    tryMatch("charater", "charter");
    tryMatch("charater", "character");
}

#endif

//bool isCompilerKeyword()
//{
//    std::set<std::string> set = {
//        "int",
//        "long",
//        "unsigned"
//        "bool",
//        "char",
//        "switch",
//        "return",
//        "if"
//        "true",
//        "false",
//        "auto",
//        "static",
//        "void",
//        "namespace",
//        "const",
//        "constexpr",
//    };
//}

} // unnamed namespace

int main(int argc, char *argv[])
{

    return 0;
}
