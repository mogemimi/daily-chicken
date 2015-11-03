// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "wordsegmenter.h"
#include <cassert>
#include <codecvt>
#include <cstdint>
#include <locale>
#include <vector>
#include <regex>
#include <unordered_set>

namespace somera {
namespace {

std::u32string toUtf32(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.from_bytes(s);
}

std::string toUtf8(const std::u32string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.to_bytes(s);
}

template <typename F>
std::vector<std::u32string> splitWords(const std::u32string& text, F isSeparator)
{
    std::vector<std::u32string> words;
    std::u32string wordBuffer;

    for (const auto & c : text) {
        if (isSeparator(c)) {
            // skip space
            if (!wordBuffer.empty()) {
                words.push_back(std::move(wordBuffer));
                wordBuffer.clear();
            }
            continue;
        }
        wordBuffer += c;
    }

    if (!wordBuffer.empty()) {
        words.push_back(std::move(wordBuffer));
        wordBuffer.clear();
    }

    return std::move(words);
}

bool isCompilerKeyword(const std::string& word)
{
    static std::unordered_set<std::string> keywords = {
        "alignas",
        "alignof",
        "and",
        "and_eq",
        "asm",
        "auto",
        "bitand",
        "bitor",
        "bool",
        "break",
        "case",
        "catch",
        "char",
        "char16_t",
        "char32_t",
        "class",
        "compl",
        "concept",
        "const",
        "constexpr",
        "const_cast",
        "continue",
        "decltype",
        "default",
        "delete",
        "do",
        "double",
        "dynamic_cast",
        "else",
        "enum",
        "explicit",
        "export",
        "extern",
        "false",
        "float",
        "for",
        "friend",
        "goto",
        "if",
        "inline",
        "int",
        "long",
        "mutable",
        "namespace",
        "new",
        "noexcept",
        "not",
        "not_eq",
        "nullptr",
        "operator",
        "or",
        "or_eq",
        "private",
        "protected",
        "public",
        "register",
        "reinterpret_cast",
        "requires",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "static_assert",
        "static_cast",
        "struct",
        "switch",
        "template",
        "this",
        "thread_local",
        "throw",
        "true",
        "try",
        "typedef",
        "typeid",
        "typename",
        "union",
        "unsigned",
        "using",
        "virtual",
        "void",
        "volatile",
        "wchar_t",
        "while",
        "xor",
        "xor_eq"
    };

    return keywords.find(word) != std::end(keywords);
}

PartOfSpeechTag findPartOfSpeechTag(const std::string& text)
{
    {
        if (isCompilerKeyword(text)) {
            return PartOfSpeechTag::CplusplusKeywords;
        }
    }
    {
        const std::regex re(R"([A-Z]*[a-z]+)");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::EnglishWord;
        }
    }
    {
        const std::regex re(R"(\d+)");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::Integer;
        }
    }
    {
        const std::regex re(R"((\d*\.\d+|\d+\.\d*)f?)");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::FloatNumber;
        }
    }
    {
        const std::regex re(R"(git@\w+\..+)");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::GitUrl;
        }
    }
    {
        const std::regex re(R"((http|https|ftp)://.+)");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::Url;
        }
    }
    {
        const std::regex re(R"((\@|\\)((f(\$|\[|\]|\{|\}))|(\$|\@|\\|\&|\~|\<|\>|\#|\%)|([a-z]{1,16})))");
        if (std::regex_match(text, re)) {
            return PartOfSpeechTag::DoxygenKeywords;
        }
    }
    return PartOfSpeechTag::Raw;
}

} // unnamed namespace

void WordSegmenter::setDictionary(const std::set<std::string>& dictionaryIn)
{
    for (auto & w : dictionaryIn) {
        dictionary.insert(w);
    }
}

void WordSegmenter::parse(
    const std::string& utf8Text,
    std::function<void(const PartOfSpeech&)> readWord)
{
    parse(utf8Text, readWord, true);
}

void WordSegmenter::parse(
    const std::string& utf8Text,
    std::function<void(const PartOfSpeech&)> readWord,
    bool recursive)
{
    const auto text = toUtf32(utf8Text);
    const auto words = splitWords(text, recursive
        ? [](int c)->bool{ return ::isspace(c); }
        : [](int c)->bool{ return !::isalnum(c); });

    std::vector<std::u32string> candidates;

    for (const auto& word : words) {
        PartOfSpeech pos;
        pos.text = toUtf8(word);
        pos.tag = PartOfSpeechTag::Raw;

        do {
            {
                auto iter = dictionary.find(pos.text);
                if (iter != std::end(dictionary)) {
                    pos.tag = PartOfSpeechTag::UserDefined;
                    break;
                }
            }
            {
                pos.tag = findPartOfSpeechTag(pos.text);
                if (pos.tag != PartOfSpeechTag::Raw) {
                    break;
                }
            }
        } while (0);

        if (pos.tag != PartOfSpeechTag::Raw) {
            readWord(pos);
        }
        else if (recursive) {
            assert(pos.tag == PartOfSpeechTag::Raw);
            parse(pos.text, readWord, false);
        }
        else {
            assert(pos.tag == PartOfSpeechTag::Raw);
            readWord(pos);
        }

    }
}

} // namespace somera
