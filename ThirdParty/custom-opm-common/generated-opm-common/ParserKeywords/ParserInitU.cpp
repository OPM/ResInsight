
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitU.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/U.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsU([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( UDADIMS() );
    p.addParserKeyword( UDQ() );
    p.addParserKeyword( UDQDIMS() );
    p.addParserKeyword( UDQPARAM() );
    p.addParserKeyword( UDT() );
    p.addParserKeyword( UDTDIMS() );
    p.addParserKeyword( UNCODHMD() );
    p.addParserKeyword( UNIFIN() );
    p.addParserKeyword( UNIFOUT() );
    p.addParserKeyword( UNIFOUTS() );
    p.addParserKeyword( UNIFSAVE() );
    p.addParserKeyword( USECUPL() );
    p.addParserKeyword( USEFLUX() );
    p.addParserKeyword( USENOFLO() );


}
}
}
