// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "consolecolor.h"
#include <sstream>

namespace somera {
namespace {

int getColorIndex(TerminalColor color)
{
    switch (color) {
    case TerminalColor::Black: return 0;
    case TerminalColor::Red: return 1;
    case TerminalColor::Green: return 2;
    case TerminalColor::Yellow: return 3;
    case TerminalColor::Blue: return 4;
    case TerminalColor::Magenta: return 5;
    case TerminalColor::Cyan: return 6;
    case TerminalColor::White: return 7;
    }
    return 0;
}

bool hasTerminalColors()
{
    if (::isatty(::fileno(stdout)) == 1) {
        const auto term = ::getenv("TERM");
        return (term != nullptr && ::strcmp(term, "dumb"));
    }
    return false;
}

std::string getColor(
    TerminalColor textColor,
    somera::Optional<TerminalColor> backgroundColor = somera::NullOpt)
{
    std::stringstream ss;
    ss << "\033[";
    ss << (30 + getColorIndex(textColor));
    if (backgroundColor) {
        ss << ";" << (40 + getColorIndex(*backgroundColor));
    }
    ss << "m";
    return ss.str();
}

} // unnamed namespace

std::string changeTerminalTextColor(
    const std::string& text,
    TerminalColor textColor,
    somera::Optional<TerminalColor> backgroundColor)
{
    static const bool enabled = hasTerminalColors();
    if (!enabled) {
        return text;
    }

    constexpr auto resetColor = "\x1b[0m";
    return getColor(textColor, backgroundColor) + text + resetColor;
}

} // namespace somera
