
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitL.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/L.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsL([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( LAB() );
    p.addParserKeyword( LANGMPL() );
    p.addParserKeyword( LANGMUIR() );
    p.addParserKeyword( LANGSOLV() );
    p.addParserKeyword( LCUNIT() );
    p.addParserKeyword( LGR() );
    p.addParserKeyword( LGRCOPY() );
    p.addParserKeyword( LGRFREE() );
    p.addParserKeyword( LGRLOCK() );
    p.addParserKeyword( LGROFF() );
    p.addParserKeyword( LGRON() );
    p.addParserKeyword( LICENSES() );
    p.addParserKeyword( LIFTOPT() );
    p.addParserKeyword( LINCOM() );
    p.addParserKeyword( LINKPERM() );
    p.addParserKeyword( LIVEOIL() );
    p.addParserKeyword( LKRO() );
    p.addParserKeyword( LKRORG() );
    p.addParserKeyword( LKRORW() );
    p.addParserKeyword( LKRW() );
    p.addParserKeyword( LKRWR() );
    p.addParserKeyword( LOAD() );
    p.addParserKeyword( LOWSALT() );
    p.addParserKeyword( LPCW() );
    p.addParserKeyword( LSALTFNC() );
    p.addParserKeyword( LSLTWNUM() );
    p.addParserKeyword( LSNUM() );
    p.addParserKeyword( LSOGCR() );
    p.addParserKeyword( LSOWCR() );
    p.addParserKeyword( LSWCR() );
    p.addParserKeyword( LSWL() );
    p.addParserKeyword( LSWLPC() );
    p.addParserKeyword( LSWU() );
    p.addParserKeyword( LTOSIGMA() );
    p.addParserKeyword( LWKRO() );
    p.addParserKeyword( LWKRORG() );
    p.addParserKeyword( LWKRORW() );
    p.addParserKeyword( LWKRW() );
    p.addParserKeyword( LWKRWR() );
    p.addParserKeyword( LWPCW() );
    p.addParserKeyword( LWSLTNUM() );
    p.addParserKeyword( LWSNUM() );
    p.addParserKeyword( LWSOGCR() );
    p.addParserKeyword( LWSOWCR() );
    p.addParserKeyword( LWSWCR() );
    p.addParserKeyword( LWSWL() );
    p.addParserKeyword( LWSWLPC() );
    p.addParserKeyword( LWSWU() );
    p.addParserKeyword( LX() );
    p.addParserKeyword( LXFIN() );
    p.addParserKeyword( LY() );
    p.addParserKeyword( LYFIN() );
    p.addParserKeyword( LZ() );
    p.addParserKeyword( LZFIN() );


}
}
}
