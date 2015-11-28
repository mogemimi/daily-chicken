// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
#include "daily/FileSystem.h"
#include "daily/StringHelper.h"
#include <gtest/iutest_switch.hpp>

using namespace somera;

TEST(CommandLineParser, TrivialCase)
{
    constexpr int argc = 4;
    const char* argv[argc] = {
        "/usr/local/bin/gcc",
        "-g",
        "hidamari.cpp",
        "sketch.cpp",
    };

    CommandLineParser parser;
    parser.addArgument("-g", CommandLineArgumentType::Flag,
        "Debug flag");
    parser.addArgument("-L", CommandLineArgumentType::JoinedOrSeparate,
        "Library search path");
    parser.parse(argc, argv);

    EXPECT_FALSE(parser.hasParseError());
    EXPECT_TRUE(parser.exists("-g"));
    EXPECT_TRUE(parser.getErrorMessage().empty());
    EXPECT_EQ("/usr/local/bin/gcc", parser.getExecutablePath());

    auto paths = parser.getPaths();
    ASSERT_EQ(2, paths.size());
    EXPECT_EQ("hidamari.cpp", paths[0]);
    EXPECT_EQ("sketch.cpp", paths[1]);
}

TEST(StringHelper, startWith)
{
    const auto startWith = StringHelper::startWith;
    EXPECT_TRUE(startWith("", ""));
    EXPECT_TRUE(startWith("-", "-"));
    EXPECT_TRUE(startWith("baka and test", "baka and test"));
    EXPECT_TRUE(startWith("baka and test", "baka"));
    EXPECT_TRUE(startWith("baka", ""));
    EXPECT_FALSE(startWith("baka", "baka and test"));
    EXPECT_FALSE(startWith("", "baka"));
    EXPECT_FALSE(startWith("-baka", "baka"));
    EXPECT_FALSE(startWith("baka", "-baka"));
}

TEST(FileSystem, normalize)
{
    const auto normalize = FileSystem::normalize;
    EXPECT_EQ(FileSystem::getCurrentDirectory(), normalize(""));
    EXPECT_EQ(FileSystem::getCurrentDirectory(), normalize("."));
    EXPECT_EQ(FileSystem::getCurrentDirectory(), normalize("./"));

    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/bin"));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/bin/"));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/bin/."));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/bin/./"));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/./bin"));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/./local/bin"));
    EXPECT_EQ("/usr/local", normalize("/usr/local/bin/.."));
    EXPECT_EQ("/usr/local", normalize("/usr/local/bin/../"));
    EXPECT_EQ("/usr", normalize("/usr/local/bin/../.."));
    EXPECT_EQ("/usr", normalize("/usr/local/bin/../../"));
    EXPECT_EQ("/usr/local/bin", normalize("/usr/local/../local/bin"));
    EXPECT_EQ("/usr/local", normalize("/usr/local/../local/bin/.."));
}

TEST(FileSystem, relative)
{
    const auto relative = FileSystem::relative;
    EXPECT_EQ("", relative("/usr/local/bin", "/usr/local/bin"));
    EXPECT_EQ("", relative("/usr/local/bin/.", "/usr/local/bin"));
    EXPECT_EQ("", relative("/usr/local/bin/./", "/usr/local/bin"));
    EXPECT_EQ("", relative("/usr/local/bin/././", "/usr/local/bin"));
    EXPECT_EQ("", relative("/usr/local/bin/./././", "/usr/local/bin"));
    EXPECT_EQ("..", relative("/usr/local/bin/..", "/usr/local/bin"));
    EXPECT_EQ("..", relative("/usr/local/bin/../", "/usr/local/bin"));
    EXPECT_EQ("..", relative("/usr/local/bin/.././", "/usr/local/bin"));
    EXPECT_EQ("..", relative("/usr/local/bin/./../", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/bin/../../", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/bin/../..", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/bin/../../.", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/bin/.././../", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/bin/./../../", "/usr/local/bin"));

    EXPECT_EQ("", relative("/usr/local/../local/bin", "/usr/local/bin"));
    EXPECT_EQ("..", relative("/usr/local", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/..", "/usr/local/bin"));
    EXPECT_EQ("../..", relative("/usr/local/../local/..", "/usr/local/bin"));

    EXPECT_EQ("gcc", relative("/usr/local/bin/gcc", "/usr/local/bin"));
    EXPECT_EQ("../opt", relative("/usr/local/opt", "/usr/local/bin"));
    EXPECT_EQ("../../share", relative("/usr/share", "/usr/local/bin"));
    EXPECT_EQ("../../share/dict", relative("/usr/share/dict", "/usr/local/bin"));
}
