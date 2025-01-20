#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_CALTRAC() { return CALTRAC(); }
const ::Opm::ParserKeyword Builtin::get_CARFIN() { return CARFIN(); }
const ::Opm::ParserKeyword Builtin::get_CART() { return CART(); }
const ::Opm::ParserKeyword Builtin::get_CBMOPTS() { return CBMOPTS(); }
const ::Opm::ParserKeyword Builtin::get_CECON() { return CECON(); }
const ::Opm::ParserKeyword Builtin::get_CECONT() { return CECONT(); }
const ::Opm::ParserKeyword Builtin::get_CIRCLE() { return CIRCLE(); }
const ::Opm::ParserKeyword Builtin::get_CNAMES() { return CNAMES(); }
const ::Opm::ParserKeyword Builtin::get_CO2SOL() { return CO2SOL(); }
const ::Opm::ParserKeyword Builtin::get_CO2STOR() { return CO2STOR(); }
const ::Opm::ParserKeyword Builtin::get_CO2STORE() { return CO2STORE(); }
const ::Opm::ParserKeyword Builtin::get_COAL() { return COAL(); }
const ::Opm::ParserKeyword Builtin::get_COALADS() { return COALADS(); }
const ::Opm::ParserKeyword Builtin::get_COALNUM() { return COALNUM(); }
const ::Opm::ParserKeyword Builtin::get_COALPP() { return COALPP(); }
const ::Opm::ParserKeyword Builtin::get_COARSEN() { return COARSEN(); }
const ::Opm::ParserKeyword Builtin::get_COLLAPSE() { return COLLAPSE(); }
const ::Opm::ParserKeyword Builtin::get_COLUMNS() { return COLUMNS(); }
const ::Opm::ParserKeyword Builtin::get_COMPDAT() { return COMPDAT(); }
const ::Opm::ParserKeyword Builtin::get_COMPDATX() { return COMPDATX(); }
const ::Opm::ParserKeyword Builtin::get_COMPFLSH() { return COMPFLSH(); }
const ::Opm::ParserKeyword Builtin::get_COMPIMB() { return COMPIMB(); }
const ::Opm::ParserKeyword Builtin::get_COMPINJK() { return COMPINJK(); }
const ::Opm::ParserKeyword Builtin::get_COMPLMPL() { return COMPLMPL(); }
const ::Opm::ParserKeyword Builtin::get_COMPLUMP() { return COMPLUMP(); }
const ::Opm::ParserKeyword Builtin::get_COMPOFF() { return COMPOFF(); }
const ::Opm::ParserKeyword Builtin::get_COMPORD() { return COMPORD(); }
const ::Opm::ParserKeyword Builtin::get_COMPRIV() { return COMPRIV(); }
const ::Opm::ParserKeyword Builtin::get_COMPRP() { return COMPRP(); }
const ::Opm::ParserKeyword Builtin::get_COMPRPL() { return COMPRPL(); }
const ::Opm::ParserKeyword Builtin::get_COMPS() { return COMPS(); }
const ::Opm::ParserKeyword Builtin::get_COMPSEGL() { return COMPSEGL(); }
const ::Opm::ParserKeyword Builtin::get_COMPSEGS() { return COMPSEGS(); }
const ::Opm::ParserKeyword Builtin::get_COMPTRAJ() { return COMPTRAJ(); }
const ::Opm::ParserKeyword Builtin::get_COMPVE() { return COMPVE(); }
const ::Opm::ParserKeyword Builtin::get_COMPVEL() { return COMPVEL(); }
const ::Opm::ParserKeyword Builtin::get_CONNECTION_PROBE() { return CONNECTION_PROBE(); }
const ::Opm::ParserKeyword Builtin::get_CONNECTION_PROBE_OPM() { return CONNECTION_PROBE_OPM(); }
const ::Opm::ParserKeyword Builtin::get_COORD() { return COORD(); }
const ::Opm::ParserKeyword Builtin::get_COORDSYS() { return COORDSYS(); }
const ::Opm::ParserKeyword Builtin::get_COPY() { return COPY(); }
const ::Opm::ParserKeyword Builtin::get_COPYBOX() { return COPYBOX(); }
const ::Opm::ParserKeyword Builtin::get_COPYREG() { return COPYREG(); }
const ::Opm::ParserKeyword Builtin::get_CPIFACT() { return CPIFACT(); }
const ::Opm::ParserKeyword Builtin::get_CPIFACTL() { return CPIFACTL(); }
const ::Opm::ParserKeyword Builtin::get_CPR() { return CPR(); }
const ::Opm::ParserKeyword Builtin::get_CREF() { return CREF(); }
const ::Opm::ParserKeyword Builtin::get_CREFW() { return CREFW(); }
const ::Opm::ParserKeyword Builtin::get_CREFWS() { return CREFWS(); }
const ::Opm::ParserKeyword Builtin::get_CRITPERM() { return CRITPERM(); }
const ::Opm::ParserKeyword Builtin::get_CSKIN() { return CSKIN(); }

void Builtin::emplaceC() const {
    this->keywords.emplace("CALTRAC", CALTRAC());
    this->keywords.emplace("CARFIN", CARFIN());
    this->keywords.emplace("CART", CART());
    this->keywords.emplace("CBMOPTS", CBMOPTS());
    this->keywords.emplace("CECON", CECON());
    this->keywords.emplace("CECONT", CECONT());
    this->keywords.emplace("CIRCLE", CIRCLE());
    this->keywords.emplace("CNAMES", CNAMES());
    this->keywords.emplace("CO2SOL", CO2SOL());
    this->keywords.emplace("CO2STOR", CO2STOR());
    this->keywords.emplace("CO2STORE", CO2STORE());
    this->keywords.emplace("COAL", COAL());
    this->keywords.emplace("COALADS", COALADS());
    this->keywords.emplace("COALNUM", COALNUM());
    this->keywords.emplace("COALPP", COALPP());
    this->keywords.emplace("COARSEN", COARSEN());
    this->keywords.emplace("COLLAPSE", COLLAPSE());
    this->keywords.emplace("COLUMNS", COLUMNS());
    this->keywords.emplace("COMPDAT", COMPDAT());
    this->keywords.emplace("COMPDATX", COMPDATX());
    this->keywords.emplace("COMPFLSH", COMPFLSH());
    this->keywords.emplace("COMPIMB", COMPIMB());
    this->keywords.emplace("COMPINJK", COMPINJK());
    this->keywords.emplace("COMPLMPL", COMPLMPL());
    this->keywords.emplace("COMPLUMP", COMPLUMP());
    this->keywords.emplace("COMPOFF", COMPOFF());
    this->keywords.emplace("COMPORD", COMPORD());
    this->keywords.emplace("COMPRIV", COMPRIV());
    this->keywords.emplace("COMPRP", COMPRP());
    this->keywords.emplace("COMPRPL", COMPRPL());
    this->keywords.emplace("COMPS", COMPS());
    this->keywords.emplace("COMPSEGL", COMPSEGL());
    this->keywords.emplace("COMPSEGS", COMPSEGS());
    this->keywords.emplace("COMPTRAJ", COMPTRAJ());
    this->keywords.emplace("COMPVE", COMPVE());
    this->keywords.emplace("COMPVEL", COMPVEL());
    this->keywords.emplace("CONNECTION_PROBE", CONNECTION_PROBE());
    this->keywords.emplace("CONNECTION_PROBE_OPM", CONNECTION_PROBE_OPM());
    this->keywords.emplace("COORD", COORD());
    this->keywords.emplace("COORDSYS", COORDSYS());
    this->keywords.emplace("COPY", COPY());
    this->keywords.emplace("COPYBOX", COPYBOX());
    this->keywords.emplace("COPYREG", COPYREG());
    this->keywords.emplace("CPIFACT", CPIFACT());
    this->keywords.emplace("CPIFACTL", CPIFACTL());
    this->keywords.emplace("CPR", CPR());
    this->keywords.emplace("CREF", CREF());
    this->keywords.emplace("CREFW", CREFW());
    this->keywords.emplace("CREFWS", CREFWS());
    this->keywords.emplace("CRITPERM", CRITPERM());
    this->keywords.emplace("CSKIN", CSKIN());
}
} }
