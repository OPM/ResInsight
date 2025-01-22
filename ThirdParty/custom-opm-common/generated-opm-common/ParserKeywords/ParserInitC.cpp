
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitC.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsC([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( CALTRAC() );
    p.addParserKeyword( CARFIN() );
    p.addParserKeyword( CART() );
    p.addParserKeyword( CBMOPTS() );
    p.addParserKeyword( CECON() );
    p.addParserKeyword( CECONT() );
    p.addParserKeyword( CIRCLE() );
    p.addParserKeyword( CNAMES() );
    p.addParserKeyword( CO2SOL() );
    p.addParserKeyword( CO2STOR() );
    p.addParserKeyword( CO2STORE() );
    p.addParserKeyword( COAL() );
    p.addParserKeyword( COALADS() );
    p.addParserKeyword( COALNUM() );
    p.addParserKeyword( COALPP() );
    p.addParserKeyword( COARSEN() );
    p.addParserKeyword( COLLAPSE() );
    p.addParserKeyword( COLUMNS() );
    p.addParserKeyword( COMPDAT() );
    p.addParserKeyword( COMPDATX() );
    p.addParserKeyword( COMPFLSH() );
    p.addParserKeyword( COMPIMB() );
    p.addParserKeyword( COMPINJK() );
    p.addParserKeyword( COMPLMPL() );
    p.addParserKeyword( COMPLUMP() );
    p.addParserKeyword( COMPOFF() );
    p.addParserKeyword( COMPORD() );
    p.addParserKeyword( COMPRIV() );
    p.addParserKeyword( COMPRP() );
    p.addParserKeyword( COMPRPL() );
    p.addParserKeyword( COMPS() );
    p.addParserKeyword( COMPSEGL() );
    p.addParserKeyword( COMPSEGS() );
    p.addParserKeyword( COMPTRAJ() );
    p.addParserKeyword( COMPVE() );
    p.addParserKeyword( COMPVEL() );
    p.addParserKeyword( CONNECTION_PROBE() );
    p.addParserKeyword( CONNECTION_PROBE_OPM() );
    p.addParserKeyword( COORD() );
    p.addParserKeyword( COORDSYS() );
    p.addParserKeyword( COPY() );
    p.addParserKeyword( COPYBOX() );
    p.addParserKeyword( COPYREG() );
    p.addParserKeyword( CPIFACT() );
    p.addParserKeyword( CPIFACTL() );
    p.addParserKeyword( CPR() );
    p.addParserKeyword( CREF() );
    p.addParserKeyword( CREFW() );
    p.addParserKeyword( CREFWS() );
    p.addParserKeyword( CRITPERM() );
    p.addParserKeyword( CSKIN() );


}
}
}
