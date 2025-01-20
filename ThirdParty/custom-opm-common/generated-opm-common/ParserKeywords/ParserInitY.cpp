
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitY.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Y.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsY([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( YMF() );
    p.addParserKeyword( YMODULE() );


}
}
}
