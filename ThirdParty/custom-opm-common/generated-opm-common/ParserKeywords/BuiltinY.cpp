#include <opm/input/eclipse/Parser/ParserKeywords/Y.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_YMF() { return YMF(); }
const ::Opm::ParserKeyword Builtin::get_YMODULE() { return YMODULE(); }

void Builtin::emplaceY() const {
    this->keywords.emplace("YMF", YMF());
    this->keywords.emplace("YMODULE", YMODULE());
}
} }
