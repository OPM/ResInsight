
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitR.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsR([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( RADFIN() );
    p.addParserKeyword( RADFIN4() );
    p.addParserKeyword( RADIAL() );
    p.addParserKeyword( RAINFALL() );
    p.addParserKeyword( RBEDCONT() );
    p.addParserKeyword( RCMASTS() );
    p.addParserKeyword( REACHES() );
    p.addParserKeyword( READDATA() );
    p.addParserKeyword( REFINE() );
    p.addParserKeyword( REGDIMS() );
    p.addParserKeyword( REGION2REGION_PROBE() );
    p.addParserKeyword( REGION2REGION_PROBE_E300() );
    p.addParserKeyword( REGIONS() );
    p.addParserKeyword( REGION_PROBE() );
    p.addParserKeyword( REGION_PROBE_OPM() );
    p.addParserKeyword( RESIDNUM() );
    p.addParserKeyword( RESTART() );
    p.addParserKeyword( RESVNUM() );
    p.addParserKeyword( RHO() );
    p.addParserKeyword( RIVDEBUG() );
    p.addParserKeyword( RIVERSYS() );
    p.addParserKeyword( RIVRDIMS() );
    p.addParserKeyword( RIVRPROP() );
    p.addParserKeyword( RIVRXSEC() );
    p.addParserKeyword( RIVSALT() );
    p.addParserKeyword( RIVTRACE() );
    p.addParserKeyword( RKTRMDIR() );
    p.addParserKeyword( ROCK() );
    p.addParserKeyword( ROCK2D() );
    p.addParserKeyword( ROCK2DTR() );
    p.addParserKeyword( ROCKCOMP() );
    p.addParserKeyword( ROCKFRAC() );
    p.addParserKeyword( ROCKNUM() );
    p.addParserKeyword( ROCKOPTS() );
    p.addParserKeyword( ROCKPAMA() );
    p.addParserKeyword( ROCKTAB() );
    p.addParserKeyword( ROCKTABH() );
    p.addParserKeyword( ROCKTABW() );
    p.addParserKeyword( ROCKTHSG() );
    p.addParserKeyword( ROCKTSIG() );
    p.addParserKeyword( ROCKV() );
    p.addParserKeyword( ROCKWNOD() );
    p.addParserKeyword( RPTCPL() );
    p.addParserKeyword( RPTGRID() );
    p.addParserKeyword( RPTGRIDL() );
    p.addParserKeyword( RPTHMD() );
    p.addParserKeyword( RPTHMG() );
    p.addParserKeyword( RPTHMW() );
    p.addParserKeyword( RPTINIT() );
    p.addParserKeyword( RPTISOL() );
    p.addParserKeyword( RPTONLY() );
    p.addParserKeyword( RPTONLYO() );
    p.addParserKeyword( RPTPROPS() );
    p.addParserKeyword( RPTREGS() );
    p.addParserKeyword( RPTRST() );
    p.addParserKeyword( RPTRUNSP() );
    p.addParserKeyword( RPTSCHED() );
    p.addParserKeyword( RPTSMRY() );
    p.addParserKeyword( RPTSOL() );
    p.addParserKeyword( RS() );
    p.addParserKeyword( RSCONST() );
    p.addParserKeyword( RSCONSTT() );
    p.addParserKeyword( RSGI() );
    p.addParserKeyword( RSSPEC() );
    p.addParserKeyword( RSVD() );
    p.addParserKeyword( RTEMP() );
    p.addParserKeyword( RTEMPA() );
    p.addParserKeyword( RTEMPVD() );
    p.addParserKeyword( RUNSPEC() );
    p.addParserKeyword( RUNSUM() );
    p.addParserKeyword( RV() );
    p.addParserKeyword( RVCONST() );
    p.addParserKeyword( RVCONSTT() );
    p.addParserKeyword( RVGI() );
    p.addParserKeyword( RVVD() );
    p.addParserKeyword( RVW() );
    p.addParserKeyword( RVWVD() );
    p.addParserKeyword( RWGSALT() );


}
}
}
