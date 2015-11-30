// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "CommandLineParser.h"
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
