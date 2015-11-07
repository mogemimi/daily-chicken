#include "worddiff.h"
//#include "consolecolor.h"
#include <gtest/iutest_switch.hpp>
#include <vector>
#include <string>

namespace somera {

//void printWordDiff(const std::string& text1, const std::string& text2)
//{
//    using somera::TerminalColor;
//    auto hunks = computeDiff(text1, text2);
//    for (auto & hunk : hunks) {
//        if (hunk.operation == DiffOperation::Equality) {
//            std::printf("%s", hunk.text.c_str());
//        }
//        if (hunk.operation == DiffOperation::Deletion) {
//            std::printf("%s", somera::changeTerminalTextColor(
//                hunk.text,
//                TerminalColor::Black,
//                TerminalColor::Cyan).c_str());
//        }
//        if (hunk.operation == DiffOperation::Insertion) {
//            std::printf("%s", somera::changeTerminalTextColor(
//                hunk.text,
//                TerminalColor::White,
//                TerminalColor::Yellow).c_str());
//        }
//    }
//    std::printf("\n");
//}

} // namespace somera

using namespace somera;

TEST(WordDiff, TrivialCase)
{
    {
        auto hunks = computeDiff("L", "");
        ASSERT_EQ(1, hunks.size());
        EXPECT_EQ("L", hunks[0].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[0].operation);
    }
    {
        auto hunks = computeDiff("", "L");
        ASSERT_EQ(1, hunks.size());
        EXPECT_EQ("L", hunks[0].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[0].operation);
    }
    {
        auto hunks = computeDiff("L", "42");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("L", hunks[0].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[0].operation);
        EXPECT_EQ("42", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("42", "L");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("42", hunks[0].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[0].operation);
        EXPECT_EQ("L", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LISp");
        ASSERT_EQ(3, hunks.size());
        EXPECT_EQ("LIS", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("P", hunks[1].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[1].operation);
        EXPECT_EQ("p", hunks[2].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[2].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LIS");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("LIS", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("P", hunks[1].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LIP");
        ASSERT_EQ(3, hunks.size());
        EXPECT_EQ("LI", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("S", hunks[1].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[1].operation);
        EXPECT_EQ("P", hunks[2].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[2].operation);
    }
    {
        auto hunks = computeDiff("LISP", "RIP");
        ASSERT_EQ(5, hunks.size());
        EXPECT_EQ("L", hunks[0].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[0].operation);
        EXPECT_EQ("R", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
        EXPECT_EQ("I", hunks[2].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[2].operation);
        EXPECT_EQ("S", hunks[3].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[3].operation);
        EXPECT_EQ("P", hunks[4].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[4].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LISPER");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("LISP", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("ER", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LLISP");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("L", hunks[0].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[0].operation);
        EXPECT_EQ("LISP", hunks[1].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("LISP", "LISPP");
        ASSERT_EQ(2, hunks.size());
        EXPECT_EQ("LISP", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("P", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
    }
    {
        auto hunks = computeDiff("sunday", "saturday");
        ASSERT_EQ(6, hunks.size());
        EXPECT_EQ("s", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("at", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
        EXPECT_EQ("u", hunks[2].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[2].operation);
        EXPECT_EQ("n", hunks[3].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[3].operation);
        EXPECT_EQ("r", hunks[4].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[4].operation);
        EXPECT_EQ("day", hunks[5].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[5].operation);
    }
    {
        auto hunks = computeDiff("Linux", "Unix");
        ASSERT_EQ(6, hunks.size());
        EXPECT_EQ("Li", hunks[0].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[0].operation);
        EXPECT_EQ("U", hunks[1].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[1].operation);
        EXPECT_EQ("n", hunks[2].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[2].operation);
        EXPECT_EQ("u", hunks[3].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[3].operation);
        EXPECT_EQ("i", hunks[4].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[4].operation);
        EXPECT_EQ("x", hunks[5].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[5].operation);
    }
    {
        auto hunks = computeDiff(
            "THE SOFTWARE IS PROVIDED AS IS",
            "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS");
        ASSERT_EQ(6, hunks.size());
        EXPECT_EQ("TH", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("E", hunks[1].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[1].operation);
        EXPECT_EQ("IS", hunks[2].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[2].operation);
        EXPECT_EQ(" SOFTWARE IS PROVIDED", hunks[3].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[3].operation);
        EXPECT_EQ(" BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS", hunks[4].text);
        EXPECT_EQ(DiffOperation::Insertion, hunks[4].operation);
        EXPECT_EQ(" AS IS", hunks[5].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[5].operation);
    }
    {
        auto hunks = computeDiff("liecense", "license");
        ASSERT_EQ(3, hunks.size());
        EXPECT_EQ("li", hunks[0].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[0].operation);
        EXPECT_EQ("e", hunks[1].text);
        EXPECT_EQ(DiffOperation::Deletion, hunks[1].operation);
        EXPECT_EQ("cense", hunks[2].text);
        EXPECT_EQ(DiffOperation::Equality, hunks[2].operation);
    }
}
