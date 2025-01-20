
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitO.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsO([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( OCOMPIDX() );
    p.addParserKeyword( OFM() );
    p.addParserKeyword( OIL() );
    p.addParserKeyword( OILAPI() );
    p.addParserKeyword( OILCOMPR() );
    p.addParserKeyword( OILDENT() );
    p.addParserKeyword( OILJT() );
    p.addParserKeyword( OILMW() );
    p.addParserKeyword( OILVISCT() );
    p.addParserKeyword( OILVTIM() );
    p.addParserKeyword( OLDTRAN() );
    p.addParserKeyword( OLDTRANR() );
    p.addParserKeyword( OPERATE() );
    p.addParserKeyword( OPERATER() );
    p.addParserKeyword( OPERNUM() );
    p.addParserKeyword( OPTIONS() );
    p.addParserKeyword( OPTIONS3() );
    p.addParserKeyword( OUTRAD() );
    p.addParserKeyword( OUTSOL() );
    p.addParserKeyword( OVERBURD() );


}
}
}
