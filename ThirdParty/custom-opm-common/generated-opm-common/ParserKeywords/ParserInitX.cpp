
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitX.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/X.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsX([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( XMF() );


}
}
}
