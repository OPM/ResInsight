
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitT.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsT([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( TABDIMS() );
    p.addParserKeyword( TBLK() );
    p.addParserKeyword( TCRIT() );
    p.addParserKeyword( TEMP() );
    p.addParserKeyword( TEMPI() );
    p.addParserKeyword( TEMPNODE() );
    p.addParserKeyword( TEMPTVD() );
    p.addParserKeyword( TEMPVD() );
    p.addParserKeyword( THCGAS() );
    p.addParserKeyword( THCO2MIX() );
    p.addParserKeyword( THCOIL() );
    p.addParserKeyword( THCONR() );
    p.addParserKeyword( THCONSF() );
    p.addParserKeyword( THCROCK() );
    p.addParserKeyword( THCWATER() );
    p.addParserKeyword( THELCOEF() );
    p.addParserKeyword( THERMAL() );
    p.addParserKeyword( THERMEXR() );
    p.addParserKeyword( THPRES() );
    p.addParserKeyword( THPRESFT() );
    p.addParserKeyword( TIGHTEN() );
    p.addParserKeyword( TIGHTENP() );
    p.addParserKeyword( TIME() );
    p.addParserKeyword( TITLE() );
    p.addParserKeyword( TLMIXPAR() );
    p.addParserKeyword( TLPMIXPA() );
    p.addParserKeyword( TNUM() );
    p.addParserKeyword( TOLCRIT() );
    p.addParserKeyword( TOPS() );
    p.addParserKeyword( TPAMEPS() );
    p.addParserKeyword( TPAMEPSS() );
    p.addParserKeyword( TRACER() );
    p.addParserKeyword( TRACERKM() );
    p.addParserKeyword( TRACERKP() );
    p.addParserKeyword( TRACERS() );
    p.addParserKeyword( TRACITVD() );
    p.addParserKeyword( TRACTVD() );
    p.addParserKeyword( TRADS() );
    p.addParserKeyword( TRANGL() );
    p.addParserKeyword( TRANR() );
    p.addParserKeyword( TRANTHT() );
    p.addParserKeyword( TRANX() );
    p.addParserKeyword( TRANY() );
    p.addParserKeyword( TRANZ() );
    p.addParserKeyword( TRDCY() );
    p.addParserKeyword( TRDIF() );
    p.addParserKeyword( TRDIS() );
    p.addParserKeyword( TREF() );
    p.addParserKeyword( TREFS() );
    p.addParserKeyword( TRKPF() );
    p.addParserKeyword( TRNHD() );
    p.addParserKeyword( TRPLPORO() );
    p.addParserKeyword( TRROCK() );
    p.addParserKeyword( TSTEP() );
    p.addParserKeyword( TUNING() );
    p.addParserKeyword( TUNINGDP() );
    p.addParserKeyword( TUNINGH() );
    p.addParserKeyword( TUNINGL() );
    p.addParserKeyword( TUNINGS() );
    p.addParserKeyword( TVDP() );
    p.addParserKeyword( TZONE() );


}
}
}
