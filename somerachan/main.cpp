#include "editdistance.h"
#include "optional.h"
#include "spellcheck.h"
#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <cassert>
#include <vector>
#include <fstream>
#include <set>

using somera::NativeSpellChecker;
using somera::EditDistance;
using somera::Optional;
using somera::NullOpt;

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

std::vector<std::string> correctWord(const std::string& word)
{
    auto correction = somera::NativeSpellChecker::correctSpelling(word);
    if (correction.empty()) {
        return {};
    }

    auto corrections = somera::NativeSpellChecker::findClosestWords(word);
    if (corrections.empty()) {
        return {std::move(correction)};
    }

    if (corrections.size() > 4) {
        corrections.resize(4);
    }

    std::sort(std::begin(corrections), std::end(corrections), [&](auto& a, auto& b) {
        const auto fuzzyA = EditDistance::closestMatchFuzzyDistance(word, a);
        const auto fuzzyB = EditDistance::closestMatchFuzzyDistance(word, b);
        if (fuzzyA != fuzzyB) {
            return fuzzyA > fuzzyB;
        }
        const auto jwA = EditDistance::jaroWinklerDistance(word, a);
        const auto jwB = EditDistance::jaroWinklerDistance(word, b);
        if (jwA != jwB) {
            return jwA > jwB;
        }
        const auto lsA = EditDistance::levenshteinDistance(word, a);
        const auto lsB = EditDistance::levenshteinDistance(word, b);
        return lsA <= lsB;
    });

    assert(!corrections.empty());
    return std::move(corrections);
}

bool isSeparator(int c)
{
    return !isalnum(c);
}

void splitWords(const std::string& text, std::function<void(const std::string&)> func)
{
    std::string wordBuffer;
    for (auto & c : text) {
        if (isSeparator(c)) {
            if (!wordBuffer.empty()) {
                func(wordBuffer);
                wordBuffer.clear();
            }
            continue;
        }
        wordBuffer += c;
    }

    if (!wordBuffer.empty()) {
        func(wordBuffer);
        wordBuffer.clear();
    }
}

void parse(int argc, char *argv[])
{
    if (argc < 2) {
        std::printf("failed");
        return;
    }

    std::string path = argv[1];

    if (path.empty()) {
        std::printf("failed");
        return;
    }

    std::fstream stream(path);

    if (!stream) {
        std::printf("cannot open file %s", path.c_str());
        return;
    }

    std::cout << "open: " << path << std::endl;

    std::set<std::string> dictionary = {"json", "impl"};

    std::string line;
    while(std::getline(stream, line)) {
        splitWords(line, [&dictionary](const std::string& word){
            auto iter = dictionary.find(word);
            if (iter != std::end(dictionary)) {
                return;
            }
            auto corrections = correctWord(word);
            if (!corrections.empty()) {
                std::printf("%10s => %10s", word.c_str(), corrections.front().c_str());
                if (corrections.size() > 1) {
                    std::printf(" (");
                    for (size_t i = 1; i < corrections.size(); ++i) {
                        if (i > 1) {
                            std::printf(" ");
                        }
                        std::printf("%s", corrections[i].c_str());
                    }
                    std::printf(")");
                }
                std::printf("\n");
            }
        });
    }
}

} // unnamed namespace

#include <sstream>

int main(int argc, char *argv[])
{
    parse(argc, argv);
    return 0;
}
