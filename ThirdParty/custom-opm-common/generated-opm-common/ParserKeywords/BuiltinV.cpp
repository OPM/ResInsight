#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_VAPOIL() { return VAPOIL(); }
const ::Opm::ParserKeyword Builtin::get_VAPPARS() { return VAPPARS(); }
const ::Opm::ParserKeyword Builtin::get_VAPWAT() { return VAPWAT(); }
const ::Opm::ParserKeyword Builtin::get_VCRIT() { return VCRIT(); }
const ::Opm::ParserKeyword Builtin::get_VDFLOW() { return VDFLOW(); }
const ::Opm::ParserKeyword Builtin::get_VDFLOWR() { return VDFLOWR(); }
const ::Opm::ParserKeyword Builtin::get_VE() { return VE(); }
const ::Opm::ParserKeyword Builtin::get_VEDEBUG() { return VEDEBUG(); }
const ::Opm::ParserKeyword Builtin::get_VEFIN() { return VEFIN(); }
const ::Opm::ParserKeyword Builtin::get_VEFRAC() { return VEFRAC(); }
const ::Opm::ParserKeyword Builtin::get_VEFRACP() { return VEFRACP(); }
const ::Opm::ParserKeyword Builtin::get_VEFRACPV() { return VEFRACPV(); }
const ::Opm::ParserKeyword Builtin::get_VEFRACV() { return VEFRACV(); }
const ::Opm::ParserKeyword Builtin::get_VFPCHK() { return VFPCHK(); }
const ::Opm::ParserKeyword Builtin::get_VFPIDIMS() { return VFPIDIMS(); }
const ::Opm::ParserKeyword Builtin::get_VFPINJ() { return VFPINJ(); }
const ::Opm::ParserKeyword Builtin::get_VFPPDIMS() { return VFPPDIMS(); }
const ::Opm::ParserKeyword Builtin::get_VFPPROD() { return VFPPROD(); }
const ::Opm::ParserKeyword Builtin::get_VFPTABL() { return VFPTABL(); }
const ::Opm::ParserKeyword Builtin::get_VISAGE() { return VISAGE(); }
const ::Opm::ParserKeyword Builtin::get_VISCAQA() { return VISCAQA(); }
const ::Opm::ParserKeyword Builtin::get_VISCD() { return VISCD(); }
const ::Opm::ParserKeyword Builtin::get_VISCREF() { return VISCREF(); }
const ::Opm::ParserKeyword Builtin::get_VISDATES() { return VISDATES(); }
const ::Opm::ParserKeyword Builtin::get_VISOPTS() { return VISOPTS(); }

void Builtin::emplaceV() const {
    this->keywords.emplace("VAPOIL", VAPOIL());
    this->keywords.emplace("VAPPARS", VAPPARS());
    this->keywords.emplace("VAPWAT", VAPWAT());
    this->keywords.emplace("VCRIT", VCRIT());
    this->keywords.emplace("VDFLOW", VDFLOW());
    this->keywords.emplace("VDFLOWR", VDFLOWR());
    this->keywords.emplace("VE", VE());
    this->keywords.emplace("VEDEBUG", VEDEBUG());
    this->keywords.emplace("VEFIN", VEFIN());
    this->keywords.emplace("VEFRAC", VEFRAC());
    this->keywords.emplace("VEFRACP", VEFRACP());
    this->keywords.emplace("VEFRACPV", VEFRACPV());
    this->keywords.emplace("VEFRACV", VEFRACV());
    this->keywords.emplace("VFPCHK", VFPCHK());
    this->keywords.emplace("VFPIDIMS", VFPIDIMS());
    this->keywords.emplace("VFPINJ", VFPINJ());
    this->keywords.emplace("VFPPDIMS", VFPPDIMS());
    this->keywords.emplace("VFPPROD", VFPPROD());
    this->keywords.emplace("VFPTABL", VFPTABL());
    this->keywords.emplace("VISAGE", VISAGE());
    this->keywords.emplace("VISCAQA", VISCAQA());
    this->keywords.emplace("VISCD", VISCD());
    this->keywords.emplace("VISCREF", VISCREF());
    this->keywords.emplace("VISDATES", VISDATES());
    this->keywords.emplace("VISOPTS", VISOPTS());
}
} }
