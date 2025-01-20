
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitM.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsM([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( MAPAXES() );
    p.addParserKeyword( MAPUNITS() );
    p.addParserKeyword( MASSFLOW() );
    p.addParserKeyword( MATCORR() );
    p.addParserKeyword( MAXVALUE() );
    p.addParserKeyword( MECH() );
    p.addParserKeyword( MEMORY() );
    p.addParserKeyword( MESSAGE() );
    p.addParserKeyword( MESSAGES() );
    p.addParserKeyword( MESSOPTS() );
    p.addParserKeyword( MESSSRVC() );
    p.addParserKeyword( METRIC() );
    p.addParserKeyword( MICP() );
    p.addParserKeyword( MICPPARA() );
    p.addParserKeyword( MINNNCT() );
    p.addParserKeyword( MINNPCOL() );
    p.addParserKeyword( MINPORV() );
    p.addParserKeyword( MINPV() );
    p.addParserKeyword( MINPVV() );
    p.addParserKeyword( MINVALUE() );
    p.addParserKeyword( MISC() );
    p.addParserKeyword( MISCIBLE() );
    p.addParserKeyword( MISCNUM() );
    p.addParserKeyword( MLANG() );
    p.addParserKeyword( MLANGSLV() );
    p.addParserKeyword( MONITOR() );
    p.addParserKeyword( MPFANUM() );
    p.addParserKeyword( MPFNNC() );
    p.addParserKeyword( MSFN() );
    p.addParserKeyword( MSGFILE() );
    p.addParserKeyword( MSUM_PROBE() );
    p.addParserKeyword( MULSGGD() );
    p.addParserKeyword( MULSGGDV() );
    p.addParserKeyword( MULTFLT() );
    p.addParserKeyword( MULTIN() );
    p.addParserKeyword( MULTIPLY() );
    p.addParserKeyword( MULTIREG() );
    p.addParserKeyword( MULTNUM() );
    p.addParserKeyword( MULTOUT() );
    p.addParserKeyword( MULTOUTS() );
    p.addParserKeyword( MULTPV() );
    p.addParserKeyword( MULTREAL() );
    p.addParserKeyword( MULTREGD() );
    p.addParserKeyword( MULTREGH() );
    p.addParserKeyword( MULTREGP() );
    p.addParserKeyword( MULTREGT() );
    p.addParserKeyword( MULTSIG() );
    p.addParserKeyword( MULTSIGV() );
    p.addParserKeyword( MULT_XYZ() );
    p.addParserKeyword( MW() );
    p.addParserKeyword( MWS() );


}
}
}
