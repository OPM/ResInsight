
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitH.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/H.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsH([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( H2SOL() );
    p.addParserKeyword( H2STORE() );
    p.addParserKeyword( HALFTRAN() );
    p.addParserKeyword( HAxxxxxx() );
    p.addParserKeyword( HBNUM() );
    p.addParserKeyword( HDISP() );
    p.addParserKeyword( HEATCR() );
    p.addParserKeyword( HEATCRT() );
    p.addParserKeyword( HMAQUCT() );
    p.addParserKeyword( HMAQUFET() );
    p.addParserKeyword( HMAQUNUM() );
    p.addParserKeyword( HMDIMS() );
    p.addParserKeyword( HMFAULTS() );
    p.addParserKeyword( HMMLAQUN() );
    p.addParserKeyword( HMMLCTAQ() );
    p.addParserKeyword( HMMLFTAQ() );
    p.addParserKeyword( HMMLTWCN() );
    p.addParserKeyword( HMMULTFT() );
    p.addParserKeyword( HMMULTSG() );
    p.addParserKeyword( HMMULTxx() );
    p.addParserKeyword( HMPROPS() );
    p.addParserKeyword( HMROCK() );
    p.addParserKeyword( HMROCKT() );
    p.addParserKeyword( HMRREF() );
    p.addParserKeyword( HMWELCON() );
    p.addParserKeyword( HMWPIMLT() );
    p.addParserKeyword( HMxxxxxx() );
    p.addParserKeyword( HRFIN() );
    p.addParserKeyword( HWELLS() );
    p.addParserKeyword( HWKRO() );
    p.addParserKeyword( HWKRORG() );
    p.addParserKeyword( HWKRORW() );
    p.addParserKeyword( HWKRW() );
    p.addParserKeyword( HWKRWR() );
    p.addParserKeyword( HWPCW() );
    p.addParserKeyword( HWSNUM() );
    p.addParserKeyword( HWSOGCR() );
    p.addParserKeyword( HWSOWCR() );
    p.addParserKeyword( HWSWCR() );
    p.addParserKeyword( HWSWL() );
    p.addParserKeyword( HWSWLPC() );
    p.addParserKeyword( HWSWU() );
    p.addParserKeyword( HXFIN() );
    p.addParserKeyword( HYDRHEAD() );
    p.addParserKeyword( HYFIN() );
    p.addParserKeyword( HYMOBGDR() );
    p.addParserKeyword( HYST() );
    p.addParserKeyword( HYSTCHCK() );
    p.addParserKeyword( HZFIN() );


}
}
}
