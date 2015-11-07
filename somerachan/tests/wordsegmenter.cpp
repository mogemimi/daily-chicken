#include "wordsegmenter.h"
#include <gtest/iutest_switch.hpp>
#include <vector>

using somera::PartOfSpeechTag;
using somera::PartOfSpeech;
using somera::WordSegmenter;

TEST(WordSegmenter, Parse_FirstCase)
{
    WordSegmenter segmenter;

    std::vector<std::string> words;
    auto read = [&](const PartOfSpeech& pos) { words.push_back(pos.text); };

    segmenter.parse("Time flies like an arrow.", read);
    ASSERT_EQ(5, words.size());
    EXPECT_EQ(words[0], "Time");
    EXPECT_EQ(words[1], "flies");
    EXPECT_EQ(words[2], "like");
    EXPECT_EQ(words[3], "an");
    EXPECT_EQ(words[4], "arrow");
}

TEST(WordSegmenter, Parse_Comma)
{
    WordSegmenter segmenter;

    std::vector<std::string> words;
    auto read = [&](const PartOfSpeech& pos) { words.push_back(pos.text); };

    segmenter.parse("Time,flies like,an arrow.", read);
    ASSERT_EQ(5, words.size());
    EXPECT_EQ(words[0], "Time");
    EXPECT_EQ(words[1], "flies");
    EXPECT_EQ(words[2], "like");
    EXPECT_EQ(words[3], "an");
    EXPECT_EQ(words[4], "arrow");
}

TEST(WordSegmenter, Parse_Contractions)
{
    WordSegmenter segmenter;
    {
        std::vector<std::string> words;
        auto read = [&](const PartOfSpeech& pos) { words.push_back(pos.text); };

        segmenter.parse("Don't forget where you belong.", read);
        ASSERT_EQ(5, words.size());
        EXPECT_EQ(words[0], "Don't");
        EXPECT_EQ(words[1], "forget");
        EXPECT_EQ(words[2], "where");
        EXPECT_EQ(words[3], "you");
        EXPECT_EQ(words[4], "belong");
    }
    {
        std::vector<std::string> words;
        auto read = [&](const PartOfSpeech& pos) { words.push_back(pos.text); };

        segmenter.parse("I mustn't run away.", read);
        ASSERT_EQ(4, words.size());
        EXPECT_EQ(words[0], "I");
        EXPECT_EQ(words[1], "mustn't");
        EXPECT_EQ(words[2], "run");
        EXPECT_EQ(words[3], "away");
    }
}

TEST(WordSegmenter, Parse_Url)
{
    WordSegmenter segmenter;

    std::vector<PartOfSpeech> words;
    auto read = [&](const PartOfSpeech& pos) { words.push_back(pos); };

    segmenter.parse("Time flies like an http://github.com arrow.", read);

    ASSERT_EQ(6, words.size());
    EXPECT_EQ(words[0].text, "Time");
    EXPECT_EQ(words[1].text, "flies");
    EXPECT_EQ(words[2].text, "like");
    EXPECT_EQ(words[3].text, "an");
    EXPECT_EQ(words[4].text, "http://github.com");
    EXPECT_EQ(words[5].text, "arrow");

    EXPECT_EQ(words[0].tag, PartOfSpeechTag::EnglishWord);
    EXPECT_EQ(words[1].tag, PartOfSpeechTag::EnglishWord);
    EXPECT_EQ(words[2].tag, PartOfSpeechTag::EnglishWord);
    EXPECT_EQ(words[3].tag, PartOfSpeechTag::EnglishWord);
    EXPECT_EQ(words[4].tag, PartOfSpeechTag::Url);
    EXPECT_EQ(words[5].tag, PartOfSpeechTag::EnglishWord);
}

TEST(WordSegmenter, Parse_Japanese)
{
    WordSegmenter segmenter;

    std::vector<PartOfSpeech> words;
    auto read = [&](const PartOfSpeech& pos) { words.push_back(pos); };

    segmenter.parse(u8"Somerachan is 不思議なソメラちゃん.", read);

    ASSERT_EQ(3, words.size());
    EXPECT_EQ(words[0].text, "Somerachan");
    EXPECT_EQ(words[1].text, "is");
    EXPECT_EQ(words[2].text, "不思議なソメラちゃん");
}

TEST(WordSegmenter, FastUrlRegex)
{
    std::regex re(R"(.+\..+)");

    EXPECT_FALSE(std::regex_match("http", re));
    EXPECT_FALSE(std::regex_match("https:", re));
    EXPECT_FALSE(std::regex_match("github.", re));
    EXPECT_FALSE(std::regex_match(".com", re));
    EXPECT_TRUE(std::regex_match("/github.com", re));
    EXPECT_TRUE(std::regex_match("//github.com", re));
    EXPECT_TRUE(std::regex_match("github.com", re));
    EXPECT_TRUE(std::regex_match("http://github.com", re));
    EXPECT_TRUE(std::regex_match("http://github.com/atom/atom", re));
    EXPECT_TRUE(std::regex_match("http://github.com/atom/atom.git", re));
    EXPECT_TRUE(std::regex_match("https://github.com", re));
    EXPECT_TRUE(std::regex_match("https://github.com/", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/atom", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/atom.git", re));
}

TEST(WordSegmenter, UrlRegex)
{
    std::regex re(R"((http|https|ftp)://.+)");

    EXPECT_FALSE(std::regex_match("http", re));
    EXPECT_FALSE(std::regex_match("https:", re));
    EXPECT_FALSE(std::regex_match("github.", re));
    EXPECT_FALSE(std::regex_match(".com", re));
    EXPECT_FALSE(std::regex_match("/github.com", re));
    EXPECT_FALSE(std::regex_match("//github.com", re));
    EXPECT_FALSE(std::regex_match("github.com", re));
    EXPECT_TRUE(std::regex_match("http://github.com", re));
    EXPECT_TRUE(std::regex_match("http://github.com/atom/atom", re));
    EXPECT_TRUE(std::regex_match("http://github.com/atom/atom.git", re));
    EXPECT_TRUE(std::regex_match("https://github.com", re));
    EXPECT_TRUE(std::regex_match("https://github.com/", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/atom", re));
    EXPECT_TRUE(std::regex_match("https://github.com/atom/atom.git", re));
    EXPECT_TRUE(std::regex_match("ftp://github.com", re));
    EXPECT_TRUE(std::regex_match("ftp://github.com/atom/atom", re));
    EXPECT_TRUE(std::regex_match("ftp://github.com/atom/atom.git", re));

    EXPECT_FALSE(std::regex_match("git@github.com:atom/atom.git", re));

    // URL with typo/error
    EXPECT_TRUE(std::regex_match("http://s", re));
    EXPECT_TRUE(std::regex_match("http://.com", re));
}

TEST(WordSegmenter, GitUrlRegex)
{
    std::regex re(R"(git@\w+\..+)");
    EXPECT_TRUE(std::regex_match("git@github.com:atom/atom.git", re));
    EXPECT_TRUE(std::regex_match("git@github.com", re));
    EXPECT_FALSE(std::regex_match("github.com", re));
    EXPECT_FALSE(std::regex_match("http://s", re));
    EXPECT_FALSE(std::regex_match("http://github.com/@status", re));
}

TEST(WordSegmenter, IntegerRegex)
{
    std::regex re(R"(\d+)");
    EXPECT_TRUE(std::regex_match("42", re));
    EXPECT_TRUE(std::regex_match("1", re));
    EXPECT_FALSE(std::regex_match("100.0", re));
    EXPECT_FALSE(std::regex_match("0.0f", re));
    EXPECT_FALSE(std::regex_match(".0f", re));
    EXPECT_FALSE(std::regex_match("3.141592f", re));
    EXPECT_FALSE(std::regex_match("a", re));
}

TEST(WordSegmenter, FloatRegex)
{
    std::regex re(R"((\d*\.\d+|\d+\.\d*)f?)");
    EXPECT_TRUE(std::regex_match("100.0", re));
    EXPECT_TRUE(std::regex_match("12.3", re));
    EXPECT_TRUE(std::regex_match(".123", re));
    EXPECT_TRUE(std::regex_match("123.", re));
    EXPECT_TRUE(std::regex_match("12.3f", re));
    EXPECT_TRUE(std::regex_match(".123f", re));
    EXPECT_TRUE(std::regex_match("123.f", re));
    EXPECT_TRUE(std::regex_match("3.141592f", re));
    EXPECT_FALSE(std::regex_match("a", re));
    EXPECT_FALSE(std::regex_match("42", re));
    EXPECT_FALSE(std::regex_match("1", re));
}

TEST(WordSegmenter, DoxygenKeywordRegex)
{
    const std::regex re(R"((\@|\\)((f(\$|\[|\]|\{|\}))|(\$|\@|\\|\&|\~|\<|\>|\#|\%)|([a-z]{1,16})))");
    EXPECT_TRUE(std::regex_match("\\f[", re));
    EXPECT_TRUE(std::regex_match("@f[", re));
    EXPECT_TRUE(std::regex_match("@<", re));
    EXPECT_TRUE(std::regex_match("@>", re));
    EXPECT_TRUE(std::regex_match("\\brief", re));
    EXPECT_TRUE(std::regex_match("@brief", re));
    EXPECT_TRUE(std::regex_match("@param", re));
    EXPECT_TRUE(std::regex_match("@return", re));
    EXPECT_FALSE(std::regex_match("@~English", re));
    EXPECT_FALSE(std::regex_match("@~english", re));
    EXPECT_FALSE(std::regex_match("english", re));
    EXPECT_FALSE(std::regex_match("@brief()", re));
    EXPECT_FALSE(std::regex_match("@Brief()", re));
    EXPECT_FALSE(std::regex_match("@param2", re));
    EXPECT_FALSE(std::regex_match("@return_zero", re));
}

TEST(WordSegmenter, SimpleWordRegex)
{
    const std::regex re(R"([A-Z]*[a-z]+)");
    EXPECT_TRUE(std::regex_match("arrow", re));
    EXPECT_TRUE(std::regex_match("Arrow", re));
    EXPECT_FALSE(std::regex_match("arrow()", re));
    EXPECT_FALSE(std::regex_match("Arrow.", re));
    EXPECT_FALSE(std::regex_match("arrow,", re));
    EXPECT_FALSE(std::regex_match("see:", re));
    EXPECT_FALSE(std::regex_match("see;", re));
    EXPECT_FALSE(std::regex_match("Look!", re));
    EXPECT_FALSE(std::regex_match("arrow\"", re));
    EXPECT_FALSE(std::regex_match("arrow'", re));
    EXPECT_FALSE(std::regex_match("arrow`", re));
    EXPECT_FALSE(std::regex_match("1arrow,", re));
    EXPECT_FALSE(std::regex_match("see:2", re));
    EXPECT_FALSE(std::regex_match("see;abc", re));
    EXPECT_FALSE(std::regex_match("2Look!", re));
}

TEST(WordSegmenter, WordAndRegex)
{
    const std::regex re(R"([A-Za-z]+(\,|\.|\:|\;|\!|\"|\`|\'))");
    EXPECT_TRUE(std::regex_match("arrow.", re));
    EXPECT_TRUE(std::regex_match("Arrow.", re));
    EXPECT_TRUE(std::regex_match("arrow,", re));
    EXPECT_TRUE(std::regex_match("see:", re));
    EXPECT_TRUE(std::regex_match("see;", re));
    EXPECT_TRUE(std::regex_match("Look!", re));
    EXPECT_TRUE(std::regex_match("arrow\"", re));
    EXPECT_TRUE(std::regex_match("arrow'", re));
    EXPECT_TRUE(std::regex_match("arrow`", re));

    EXPECT_FALSE(std::regex_match("arrow", re));
    EXPECT_FALSE(std::regex_match("Arrow", re));
    EXPECT_FALSE(std::regex_match("1arrow,", re));
    EXPECT_FALSE(std::regex_match("see:2", re));
    EXPECT_FALSE(std::regex_match("see;abc", re));
    EXPECT_FALSE(std::regex_match("2Look!", re));
}

TEST(WordSegmenter, ContractionRegex)
{
    const std::regex re(R"([A-Z]?[a-z]+n't)");
    EXPECT_TRUE(std::regex_match("Don't", re));
    EXPECT_TRUE(std::regex_match("don't", re));
    EXPECT_TRUE(std::regex_match("didn't", re));
    EXPECT_TRUE(std::regex_match("haven't", re));
    EXPECT_TRUE(std::regex_match("needn't", re));
    EXPECT_TRUE(std::regex_match("wasn't", re));
    EXPECT_TRUE(std::regex_match("aren't", re));
    EXPECT_TRUE(std::regex_match("won't", re));

    EXPECT_FALSE(std::regex_match("n't", re));
    EXPECT_FALSE(std::regex_match("1n't", re));
    EXPECT_FALSE(std::regex_match(".n't", re));
    EXPECT_FALSE(std::regex_match("n't.", re));
    EXPECT_FALSE(std::regex_match("n't,", re));
    EXPECT_FALSE(std::regex_match("Dn't", re));
}
