#include <opm/input/eclipse/Parser/ParserKeywords/Q.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_QDRILL() { return QDRILL(); }
const ::Opm::ParserKeyword Builtin::get_QHRATING() { return QHRATING(); }
const ::Opm::ParserKeyword Builtin::get_QMOBIL() { return QMOBIL(); }

void Builtin::emplaceQ() const {
    this->keywords.emplace("QDRILL", QDRILL());
    this->keywords.emplace("QHRATING", QHRATING());
    this->keywords.emplace("QMOBIL", QMOBIL());
}
} }
