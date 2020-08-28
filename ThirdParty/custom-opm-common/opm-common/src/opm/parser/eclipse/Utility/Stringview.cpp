#include <iterator>
#include <ostream>

#include <opm/parser/eclipse/Utility/Stringview.hpp>

std::ostream& Opm::operator<<( std::ostream& stream, const Opm::string_view& view ) {
    std::copy( view.begin(), view.end(), std::ostream_iterator< char >( stream ) );
    return stream;
}
