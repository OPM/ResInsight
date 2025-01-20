#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_ECHO() { return ECHO(); }
const ::Opm::ParserKeyword Builtin::get_ECLMC() { return ECLMC(); }
const ::Opm::ParserKeyword Builtin::get_EDIT() { return EDIT(); }
const ::Opm::ParserKeyword Builtin::get_EDITNNC() { return EDITNNC(); }
const ::Opm::ParserKeyword Builtin::get_EDITNNCR() { return EDITNNCR(); }
const ::Opm::ParserKeyword Builtin::get_EHYSTR() { return EHYSTR(); }
const ::Opm::ParserKeyword Builtin::get_EHYSTRR() { return EHYSTRR(); }
const ::Opm::ParserKeyword Builtin::get_END() { return END(); }
const ::Opm::ParserKeyword Builtin::get_ENDACTIO() { return ENDACTIO(); }
const ::Opm::ParserKeyword Builtin::get_ENDBOX() { return ENDBOX(); }
const ::Opm::ParserKeyword Builtin::get_ENDDYN() { return ENDDYN(); }
const ::Opm::ParserKeyword Builtin::get_ENDFIN() { return ENDFIN(); }
const ::Opm::ParserKeyword Builtin::get_ENDINC() { return ENDINC(); }
const ::Opm::ParserKeyword Builtin::get_ENDNUM() { return ENDNUM(); }
const ::Opm::ParserKeyword Builtin::get_ENDPOINT_SPECIFIERS() { return ENDPOINT_SPECIFIERS(); }
const ::Opm::ParserKeyword Builtin::get_ENDSCALE() { return ENDSCALE(); }
const ::Opm::ParserKeyword Builtin::get_ENDSKIP() { return ENDSKIP(); }
const ::Opm::ParserKeyword Builtin::get_ENKRVD() { return ENKRVD(); }
const ::Opm::ParserKeyword Builtin::get_ENPCVD() { return ENPCVD(); }
const ::Opm::ParserKeyword Builtin::get_ENPTVD() { return ENPTVD(); }
const ::Opm::ParserKeyword Builtin::get_ENSPCVD() { return ENSPCVD(); }
const ::Opm::ParserKeyword Builtin::get_EOS() { return EOS(); }
const ::Opm::ParserKeyword Builtin::get_EOSNUM() { return EOSNUM(); }
const ::Opm::ParserKeyword Builtin::get_EPSDBGS() { return EPSDBGS(); }
const ::Opm::ParserKeyword Builtin::get_EPSDEBUG() { return EPSDEBUG(); }
const ::Opm::ParserKeyword Builtin::get_EQLDIMS() { return EQLDIMS(); }
const ::Opm::ParserKeyword Builtin::get_EQLNUM() { return EQLNUM(); }
const ::Opm::ParserKeyword Builtin::get_EQLOPTS() { return EQLOPTS(); }
const ::Opm::ParserKeyword Builtin::get_EQLZCORN() { return EQLZCORN(); }
const ::Opm::ParserKeyword Builtin::get_EQUALREG() { return EQUALREG(); }
const ::Opm::ParserKeyword Builtin::get_EQUALS() { return EQUALS(); }
const ::Opm::ParserKeyword Builtin::get_EQUIL() { return EQUIL(); }
const ::Opm::ParserKeyword Builtin::get_ESSNODE() { return ESSNODE(); }
const ::Opm::ParserKeyword Builtin::get_EXCAVATE() { return EXCAVATE(); }
const ::Opm::ParserKeyword Builtin::get_EXCEL() { return EXCEL(); }
const ::Opm::ParserKeyword Builtin::get_EXIT() { return EXIT(); }
const ::Opm::ParserKeyword Builtin::get_EXTFIN() { return EXTFIN(); }
const ::Opm::ParserKeyword Builtin::get_EXTHOST() { return EXTHOST(); }
const ::Opm::ParserKeyword Builtin::get_EXTRAPMS() { return EXTRAPMS(); }
const ::Opm::ParserKeyword Builtin::get_EXTREPGL() { return EXTREPGL(); }

void Builtin::emplaceE() const {
    this->keywords.emplace("ECHO", ECHO());
    this->keywords.emplace("ECLMC", ECLMC());
    this->keywords.emplace("EDIT", EDIT());
    this->keywords.emplace("EDITNNC", EDITNNC());
    this->keywords.emplace("EDITNNCR", EDITNNCR());
    this->keywords.emplace("EHYSTR", EHYSTR());
    this->keywords.emplace("EHYSTRR", EHYSTRR());
    this->keywords.emplace("END", END());
    this->keywords.emplace("ENDACTIO", ENDACTIO());
    this->keywords.emplace("ENDBOX", ENDBOX());
    this->keywords.emplace("ENDDYN", ENDDYN());
    this->keywords.emplace("ENDFIN", ENDFIN());
    this->keywords.emplace("ENDINC", ENDINC());
    this->keywords.emplace("ENDNUM", ENDNUM());
    this->keywords.emplace("ENDPOINT_SPECIFIERS", ENDPOINT_SPECIFIERS());
    this->keywords.emplace("ENDSCALE", ENDSCALE());
    this->keywords.emplace("ENDSKIP", ENDSKIP());
    this->keywords.emplace("ENKRVD", ENKRVD());
    this->keywords.emplace("ENPCVD", ENPCVD());
    this->keywords.emplace("ENPTVD", ENPTVD());
    this->keywords.emplace("ENSPCVD", ENSPCVD());
    this->keywords.emplace("EOS", EOS());
    this->keywords.emplace("EOSNUM", EOSNUM());
    this->keywords.emplace("EPSDBGS", EPSDBGS());
    this->keywords.emplace("EPSDEBUG", EPSDEBUG());
    this->keywords.emplace("EQLDIMS", EQLDIMS());
    this->keywords.emplace("EQLNUM", EQLNUM());
    this->keywords.emplace("EQLOPTS", EQLOPTS());
    this->keywords.emplace("EQLZCORN", EQLZCORN());
    this->keywords.emplace("EQUALREG", EQUALREG());
    this->keywords.emplace("EQUALS", EQUALS());
    this->keywords.emplace("EQUIL", EQUIL());
    this->keywords.emplace("ESSNODE", ESSNODE());
    this->keywords.emplace("EXCAVATE", EXCAVATE());
    this->keywords.emplace("EXCEL", EXCEL());
    this->keywords.emplace("EXIT", EXIT());
    this->keywords.emplace("EXTFIN", EXTFIN());
    this->keywords.emplace("EXTHOST", EXTHOST());
    this->keywords.emplace("EXTRAPMS", EXTRAPMS());
    this->keywords.emplace("EXTREPGL", EXTREPGL());
}
} }
