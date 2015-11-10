// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "iTunesNowplaying.h"
#include <cstdio>
#include <sstream>
#include <utility>
#ifdef __APPLE_CC__
#include <Availability.h>
#endif

namespace somera {
namespace iTunesNowPlaying {
namespace {

std::string subprocessCall(const std::string& command)
{
    constexpr int maxBufferSize = 255;
    char buffer[maxBufferSize];
    ::FILE* stream = ::popen(command.c_str(), "r");
    if (stream == nullptr) {
        ::pclose(stream);
        return "";
    }
    std::string output;
    while (::fgets(buffer, maxBufferSize, stream) != nullptr) {
        output.append(buffer);
    }
    ::pclose(stream);
    return std::move(output);
}

} // unnamed namespace

somera::Optional<Track> getCurrentTrack()
{
#if defined(__APPLE_CC__) \
    && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8)
    // for Mac OS X
#define SOMERA_APPLESCRIPT_TOSTRING(x) std::string(#x)
    auto output = subprocessCall("osascript -e" + SOMERA_APPLESCRIPT_TOSTRING(
        'try' -e
            'tell application "iTunes"' -e
                'set trackName to name of current track' -e
                'set artistName to artist of current track' -e
                'set albumName to album of current track' -e
                'return trackName & "\n" & artistName & "\n" & albumName' -e
            'end tell' -e
        'end try'
    ));

    if (output.empty()) {
        return somera::NullOpt;
    }

    Track track;
    std::stringstream stream;
    stream << output;
    std::getline(stream, track.trackName, '\n');
    std::getline(stream, track.artistName, '\n');
    std::getline(stream, track.albumName, '\n');
    return std::move(track);
#else
    return somera::NullOpt;
#endif
}

} // namespace iTunesNowPlaying
} // namespace somera
