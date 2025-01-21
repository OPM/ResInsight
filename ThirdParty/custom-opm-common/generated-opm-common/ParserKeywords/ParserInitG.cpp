
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitG.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsG([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( GAS() );
    p.addParserKeyword( GASBEGIN() );
    p.addParserKeyword( GASCONC() );
    p.addParserKeyword( GASDENT() );
    p.addParserKeyword( GASEND() );
    p.addParserKeyword( GASFCOMP() );
    p.addParserKeyword( GASFDECR() );
    p.addParserKeyword( GASFDELC() );
    p.addParserKeyword( GASFIELD() );
    p.addParserKeyword( GASFTARG() );
    p.addParserKeyword( GASJT() );
    p.addParserKeyword( GASMONTH() );
    p.addParserKeyword( GASPERIO() );
    p.addParserKeyword( GASSATC() );
    p.addParserKeyword( GASVISCT() );
    p.addParserKeyword( GASWAT() );
    p.addParserKeyword( GASYEAR() );
    p.addParserKeyword( GCALECON() );
    p.addParserKeyword( GCOMPIDX() );
    p.addParserKeyword( GCONCAL() );
    p.addParserKeyword( GCONENG() );
    p.addParserKeyword( GCONINJE() );
    p.addParserKeyword( GCONPRI() );
    p.addParserKeyword( GCONPROD() );
    p.addParserKeyword( GCONSALE() );
    p.addParserKeyword( GCONSUMP() );
    p.addParserKeyword( GCONTOL() );
    p.addParserKeyword( GCUTBACK() );
    p.addParserKeyword( GCUTBACT() );
    p.addParserKeyword( GCVD() );
    p.addParserKeyword( GDCQ() );
    p.addParserKeyword( GDCQECON() );
    p.addParserKeyword( GDFILE() );
    p.addParserKeyword( GDIMS() );
    p.addParserKeyword( GDORIENT() );
    p.addParserKeyword( GDRILPOT() );
    p.addParserKeyword( GECON() );
    p.addParserKeyword( GECONT() );
    p.addParserKeyword( GEFAC() );
    p.addParserKeyword( GETDATA() );
    p.addParserKeyword( GETGLOB() );
    p.addParserKeyword( GI() );
    p.addParserKeyword( GIALL() );
    p.addParserKeyword( GIMODEL() );
    p.addParserKeyword( GINODE() );
    p.addParserKeyword( GLIFTLIM() );
    p.addParserKeyword( GLIFTOPT() );
    p.addParserKeyword( GMWSET() );
    p.addParserKeyword( GNETDP() );
    p.addParserKeyword( GNETINJE() );
    p.addParserKeyword( GNETPUMP() );
    p.addParserKeyword( GPMAINT() );
    p.addParserKeyword( GRADGRUP() );
    p.addParserKeyword( GRADRESV() );
    p.addParserKeyword( GRADRFT() );
    p.addParserKeyword( GRADWELL() );
    p.addParserKeyword( GRAVCONS() );
    p.addParserKeyword( GRAVDR() );
    p.addParserKeyword( GRAVDRB() );
    p.addParserKeyword( GRAVDRM() );
    p.addParserKeyword( GRAVITY() );
    p.addParserKeyword( GRDREACH() );
    p.addParserKeyword( GRID() );
    p.addParserKeyword( GRIDFILE() );
    p.addParserKeyword( GRIDOPTS() );
    p.addParserKeyword( GRIDUNIT() );
    p.addParserKeyword( GROUP_PROBE() );
    p.addParserKeyword( GROUP_PROBE_OPM() );
    p.addParserKeyword( GRUPMAST() );
    p.addParserKeyword( GRUPNET() );
    p.addParserKeyword( GRUPRIG() );
    p.addParserKeyword( GRUPSLAV() );
    p.addParserKeyword( GRUPTARG() );
    p.addParserKeyword( GRUPTREE() );
    p.addParserKeyword( GSATINJE() );
    p.addParserKeyword( GSATPROD() );
    p.addParserKeyword( GSEPCOND() );
    p.addParserKeyword( GSF() );
    p.addParserKeyword( GSSCPTST() );
    p.addParserKeyword( GSWINGF() );
    p.addParserKeyword( GTADD() );
    p.addParserKeyword( GTMULT() );
    p.addParserKeyword( GUIDECAL() );
    p.addParserKeyword( GUIDERAT() );
    p.addParserKeyword( GUPFREQ() );
    p.addParserKeyword( GWRTWCV() );


}
}
}
