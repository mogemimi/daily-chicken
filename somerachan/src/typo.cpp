// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "typo.h"
#include "editdistance.h"
#include "spellcheck.h"
#include "worddiff.h"
#include "daily/StringHelper.h"
#include <cassert>

namespace somera {
namespace {

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

bool exists(std::map<std::string, Typo>& typos, const std::string& word)
{
    return typos.find(word) != std::end(typos);
}

void addTypoSource(
    std::map<std::string, Typo>& typos,
    const std::string& word,
    TypoSource && source)
{
    auto iter = typos.find(word);
    assert(iter != std::end(typos));
    iter->second.sources.push_back(std::move(source));
}

void addTypo(std::map<std::string, Typo>& typos, Typo && typo)
{
    auto word = typo.typo;
    typos.emplace(word, std::move(typo));
}

template <class Container, typename Func>
void eraseIf(Container & container, Func func)
{
    container.erase(std::remove_if(
        std::begin(container), std::end(container), func),
        std::end(container));
}

} // unnamed namespace

TypoMan::TypoMan() noexcept
    : minimumWordSize(3)
    , isStrictWhiteSpace(true)
    , isStrictHyphen(true)
    , isStrictLetterCase(true)
{
    std::set<std::string> dictionary = {
        "jpg",
        "png",
        "mp3",
        "ogg",
        "json",
        "impl",
        "//!", // doxygen
        "NONINFRINGEMENT", // for MIT License
        "overrided" // => overriden
    };
    segmenter.setDictionary(dictionary);
}

void TypoMan::computeFromSentence(
    const std::string& sentence, const TypoSource& sourceIn)
{
    segmenter.parse(sentence, [&](const somera::PartOfSpeech& pos)
    {
        const auto word = pos.text;
        if (pos.tag != somera::PartOfSpeechTag::Raw &&
            pos.tag != somera::PartOfSpeechTag::EnglishWord) {
            return;
        }
        if (exists(typos, word)) {
            TypoSource source = sourceIn;
            addTypoSource(typos, word, std::move(source));
            return;
        }
        computeFromWord(word);
    });
}

void TypoMan::computeFromIdentifier(const std::string& identifier)
{
    auto words = somera::IdentifierWordSegmenter::parse(identifier);
    for (auto & word : words) {
        computeFromWord(word);
    }
}

void TypoMan::computeFromWord(const std::string& word)
{
    if (word.empty()) {
        return;
    }
    if (static_cast<int>(word.size()) < minimumWordSize) {
        return;
    }
    if (exists(typos, word)) {
        return;
    }
    auto corrections = correctWord(word);
    if (corrections.empty()) {
        return;
    }

    if (!isStrictWhiteSpace) {
        eraseIf(corrections, [&](const std::string& correction) {
            auto hunks = somera::computeDiff(word, correction);
            for (auto & hunk : hunks) {
                if (hunk.operation == DiffOperation::Equality) {
                    continue;
                }
                if (hunk.operation == DiffOperation::Insertion
                    && hunk.text == " ") {
                    continue;
                }
                return false;
            }
            return true;
        });
    }
    if (!isStrictHyphen) {
        eraseIf(corrections, [&](const std::string& correction) {
            auto hunks = somera::computeDiff(word, correction);
            for (auto & hunk : hunks) {
                if (hunk.operation == DiffOperation::Equality) {
                    continue;
                }
                if (hunk.operation == DiffOperation::Insertion
                    && hunk.text == "-") {
                    continue;
                }
                return false;
            }
            return true;
        });
    }
    if (!isStrictLetterCase) {
        eraseIf(corrections, [&](const std::string& correction) {
            return StringHelper::toLower(word) == StringHelper::toLower(correction);
        });
    }

    Typo typo;
    typo.typo = word;
    typo.corrections = std::move(corrections);
    if (onFoundTypo && !typo.corrections.empty()) {
        onFoundTypo(typo);
    }
    addTypo(typos, std::move(typo));
}

void TypoMan::setMinimumWordSize(int wordSize)
{
    this->minimumWordSize = std::min(wordSize, 0);
}

void TypoMan::setStrictWhiteSpace(bool strictWhiteSpace)
{
    this->isStrictWhiteSpace = strictWhiteSpace;
}

void TypoMan::setStrictHyphen(bool strictHyphen)
{
    this->isStrictHyphen = strictHyphen;
}

void TypoMan::setStrictLetterCase(bool strictLetterCase)
{
    this->isStrictLetterCase = strictLetterCase;
}

void TypoMan::setFoundCallback(std::function<void(const Typo&)> callback)
{
    assert(callback);
    onFoundTypo = callback;
}

} // namespace somera
