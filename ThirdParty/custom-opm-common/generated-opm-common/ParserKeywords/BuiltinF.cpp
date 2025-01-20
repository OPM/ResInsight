#include <opm/input/eclipse/Parser/ParserKeywords/F.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_FAULTDIM() { return FAULTDIM(); }
const ::Opm::ParserKeyword Builtin::get_FAULTS() { return FAULTS(); }
const ::Opm::ParserKeyword Builtin::get_FBHPDEF() { return FBHPDEF(); }
const ::Opm::ParserKeyword Builtin::get_FHERCHBL() { return FHERCHBL(); }
const ::Opm::ParserKeyword Builtin::get_FIELD() { return FIELD(); }
const ::Opm::ParserKeyword Builtin::get_FIELDSEP() { return FIELDSEP(); }
const ::Opm::ParserKeyword Builtin::get_FIELD_PROBE() { return FIELD_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_FIELD_PROBE_OPM() { return FIELD_PROBE_OPM(); }
const ::Opm::ParserKeyword Builtin::get_FILEUNIT() { return FILEUNIT(); }
const ::Opm::ParserKeyword Builtin::get_FILLEPS() { return FILLEPS(); }
const ::Opm::ParserKeyword Builtin::get_FIPNUM() { return FIPNUM(); }
const ::Opm::ParserKeyword Builtin::get_FIPOWG() { return FIPOWG(); }
const ::Opm::ParserKeyword Builtin::get_FIPSEP() { return FIPSEP(); }
const ::Opm::ParserKeyword Builtin::get_FIP_PROBE() { return FIP_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_FLUXNUM() { return FLUXNUM(); }
const ::Opm::ParserKeyword Builtin::get_FLUXREG() { return FLUXREG(); }
const ::Opm::ParserKeyword Builtin::get_FLUXTYPE() { return FLUXTYPE(); }
const ::Opm::ParserKeyword Builtin::get_FMTHMD() { return FMTHMD(); }
const ::Opm::ParserKeyword Builtin::get_FMTIN() { return FMTIN(); }
const ::Opm::ParserKeyword Builtin::get_FMTOUT() { return FMTOUT(); }
const ::Opm::ParserKeyword Builtin::get_FMWSET() { return FMWSET(); }
const ::Opm::ParserKeyword Builtin::get_FOAM() { return FOAM(); }
const ::Opm::ParserKeyword Builtin::get_FOAMADS() { return FOAMADS(); }
const ::Opm::ParserKeyword Builtin::get_FOAMDCYO() { return FOAMDCYO(); }
const ::Opm::ParserKeyword Builtin::get_FOAMDCYW() { return FOAMDCYW(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFCN() { return FOAMFCN(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFRM() { return FOAMFRM(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFSC() { return FOAMFSC(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFSO() { return FOAMFSO(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFST() { return FOAMFST(); }
const ::Opm::ParserKeyword Builtin::get_FOAMFSW() { return FOAMFSW(); }
const ::Opm::ParserKeyword Builtin::get_FOAMMOB() { return FOAMMOB(); }
const ::Opm::ParserKeyword Builtin::get_FOAMMOBP() { return FOAMMOBP(); }
const ::Opm::ParserKeyword Builtin::get_FOAMMOBS() { return FOAMMOBS(); }
const ::Opm::ParserKeyword Builtin::get_FOAMOPTS() { return FOAMOPTS(); }
const ::Opm::ParserKeyword Builtin::get_FOAMROCK() { return FOAMROCK(); }
const ::Opm::ParserKeyword Builtin::get_FORMFEED() { return FORMFEED(); }
const ::Opm::ParserKeyword Builtin::get_FRICTION() { return FRICTION(); }
const ::Opm::ParserKeyword Builtin::get_FULLIMP() { return FULLIMP(); }

void Builtin::emplaceF() const {
    this->keywords.emplace("FAULTDIM", FAULTDIM());
    this->keywords.emplace("FAULTS", FAULTS());
    this->keywords.emplace("FBHPDEF", FBHPDEF());
    this->keywords.emplace("FHERCHBL", FHERCHBL());
    this->keywords.emplace("FIELD", FIELD());
    this->keywords.emplace("FIELDSEP", FIELDSEP());
    this->keywords.emplace("FIELD_PROBE", FIELD_PROBE());
    this->keywords.emplace("FIELD_PROBE_OPM", FIELD_PROBE_OPM());
    this->keywords.emplace("FILEUNIT", FILEUNIT());
    this->keywords.emplace("FILLEPS", FILLEPS());
    this->keywords.emplace("FIPNUM", FIPNUM());
    this->keywords.emplace("FIPOWG", FIPOWG());
    this->keywords.emplace("FIPSEP", FIPSEP());
    this->keywords.emplace("FIP_PROBE", FIP_PROBE());
    this->keywords.emplace("FLUXNUM", FLUXNUM());
    this->keywords.emplace("FLUXREG", FLUXREG());
    this->keywords.emplace("FLUXTYPE", FLUXTYPE());
    this->keywords.emplace("FMTHMD", FMTHMD());
    this->keywords.emplace("FMTIN", FMTIN());
    this->keywords.emplace("FMTOUT", FMTOUT());
    this->keywords.emplace("FMWSET", FMWSET());
    this->keywords.emplace("FOAM", FOAM());
    this->keywords.emplace("FOAMADS", FOAMADS());
    this->keywords.emplace("FOAMDCYO", FOAMDCYO());
    this->keywords.emplace("FOAMDCYW", FOAMDCYW());
    this->keywords.emplace("FOAMFCN", FOAMFCN());
    this->keywords.emplace("FOAMFRM", FOAMFRM());
    this->keywords.emplace("FOAMFSC", FOAMFSC());
    this->keywords.emplace("FOAMFSO", FOAMFSO());
    this->keywords.emplace("FOAMFST", FOAMFST());
    this->keywords.emplace("FOAMFSW", FOAMFSW());
    this->keywords.emplace("FOAMMOB", FOAMMOB());
    this->keywords.emplace("FOAMMOBP", FOAMMOBP());
    this->keywords.emplace("FOAMMOBS", FOAMMOBS());
    this->keywords.emplace("FOAMOPTS", FOAMOPTS());
    this->keywords.emplace("FOAMROCK", FOAMROCK());
    this->keywords.emplace("FORMFEED", FORMFEED());
    this->keywords.emplace("FRICTION", FRICTION());
    this->keywords.emplace("FULLIMP", FULLIMP());
}
} }
