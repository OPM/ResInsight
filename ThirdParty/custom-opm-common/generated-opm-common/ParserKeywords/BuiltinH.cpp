#include <opm/input/eclipse/Parser/ParserKeywords/H.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_H2SOL() { return H2SOL(); }
const ::Opm::ParserKeyword Builtin::get_H2STORE() { return H2STORE(); }
const ::Opm::ParserKeyword Builtin::get_HALFTRAN() { return HALFTRAN(); }
const ::Opm::ParserKeyword Builtin::get_HAxxxxxx() { return HAxxxxxx(); }
const ::Opm::ParserKeyword Builtin::get_HBNUM() { return HBNUM(); }
const ::Opm::ParserKeyword Builtin::get_HDISP() { return HDISP(); }
const ::Opm::ParserKeyword Builtin::get_HEATCR() { return HEATCR(); }
const ::Opm::ParserKeyword Builtin::get_HEATCRT() { return HEATCRT(); }
const ::Opm::ParserKeyword Builtin::get_HMAQUCT() { return HMAQUCT(); }
const ::Opm::ParserKeyword Builtin::get_HMAQUFET() { return HMAQUFET(); }
const ::Opm::ParserKeyword Builtin::get_HMAQUNUM() { return HMAQUNUM(); }
const ::Opm::ParserKeyword Builtin::get_HMDIMS() { return HMDIMS(); }
const ::Opm::ParserKeyword Builtin::get_HMFAULTS() { return HMFAULTS(); }
const ::Opm::ParserKeyword Builtin::get_HMMLAQUN() { return HMMLAQUN(); }
const ::Opm::ParserKeyword Builtin::get_HMMLCTAQ() { return HMMLCTAQ(); }
const ::Opm::ParserKeyword Builtin::get_HMMLFTAQ() { return HMMLFTAQ(); }
const ::Opm::ParserKeyword Builtin::get_HMMLTWCN() { return HMMLTWCN(); }
const ::Opm::ParserKeyword Builtin::get_HMMULTFT() { return HMMULTFT(); }
const ::Opm::ParserKeyword Builtin::get_HMMULTSG() { return HMMULTSG(); }
const ::Opm::ParserKeyword Builtin::get_HMMULTxx() { return HMMULTxx(); }
const ::Opm::ParserKeyword Builtin::get_HMPROPS() { return HMPROPS(); }
const ::Opm::ParserKeyword Builtin::get_HMROCK() { return HMROCK(); }
const ::Opm::ParserKeyword Builtin::get_HMROCKT() { return HMROCKT(); }
const ::Opm::ParserKeyword Builtin::get_HMRREF() { return HMRREF(); }
const ::Opm::ParserKeyword Builtin::get_HMWELCON() { return HMWELCON(); }
const ::Opm::ParserKeyword Builtin::get_HMWPIMLT() { return HMWPIMLT(); }
const ::Opm::ParserKeyword Builtin::get_HMxxxxxx() { return HMxxxxxx(); }
const ::Opm::ParserKeyword Builtin::get_HRFIN() { return HRFIN(); }
const ::Opm::ParserKeyword Builtin::get_HWELLS() { return HWELLS(); }
const ::Opm::ParserKeyword Builtin::get_HWKRO() { return HWKRO(); }
const ::Opm::ParserKeyword Builtin::get_HWKRORG() { return HWKRORG(); }
const ::Opm::ParserKeyword Builtin::get_HWKRORW() { return HWKRORW(); }
const ::Opm::ParserKeyword Builtin::get_HWKRW() { return HWKRW(); }
const ::Opm::ParserKeyword Builtin::get_HWKRWR() { return HWKRWR(); }
const ::Opm::ParserKeyword Builtin::get_HWPCW() { return HWPCW(); }
const ::Opm::ParserKeyword Builtin::get_HWSNUM() { return HWSNUM(); }
const ::Opm::ParserKeyword Builtin::get_HWSOGCR() { return HWSOGCR(); }
const ::Opm::ParserKeyword Builtin::get_HWSOWCR() { return HWSOWCR(); }
const ::Opm::ParserKeyword Builtin::get_HWSWCR() { return HWSWCR(); }
const ::Opm::ParserKeyword Builtin::get_HWSWL() { return HWSWL(); }
const ::Opm::ParserKeyword Builtin::get_HWSWLPC() { return HWSWLPC(); }
const ::Opm::ParserKeyword Builtin::get_HWSWU() { return HWSWU(); }
const ::Opm::ParserKeyword Builtin::get_HXFIN() { return HXFIN(); }
const ::Opm::ParserKeyword Builtin::get_HYDRHEAD() { return HYDRHEAD(); }
const ::Opm::ParserKeyword Builtin::get_HYFIN() { return HYFIN(); }
const ::Opm::ParserKeyword Builtin::get_HYMOBGDR() { return HYMOBGDR(); }
const ::Opm::ParserKeyword Builtin::get_HYST() { return HYST(); }
const ::Opm::ParserKeyword Builtin::get_HYSTCHCK() { return HYSTCHCK(); }
const ::Opm::ParserKeyword Builtin::get_HZFIN() { return HZFIN(); }

void Builtin::emplaceH() const {
    this->keywords.emplace("H2SOL", H2SOL());
    this->keywords.emplace("H2STORE", H2STORE());
    this->keywords.emplace("HALFTRAN", HALFTRAN());
    this->keywords.emplace("HAxxxxxx", HAxxxxxx());
    this->keywords.emplace("HBNUM", HBNUM());
    this->keywords.emplace("HDISP", HDISP());
    this->keywords.emplace("HEATCR", HEATCR());
    this->keywords.emplace("HEATCRT", HEATCRT());
    this->keywords.emplace("HMAQUCT", HMAQUCT());
    this->keywords.emplace("HMAQUFET", HMAQUFET());
    this->keywords.emplace("HMAQUNUM", HMAQUNUM());
    this->keywords.emplace("HMDIMS", HMDIMS());
    this->keywords.emplace("HMFAULTS", HMFAULTS());
    this->keywords.emplace("HMMLAQUN", HMMLAQUN());
    this->keywords.emplace("HMMLCTAQ", HMMLCTAQ());
    this->keywords.emplace("HMMLFTAQ", HMMLFTAQ());
    this->keywords.emplace("HMMLTWCN", HMMLTWCN());
    this->keywords.emplace("HMMULTFT", HMMULTFT());
    this->keywords.emplace("HMMULTSG", HMMULTSG());
    this->keywords.emplace("HMMULTxx", HMMULTxx());
    this->keywords.emplace("HMPROPS", HMPROPS());
    this->keywords.emplace("HMROCK", HMROCK());
    this->keywords.emplace("HMROCKT", HMROCKT());
    this->keywords.emplace("HMRREF", HMRREF());
    this->keywords.emplace("HMWELCON", HMWELCON());
    this->keywords.emplace("HMWPIMLT", HMWPIMLT());
    this->keywords.emplace("HMxxxxxx", HMxxxxxx());
    this->keywords.emplace("HRFIN", HRFIN());
    this->keywords.emplace("HWELLS", HWELLS());
    this->keywords.emplace("HWKRO", HWKRO());
    this->keywords.emplace("HWKRORG", HWKRORG());
    this->keywords.emplace("HWKRORW", HWKRORW());
    this->keywords.emplace("HWKRW", HWKRW());
    this->keywords.emplace("HWKRWR", HWKRWR());
    this->keywords.emplace("HWPCW", HWPCW());
    this->keywords.emplace("HWSNUM", HWSNUM());
    this->keywords.emplace("HWSOGCR", HWSOGCR());
    this->keywords.emplace("HWSOWCR", HWSOWCR());
    this->keywords.emplace("HWSWCR", HWSWCR());
    this->keywords.emplace("HWSWL", HWSWL());
    this->keywords.emplace("HWSWLPC", HWSWLPC());
    this->keywords.emplace("HWSWU", HWSWU());
    this->keywords.emplace("HXFIN", HXFIN());
    this->keywords.emplace("HYDRHEAD", HYDRHEAD());
    this->keywords.emplace("HYFIN", HYFIN());
    this->keywords.emplace("HYMOBGDR", HYMOBGDR());
    this->keywords.emplace("HYST", HYST());
    this->keywords.emplace("HYSTCHCK", HYSTCHCK());
    this->keywords.emplace("HZFIN", HZFIN());
}
} }
