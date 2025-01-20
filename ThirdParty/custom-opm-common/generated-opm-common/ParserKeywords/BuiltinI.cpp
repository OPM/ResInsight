#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Builtin.hpp>
namespace Opm { namespace ParserKeywords {
const ::Opm::ParserKeyword Builtin::get_IHOST() { return IHOST(); }
const ::Opm::ParserKeyword Builtin::get_IMBNUM() { return IMBNUM(); }
const ::Opm::ParserKeyword Builtin::get_IMBNUMMF() { return IMBNUMMF(); }
const ::Opm::ParserKeyword Builtin::get_IMKRVD() { return IMKRVD(); }
const ::Opm::ParserKeyword Builtin::get_IMPCVD() { return IMPCVD(); }
const ::Opm::ParserKeyword Builtin::get_IMPES() { return IMPES(); }
const ::Opm::ParserKeyword Builtin::get_IMPLICIT() { return IMPLICIT(); }
const ::Opm::ParserKeyword Builtin::get_IMPORT() { return IMPORT(); }
const ::Opm::ParserKeyword Builtin::get_IMPTVD() { return IMPTVD(); }
const ::Opm::ParserKeyword Builtin::get_IMSPCVD() { return IMSPCVD(); }
const ::Opm::ParserKeyword Builtin::get_INCLUDE() { return INCLUDE(); }
const ::Opm::ParserKeyword Builtin::get_INIT() { return INIT(); }
const ::Opm::ParserKeyword Builtin::get_INRAD() { return INRAD(); }
const ::Opm::ParserKeyword Builtin::get_INSPEC() { return INSPEC(); }
const ::Opm::ParserKeyword Builtin::get_INTPC() { return INTPC(); }
const ::Opm::ParserKeyword Builtin::get_IONROCK() { return IONROCK(); }
const ::Opm::ParserKeyword Builtin::get_IONXROCK() { return IONXROCK(); }
const ::Opm::ParserKeyword Builtin::get_IONXSURF() { return IONXSURF(); }
const ::Opm::ParserKeyword Builtin::get_IPCG() { return IPCG(); }
const ::Opm::ParserKeyword Builtin::get_IPCW() { return IPCW(); }
const ::Opm::ParserKeyword Builtin::get_ISGCR() { return ISGCR(); }
const ::Opm::ParserKeyword Builtin::get_ISGL() { return ISGL(); }
const ::Opm::ParserKeyword Builtin::get_ISGLPC() { return ISGLPC(); }
const ::Opm::ParserKeyword Builtin::get_ISGU() { return ISGU(); }
const ::Opm::ParserKeyword Builtin::get_ISOGCR() { return ISOGCR(); }
const ::Opm::ParserKeyword Builtin::get_ISOLNUM() { return ISOLNUM(); }
const ::Opm::ParserKeyword Builtin::get_ISOWCR() { return ISOWCR(); }
const ::Opm::ParserKeyword Builtin::get_ISWCR() { return ISWCR(); }
const ::Opm::ParserKeyword Builtin::get_ISWL() { return ISWL(); }
const ::Opm::ParserKeyword Builtin::get_ISWLPC() { return ISWLPC(); }
const ::Opm::ParserKeyword Builtin::get_ISWU() { return ISWU(); }

void Builtin::emplaceI() const {
    this->keywords.emplace("IHOST", IHOST());
    this->keywords.emplace("IMBNUM", IMBNUM());
    this->keywords.emplace("IMBNUMMF", IMBNUMMF());
    this->keywords.emplace("IMKRVD", IMKRVD());
    this->keywords.emplace("IMPCVD", IMPCVD());
    this->keywords.emplace("IMPES", IMPES());
    this->keywords.emplace("IMPLICIT", IMPLICIT());
    this->keywords.emplace("IMPORT", IMPORT());
    this->keywords.emplace("IMPTVD", IMPTVD());
    this->keywords.emplace("IMSPCVD", IMSPCVD());
    this->keywords.emplace("INCLUDE", INCLUDE());
    this->keywords.emplace("INIT", INIT());
    this->keywords.emplace("INRAD", INRAD());
    this->keywords.emplace("INSPEC", INSPEC());
    this->keywords.emplace("INTPC", INTPC());
    this->keywords.emplace("IONROCK", IONROCK());
    this->keywords.emplace("IONXROCK", IONXROCK());
    this->keywords.emplace("IONXSURF", IONXSURF());
    this->keywords.emplace("IPCG", IPCG());
    this->keywords.emplace("IPCW", IPCW());
    this->keywords.emplace("ISGCR", ISGCR());
    this->keywords.emplace("ISGL", ISGL());
    this->keywords.emplace("ISGLPC", ISGLPC());
    this->keywords.emplace("ISGU", ISGU());
    this->keywords.emplace("ISOGCR", ISOGCR());
    this->keywords.emplace("ISOLNUM", ISOLNUM());
    this->keywords.emplace("ISOWCR", ISOWCR());
    this->keywords.emplace("ISWCR", ISWCR());
    this->keywords.emplace("ISWL", ISWL());
    this->keywords.emplace("ISWLPC", ISWLPC());
    this->keywords.emplace("ISWU", ISWU());
}
} }
