
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitQ.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Q.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsQ([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( QDRILL() );
    p.addParserKeyword( QHRATING() );
    p.addParserKeyword( QMOBIL() );


}
}
}
