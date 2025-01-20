
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitD.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsD([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( DATE() );
    p.addParserKeyword( DATES() );
    p.addParserKeyword( DATUM() );
    p.addParserKeyword( DATUMR() );
    p.addParserKeyword( DATUMRX() );
    p.addParserKeyword( DCQDEFN() );
    p.addParserKeyword( DEBUG_() );
    p.addParserKeyword( DELAYACT() );
    p.addParserKeyword( DENAQA() );
    p.addParserKeyword( DENSITY() );
    p.addParserKeyword( DEPTH() );
    p.addParserKeyword( DEPTHTAB() );
    p.addParserKeyword( DEPTHZ() );
    p.addParserKeyword( DIAGDISP() );
    p.addParserKeyword( DIFF() );
    p.addParserKeyword( DIFFAGAS() );
    p.addParserKeyword( DIFFAWAT() );
    p.addParserKeyword( DIFFC() );
    p.addParserKeyword( DIFFCGAS() );
    p.addParserKeyword( DIFFCOAL() );
    p.addParserKeyword( DIFFCWAT() );
    p.addParserKeyword( DIFFDP() );
    p.addParserKeyword( DIFFMMF() );
    p.addParserKeyword( DIFFMR() );
    p.addParserKeyword( DIFFMTHT() );
    p.addParserKeyword( DIFFMX() );
    p.addParserKeyword( DIFFMY() );
    p.addParserKeyword( DIFFMZ() );
    p.addParserKeyword( DIFFR() );
    p.addParserKeyword( DIFFTHT() );
    p.addParserKeyword( DIFFUSE() );
    p.addParserKeyword( DIFFX() );
    p.addParserKeyword( DIFFY() );
    p.addParserKeyword( DIFFZ() );
    p.addParserKeyword( DIMENS() );
    p.addParserKeyword( DIMPES() );
    p.addParserKeyword( DIMPLICT() );
    p.addParserKeyword( DISGAS() );
    p.addParserKeyword( DISGASW() );
    p.addParserKeyword( DISPDIMS() );
    p.addParserKeyword( DISPERC() );
    p.addParserKeyword( DISPERSE() );
    p.addParserKeyword( DOMAINS() );
    p.addParserKeyword( DPGRID() );
    p.addParserKeyword( DPKRMOD() );
    p.addParserKeyword( DPNUM() );
    p.addParserKeyword( DR() );
    p.addParserKeyword( DREF() );
    p.addParserKeyword( DREFS() );
    p.addParserKeyword( DRILPRI() );
    p.addParserKeyword( DRSDT() );
    p.addParserKeyword( DRSDTCON() );
    p.addParserKeyword( DRSDTR() );
    p.addParserKeyword( DRV() );
    p.addParserKeyword( DRVDT() );
    p.addParserKeyword( DRVDTR() );
    p.addParserKeyword( DSPDEINT() );
    p.addParserKeyword( DTHETA() );
    p.addParserKeyword( DTHETAV() );
    p.addParserKeyword( DUALPERM() );
    p.addParserKeyword( DUALPORO() );
    p.addParserKeyword( DUMPCUPL() );
    p.addParserKeyword( DUMPFLUX() );
    p.addParserKeyword( DX() );
    p.addParserKeyword( DXV() );
    p.addParserKeyword( DY() );
    p.addParserKeyword( DYNAMICR() );
    p.addParserKeyword( DYNRDIMS() );
    p.addParserKeyword( DYV() );
    p.addParserKeyword( DZ() );
    p.addParserKeyword( DZMATRIX() );
    p.addParserKeyword( DZMTRX() );
    p.addParserKeyword( DZMTRXV() );
    p.addParserKeyword( DZNET() );
    p.addParserKeyword( DZV() );


}
}
}
