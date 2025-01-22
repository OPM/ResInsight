
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitN.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsN([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( NARROW() );
    p.addParserKeyword( NCOMPS() );
    p.addParserKeyword( NCONSUMP() );
    p.addParserKeyword( NEFAC() );
    p.addParserKeyword( NETBALAN() );
    p.addParserKeyword( NETCOMPA() );
    p.addParserKeyword( NETWORK() );
    p.addParserKeyword( NETWORK_PROBE() );
    p.addParserKeyword( NEWTRAN() );
    p.addParserKeyword( NEXTSTEP() );
    p.addParserKeyword( NEXTSTPL() );
    p.addParserKeyword( NINENUM() );
    p.addParserKeyword( NINEPOIN() );
    p.addParserKeyword( NMATOPTS() );
    p.addParserKeyword( NMATRIX() );
    p.addParserKeyword( NMESSAGE() );
    p.addParserKeyword( NNC() );
    p.addParserKeyword( NNEWTF() );
    p.addParserKeyword( NOCASC() );
    p.addParserKeyword( NODEPROP() );
    p.addParserKeyword( NODPPM() );
    p.addParserKeyword( NOECHO() );
    p.addParserKeyword( NOGGF() );
    p.addParserKeyword( NOGRAV() );
    p.addParserKeyword( NOHMD() );
    p.addParserKeyword( NOHMO() );
    p.addParserKeyword( NOHYST() );
    p.addParserKeyword( NOINSPEC() );
    p.addParserKeyword( NOMONITO() );
    p.addParserKeyword( NONNC() );
    p.addParserKeyword( NORSSPEC() );
    p.addParserKeyword( NOSIM() );
    p.addParserKeyword( NOWARN() );
    p.addParserKeyword( NOWARNEP() );
    p.addParserKeyword( NRSOUT() );
    p.addParserKeyword( NSTACK() );
    p.addParserKeyword( NTG() );
    p.addParserKeyword( NUMRES() );
    p.addParserKeyword( NUPCOL() );
    p.addParserKeyword( NWATREM() );
    p.addParserKeyword( NXFIN() );
    p.addParserKeyword( NYFIN() );
    p.addParserKeyword( NZFIN() );


}
}
}
