#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>
#include <algorithm>
#include <numeric> // For std::accumulate
#include <cctype>  // For std::tolower and std::toupper

namespace StringUtil {

    inline std::vector<std::string> split(const std::string& str,
                                            const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        size_t lastPos = 0;
        while ((pos = str.find(delimiter, lastPos)) != std::string::npos) {
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = pos + delimiter.length();
        }
        tokens.push_back(str.substr(lastPos));
        return tokens;
    }

    inline std::string replace(const std::string& str,
                                const std::string& oldSubstr,
                                const std::string& newSubstr) {
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(oldSubstr, pos)) != std::string::npos) {
            result.replace(pos, oldSubstr.length(), newSubstr);
            pos += newSubstr.length();
        }
        return result;
    }

    inline std::string join(const std::vector<std::string>& strings,
                            const std::string& delimiter) {
        if (strings.empty()) {
            return "";
        }
        return std::accumulate(strings.begin() + 1, strings.end(), strings[0],
                                [&](std::string a, std::string b) {
                                    return a + delimiter + b;
                                });
    }

    inline std::string lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
                        [](unsigned char c){ return std::tolower(c); });
        return str;
    }

    inline std::string upper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
                        [](unsigned char c){ return std::toupper(c); });
        return str;
    }

} 

#endif // STRING_UTIL_H