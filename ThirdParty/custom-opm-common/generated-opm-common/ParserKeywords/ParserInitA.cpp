
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitA.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsA([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( ACF() );
    p.addParserKeyword( ACTCO2S() );
    p.addParserKeyword( ACTDIMS() );
    p.addParserKeyword( ACTION() );
    p.addParserKeyword( ACTIONG() );
    p.addParserKeyword( ACTIONR() );
    p.addParserKeyword( ACTIONS() );
    p.addParserKeyword( ACTIONW() );
    p.addParserKeyword( ACTIONX() );
    p.addParserKeyword( ACTNUM() );
    p.addParserKeyword( ACTPARAM() );
    p.addParserKeyword( ADD() );
    p.addParserKeyword( ADDREG() );
    p.addParserKeyword( ADDZCORN() );
    p.addParserKeyword( ADSALNOD() );
    p.addParserKeyword( ADSORP() );
    p.addParserKeyword( AITS() );
    p.addParserKeyword( AITSOFF() );
    p.addParserKeyword( ALKADS() );
    p.addParserKeyword( ALKALINE() );
    p.addParserKeyword( ALKROCK() );
    p.addParserKeyword( ALL() );
    p.addParserKeyword( ALPOLADS() );
    p.addParserKeyword( ALSURFAD() );
    p.addParserKeyword( ALSURFST() );
    p.addParserKeyword( AMALGAM() );
    p.addParserKeyword( API() );
    p.addParserKeyword( APIGROUP() );
    p.addParserKeyword( APILIM() );
    p.addParserKeyword( APIVD() );
    p.addParserKeyword( AQANCONL() );
    p.addParserKeyword( AQANNC() );
    p.addParserKeyword( AQANTRC() );
    p.addParserKeyword( AQUALIST() );
    p.addParserKeyword( AQUANCON() );
    p.addParserKeyword( AQUCHGAS() );
    p.addParserKeyword( AQUCHWAT() );
    p.addParserKeyword( AQUCON() );
    p.addParserKeyword( AQUCT() );
    p.addParserKeyword( AQUCWFAC() );
    p.addParserKeyword( AQUDIMS() );
    p.addParserKeyword( AQUFET() );
    p.addParserKeyword( AQUFETP() );
    p.addParserKeyword( AQUFLUX() );
    p.addParserKeyword( AQUIFER_PROBE_ANALYTIC() );
    p.addParserKeyword( AQUIFER_PROBE_ANALYTIC_NAMED() );
    p.addParserKeyword( AQUIFER_PROBE_NUMERIC() );
    p.addParserKeyword( AQUNNC() );
    p.addParserKeyword( AQUNUM() );
    p.addParserKeyword( AQUTAB() );
    p.addParserKeyword( AUTOCOAR() );
    p.addParserKeyword( AUTOREF() );


}
}
}
