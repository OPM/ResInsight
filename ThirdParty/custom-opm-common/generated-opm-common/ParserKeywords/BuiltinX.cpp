#include <opm/input/eclipse/Parser/ParserKeywords/X.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_XMF() { return XMF(); }

void Builtin::emplaceX() const {
    this->keywords.emplace("XMF", XMF());
}
} }
