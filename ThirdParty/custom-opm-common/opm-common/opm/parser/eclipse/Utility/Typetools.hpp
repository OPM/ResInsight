#ifndef OPM_TYPETOOLS_HPP
#define OPM_TYPETOOLS_HPP
#include <string>
#include <algorithm>

#include <opm/parser/eclipse/Deck/UDAValue.hpp>

namespace Opm {

enum class type_tag {
    unknown = 0,
    integer = 1,
    string  = 2,
    raw_string = 3,
    fdouble = 4,
    uda     = 5,
};

/*
  The RawString class itself is just a glorified std::string, it does not have
  any additional data nor behavior which differentiates it from std::string, but
  the use of a separate string class allows the compiler to differentiate
  between different behavior for normal strings and raw strings. The special
  behavior for raw strings is:

    1. The input data is terminated on the *last* '/' and not the first - to allow
       '/' as part of the input.

    2. '* is not treated as a multiplier/default, but rather as a normal token.

    3. Quotes are not removed from the input, and when writing quotes are not
       added.

*/

class RawString : public std::string
{
public:

    static std::vector<std::string> strings(const std::vector<RawString>& raw_strings) {
        std::vector<std::string> std_strings;
        for (const auto& rstring : raw_strings)
            std_strings.push_back( rstring );
        return std_strings;
    }

};

inline std::string tag_name( type_tag x ) {
    switch( x ) {
        case type_tag::integer:     return "int";
        case type_tag::string:      return "std::string";
        case type_tag::raw_string:  return "RawString";
        case type_tag::fdouble:     return "double";
        case type_tag::uda:         return "UDAValue";
        case type_tag::unknown:     return "unknown";
    }
    return "unknown";
}

template< typename T > type_tag get_type();

template<> inline type_tag get_type< int >() {
    return type_tag::integer;
}

template<> inline type_tag get_type< double >() {
    return type_tag::fdouble;
}

template<> inline type_tag get_type< std::string >() {
    return type_tag::string;
}

template<> inline type_tag get_type< RawString >() {
    return type_tag::raw_string;
}

template<> inline type_tag get_type<UDAValue>() {
  return type_tag::uda;
}

}

#endif //OPM_TYPETOOLS_HPP
