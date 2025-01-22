#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_BC() { return BC(); }
const ::Opm::ParserKeyword Builtin::get_BCCON() { return BCCON(); }
const ::Opm::ParserKeyword Builtin::get_BCPROP() { return BCPROP(); }
const ::Opm::ParserKeyword Builtin::get_BDENSITY() { return BDENSITY(); }
const ::Opm::ParserKeyword Builtin::get_BGGI() { return BGGI(); }
const ::Opm::ParserKeyword Builtin::get_BIC() { return BIC(); }
const ::Opm::ParserKeyword Builtin::get_BIGMODEL() { return BIGMODEL(); }
const ::Opm::ParserKeyword Builtin::get_BIOTCOEF() { return BIOTCOEF(); }
const ::Opm::ParserKeyword Builtin::get_BLACKOIL() { return BLACKOIL(); }
const ::Opm::ParserKeyword Builtin::get_BLOCK_PROBE() { return BLOCK_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_BLOCK_PROBE300() { return BLOCK_PROBE300(); }
const ::Opm::ParserKeyword Builtin::get_BOGI() { return BOGI(); }
const ::Opm::ParserKeyword Builtin::get_BOUNDARY() { return BOUNDARY(); }
const ::Opm::ParserKeyword Builtin::get_BOX() { return BOX(); }
const ::Opm::ParserKeyword Builtin::get_BPARA() { return BPARA(); }
const ::Opm::ParserKeyword Builtin::get_BPIDIMS() { return BPIDIMS(); }
const ::Opm::ParserKeyword Builtin::get_BRANPROP() { return BRANPROP(); }
const ::Opm::ParserKeyword Builtin::get_BRINE() { return BRINE(); }
const ::Opm::ParserKeyword Builtin::get_BTOBALFA() { return BTOBALFA(); }
const ::Opm::ParserKeyword Builtin::get_BTOBALFV() { return BTOBALFV(); }

void Builtin::emplaceB() const {
    this->keywords.emplace("BC", BC());
    this->keywords.emplace("BCCON", BCCON());
    this->keywords.emplace("BCPROP", BCPROP());
    this->keywords.emplace("BDENSITY", BDENSITY());
    this->keywords.emplace("BGGI", BGGI());
    this->keywords.emplace("BIC", BIC());
    this->keywords.emplace("BIGMODEL", BIGMODEL());
    this->keywords.emplace("BIOTCOEF", BIOTCOEF());
    this->keywords.emplace("BLACKOIL", BLACKOIL());
    this->keywords.emplace("BLOCK_PROBE", BLOCK_PROBE());
    this->keywords.emplace("BLOCK_PROBE300", BLOCK_PROBE300());
    this->keywords.emplace("BOGI", BOGI());
    this->keywords.emplace("BOUNDARY", BOUNDARY());
    this->keywords.emplace("BOX", BOX());
    this->keywords.emplace("BPARA", BPARA());
    this->keywords.emplace("BPIDIMS", BPIDIMS());
    this->keywords.emplace("BRANPROP", BRANPROP());
    this->keywords.emplace("BRINE", BRINE());
    this->keywords.emplace("BTOBALFA", BTOBALFA());
    this->keywords.emplace("BTOBALFV", BTOBALFV());
}
} }
