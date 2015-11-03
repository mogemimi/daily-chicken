#include <string>

namespace somera {

struct EditDistance {
    static double closestMatchFuzzyDistance(const std::string& left, const std::string& right);

    static double jaroWinklerDistance(const std::string& left, const std::string& right);

    static int levenshteinDistance(const std::string& left, const std::string& right);
};

} // namespace somera
