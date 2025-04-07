
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitE.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsE([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( ECHO() );
    p.addParserKeyword( ECLMC() );
    p.addParserKeyword( EDIT() );
    p.addParserKeyword( EDITNNC() );
    p.addParserKeyword( EDITNNCR() );
    p.addParserKeyword( EHYSTR() );
    p.addParserKeyword( EHYSTRR() );
    p.addParserKeyword( END() );
    p.addParserKeyword( ENDACTIO() );
    p.addParserKeyword( ENDBOX() );
    p.addParserKeyword( ENDDYN() );
    p.addParserKeyword( ENDFIN() );
    p.addParserKeyword( ENDINC() );
    p.addParserKeyword( ENDNUM() );
    p.addParserKeyword( ENDPOINT_SPECIFIERS() );
    p.addParserKeyword( ENDSCALE() );
    p.addParserKeyword( ENDSKIP() );
    p.addParserKeyword( ENKRVD() );
    p.addParserKeyword( ENPCVD() );
    p.addParserKeyword( ENPTVD() );
    p.addParserKeyword( ENSPCVD() );
    p.addParserKeyword( EOS() );
    p.addParserKeyword( EOSNUM() );
    p.addParserKeyword( EPSDBGS() );
    p.addParserKeyword( EPSDEBUG() );
    p.addParserKeyword( EQLDIMS() );
    p.addParserKeyword( EQLNUM() );
    p.addParserKeyword( EQLOPTS() );
    p.addParserKeyword( EQLZCORN() );
    p.addParserKeyword( EQUALREG() );
    p.addParserKeyword( EQUALS() );
    p.addParserKeyword( EQUIL() );
    p.addParserKeyword( ESSNODE() );
    p.addParserKeyword( EXCAVATE() );
    p.addParserKeyword( EXCEL() );
    p.addParserKeyword( EXIT() );
    p.addParserKeyword( EXTFIN() );
    p.addParserKeyword( EXTHOST() );
    p.addParserKeyword( EXTRAPMS() );
    p.addParserKeyword( EXTREPGL() );


}
}
}
