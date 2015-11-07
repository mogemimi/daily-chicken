#include "editdistance.h"
#include <gtest/iutest_switch.hpp>
#include <vector>

using somera::EditDistance;
using namespace somera;

TEST(EditDistance, ClosestMatchFuzzyDistance)
{
    const auto closestMatch = EditDistance::closestMatchFuzzyDistance;

    EXPECT_EQ(1.0, closestMatch("LISP", "LISP"));
    EXPECT_GT(1.0, closestMatch("LISP", "LISp"));
    EXPECT_LT(0.9, closestMatch("LISP", "LISp"));

    EXPECT_EQ(1.0, closestMatch(u8"不思議なソメラちゃん", u8"不思議なソメラちゃん"));
    EXPECT_GT(0.72, closestMatch(u8"不思議なソメラちゃん", u8"不思議なLISPソメラちゃん"));
    EXPECT_LT(0.71, closestMatch(u8"不思議なソメラちゃん", u8"不思議なLISPソメラちゃん"));

    EXPECT_EQ(1.0, closestMatch(
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93",
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93"));
    EXPECT_GT(0.72, closestMatch(
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93",
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\x4c\x49\x53\x50\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93"));
    EXPECT_LT(0.71, closestMatch(
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93",
        u8"\xe4\xb8\x8d\xe6\x80\x9d\xe8\xad\xb0\xe3\x81\xaa\x4c\x49\x53\x50\xe3\x82\xbd\xe3\x83\xa1\xe3\x83\xa9\xe3\x81\xa1\xe3\x82\x83\xe3\x82\x93"));
}

TEST(EditDistance, JaroWinklerDistance)
{
    const auto jaroWinkler = EditDistance::jaroWinklerDistance;

    EXPECT_EQ(1.0, jaroWinkler("LISP", "LISP"));
    EXPECT_GT(1.0, jaroWinkler("LISP", "LIS"));
    EXPECT_LT(0.8, jaroWinkler("LISP", "LIS"));
}

TEST(EditDistance, LevenshteinDistance)
{
    const auto levenshtein = EditDistance::levenshteinDistance;

    EXPECT_EQ(0, levenshtein("LISP", "LISP"));
    EXPECT_EQ(2, levenshtein("LISP", "LISp"));
    EXPECT_EQ(4, levenshtein("LISP", "LIsp"));
    EXPECT_EQ(6, levenshtein("LISP", "Lisp"));
    EXPECT_EQ(8, levenshtein("LISP", "lisp"));
    EXPECT_EQ(7, levenshtein("LISP", "lis"));
    EXPECT_EQ(6, levenshtein("LISP", "li"));
    EXPECT_EQ(5, levenshtein("LISP", "l"));
    EXPECT_EQ(4, levenshtein("LISP", ""));
    EXPECT_EQ(2, levenshtein("LISP", "LLISPP"));
    EXPECT_EQ(0, levenshtein("abcd", "abcd"));
    EXPECT_EQ(2, levenshtein("abcd", "abcD"));
    EXPECT_EQ(2, levenshtein("abcd", "Abcd"));
    EXPECT_EQ(0, levenshtein("abc", "abc"));
    EXPECT_EQ(2, levenshtein("abc", "Abc"));
    EXPECT_EQ(2, levenshtein("abc", "aBc"));
    EXPECT_EQ(2, levenshtein("abc", "abC"));
    EXPECT_EQ(4, levenshtein("abc", "ABc"));
    EXPECT_EQ(4, levenshtein("abc", "aBC"));
    EXPECT_EQ(4, levenshtein("abc", "AbC"));
    EXPECT_EQ(2, levenshtein("abc", "acb"));
    EXPECT_EQ(2, levenshtein("abc", "bac"));
    EXPECT_EQ(2, levenshtein("abc", "bca"));
    EXPECT_EQ(2, levenshtein("abc", "acB"));
    EXPECT_EQ(0, levenshtein("A", "A"));
    EXPECT_EQ(2, levenshtein("A", "a"));
    EXPECT_EQ(2, levenshtein("A", "B"));
    EXPECT_EQ(2, levenshtein("Typo", "Type"));
    EXPECT_EQ(2, levenshtein("volid", "solid"));
    EXPECT_EQ(2, levenshtein("volid", "valid"));
    EXPECT_EQ(1, levenshtein("volid", "void"));
    EXPECT_EQ(4, levenshtein("volid", "voile"));
    EXPECT_EQ(2, levenshtein("volid", "vilid"));
    EXPECT_EQ(2, levenshtein("volid", "vllid"));
    EXPECT_EQ(2, levenshtein("character", "charter"));
    EXPECT_EQ(1, levenshtein("character", "charater"));
    EXPECT_EQ(1, levenshtein("unkown", "unknown"));
}
