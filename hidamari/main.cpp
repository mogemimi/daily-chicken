// Copyright (c) 2015 mogemimi. Distributed under the MIT license.

#include <iostream>
#include <vector>
#include <string>

namespace somera {

struct CompileOptions {
    std::vector<std::string> sources;
    std::vector<std::string> includePaths;
    std::vector<std::string> libraies;
    std::vector<std::string> librarySearchPaths;
};

//-g

} // namespace somera

int main(int argc, char *argv[])
{
    std::cout << "finished" << std::endl;
    return 0;
}
