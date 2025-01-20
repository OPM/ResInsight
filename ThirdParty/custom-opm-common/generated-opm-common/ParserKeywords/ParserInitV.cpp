
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitV.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsV([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( VAPOIL() );
    p.addParserKeyword( VAPPARS() );
    p.addParserKeyword( VAPWAT() );
    p.addParserKeyword( VCRIT() );
    p.addParserKeyword( VDFLOW() );
    p.addParserKeyword( VDFLOWR() );
    p.addParserKeyword( VE() );
    p.addParserKeyword( VEDEBUG() );
    p.addParserKeyword( VEFIN() );
    p.addParserKeyword( VEFRAC() );
    p.addParserKeyword( VEFRACP() );
    p.addParserKeyword( VEFRACPV() );
    p.addParserKeyword( VEFRACV() );
    p.addParserKeyword( VFPCHK() );
    p.addParserKeyword( VFPIDIMS() );
    p.addParserKeyword( VFPINJ() );
    p.addParserKeyword( VFPPDIMS() );
    p.addParserKeyword( VFPPROD() );
    p.addParserKeyword( VFPTABL() );
    p.addParserKeyword( VISAGE() );
    p.addParserKeyword( VISCAQA() );
    p.addParserKeyword( VISCD() );
    p.addParserKeyword( VISCREF() );
    p.addParserKeyword( VISDATES() );
    p.addParserKeyword( VISOPTS() );


}
}
}
