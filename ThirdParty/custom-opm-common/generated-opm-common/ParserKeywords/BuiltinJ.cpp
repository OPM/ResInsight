#include <opm/input/eclipse/Parser/ParserKeywords/J.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_JFUNC() { return JFUNC(); }
const ::Opm::ParserKeyword Builtin::get_JFUNCR() { return JFUNCR(); }

void Builtin::emplaceJ() const {
    this->keywords.emplace("JFUNC", JFUNC());
    this->keywords.emplace("JFUNCR", JFUNCR());
}
} }
