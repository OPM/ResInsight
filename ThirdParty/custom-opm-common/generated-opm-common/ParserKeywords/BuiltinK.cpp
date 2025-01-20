#include <opm/input/eclipse/Parser/ParserKeywords/K.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_KRNUM() { return KRNUM(); }
const ::Opm::ParserKeyword Builtin::get_KRNUMMF() { return KRNUMMF(); }

void Builtin::emplaceK() const {
    this->keywords.emplace("KRNUM", KRNUM());
    this->keywords.emplace("KRNUMMF", KRNUMMF());
}
} }
