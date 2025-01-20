
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitK.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/K.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsK([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( KRNUM() );
    p.addParserKeyword( KRNUMMF() );


}
}
}
