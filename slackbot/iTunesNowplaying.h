// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include "../somerachan/src/optional.h"
#include <string>

namespace somera {
namespace iTunesNowplaying {

struct Track {
    std::string trackName;
    std::string artistName;
    std::string albumName;
};

somera::Optional<Track> getCurrentTrack();

} // namespace iTunesNowplaying
} // namespace somera
