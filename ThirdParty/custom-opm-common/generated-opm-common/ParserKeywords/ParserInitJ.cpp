
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitJ.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/J.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsJ([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( JFUNC() );
    p.addParserKeyword( JFUNCR() );


}
}
}
