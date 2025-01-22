
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitF.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/F.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsF([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( FAULTDIM() );
    p.addParserKeyword( FAULTS() );
    p.addParserKeyword( FBHPDEF() );
    p.addParserKeyword( FHERCHBL() );
    p.addParserKeyword( FIELD() );
    p.addParserKeyword( FIELDSEP() );
    p.addParserKeyword( FIELD_PROBE() );
    p.addParserKeyword( FIELD_PROBE_OPM() );
    p.addParserKeyword( FILEUNIT() );
    p.addParserKeyword( FILLEPS() );
    p.addParserKeyword( FIPNUM() );
    p.addParserKeyword( FIPOWG() );
    p.addParserKeyword( FIPSEP() );
    p.addParserKeyword( FIP_PROBE() );
    p.addParserKeyword( FLUXNUM() );
    p.addParserKeyword( FLUXREG() );
    p.addParserKeyword( FLUXTYPE() );
    p.addParserKeyword( FMTHMD() );
    p.addParserKeyword( FMTIN() );
    p.addParserKeyword( FMTOUT() );
    p.addParserKeyword( FMWSET() );
    p.addParserKeyword( FOAM() );
    p.addParserKeyword( FOAMADS() );
    p.addParserKeyword( FOAMDCYO() );
    p.addParserKeyword( FOAMDCYW() );
    p.addParserKeyword( FOAMFCN() );
    p.addParserKeyword( FOAMFRM() );
    p.addParserKeyword( FOAMFSC() );
    p.addParserKeyword( FOAMFSO() );
    p.addParserKeyword( FOAMFST() );
    p.addParserKeyword( FOAMFSW() );
    p.addParserKeyword( FOAMMOB() );
    p.addParserKeyword( FOAMMOBP() );
    p.addParserKeyword( FOAMMOBS() );
    p.addParserKeyword( FOAMOPTS() );
    p.addParserKeyword( FOAMROCK() );
    p.addParserKeyword( FORMFEED() );
    p.addParserKeyword( FRICTION() );
    p.addParserKeyword( FULLIMP() );


}
}
}
