#include "wordsegmenter.h"
#include <gtest/iutest_switch.hpp>
#include <vector>

using somera::PartOfSpeechTag;
using somera::PartOfSpeech;
using somera::WordSegmenter;

TEST(WordSegmenter, TrivialCase)
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

TEST(WordSegmenter, TrivialCase2)
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

TEST(WordSegmenter, Url)
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
