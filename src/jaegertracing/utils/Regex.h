#ifndef JAEGERTRACING_UTILS_REGEX_H
#define JAEGERTRACING_UTILS_REGEX_H

/*
 * std::regex wrapper that falls back to boost::regex for gcc 4.8.x, because of
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
 */

#ifdef USE_BOOST_REGEX
#include <boost/regex.hpp>
#else
#include <regex>
#endif

namespace jaegertracing {
namespace utils {
namespace regex {

#ifdef USE_BOOST_REGEX
    typedef boost::regex regex;
    typedef boost::smatch smatch;

    inline bool regex_match(const std::string& haystack, boost::smatch& results, const boost::regex& pattern) {
        return boost::regex_match(haystack, results, pattern);
    }

    inline bool regex_search(const std::string& haystack, const boost::regex& pattern) {
        return boost::regex_search(haystack, pattern);
    }
    inline bool regex_search(const std::string& haystack, boost::smatch& results, const boost::regex& pattern) {
        return boost::regex_search(haystack, results, pattern);
    }

#else

    typedef std::regex regex;
    typedef std::smatch smatch;

    inline bool regex_match(const std::string& haystack, std::smatch& results, const std::regex& pattern) {
        return std::regex_match(haystack, results, pattern);
    }

    inline bool regex_search(const std::string& haystack, const std::regex& pattern) {
        return std::regex_search(haystack, pattern);
    }

    inline bool regex_search(const std::string& haystack, std::smatch& results, const std::regex& pattern) {
        return std::regex_search(haystack, results, pattern);
    }
#endif // USE_BOOST_REGEX
}
}
}
#endif

