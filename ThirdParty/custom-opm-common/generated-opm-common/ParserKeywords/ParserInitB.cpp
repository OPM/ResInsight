
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitB.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsB([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( BC() );
    p.addParserKeyword( BCCON() );
    p.addParserKeyword( BCPROP() );
    p.addParserKeyword( BDENSITY() );
    p.addParserKeyword( BGGI() );
    p.addParserKeyword( BIC() );
    p.addParserKeyword( BIGMODEL() );
    p.addParserKeyword( BIOTCOEF() );
    p.addParserKeyword( BLACKOIL() );
    p.addParserKeyword( BLOCK_PROBE() );
    p.addParserKeyword( BLOCK_PROBE300() );
    p.addParserKeyword( BOGI() );
    p.addParserKeyword( BOUNDARY() );
    p.addParserKeyword( BOX() );
    p.addParserKeyword( BPARA() );
    p.addParserKeyword( BPIDIMS() );
    p.addParserKeyword( BRANPROP() );
    p.addParserKeyword( BRINE() );
    p.addParserKeyword( BTOBALFA() );
    p.addParserKeyword( BTOBALFV() );


}
}
}
