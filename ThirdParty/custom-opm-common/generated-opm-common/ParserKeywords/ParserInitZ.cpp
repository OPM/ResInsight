
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitZ.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Z.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsZ([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( ZCORN() );
    p.addParserKeyword( ZFACT1() );
    p.addParserKeyword( ZFACT1S() );
    p.addParserKeyword( ZFACTOR() );
    p.addParserKeyword( ZFACTORS() );
    p.addParserKeyword( ZIPP2OFF() );
    p.addParserKeyword( ZIPPY2() );
    p.addParserKeyword( ZMF() );
    p.addParserKeyword( ZMFVD() );


}
}
}
