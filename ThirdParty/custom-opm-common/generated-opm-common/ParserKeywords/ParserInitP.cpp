
#include <opm/input/eclipse/Parser/Parser.hpp>
#include<opm/input/eclipse/Parser/ParserKeywords/ParserInitP.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>

namespace Opm {
namespace ParserKeywords {
void addDefaultKeywordsP([[maybe_unused]] Parser& p){
    //Builtin keywords;
    p.addParserKeyword( PARALLEL() );
    p.addParserKeyword( PARAOPTS() );
    p.addParserKeyword( PARTTRAC() );
    p.addParserKeyword( PATHS() );
    p.addParserKeyword( PBUB() );
    p.addParserKeyword( PBVD() );
    p.addParserKeyword( PCFACT() );
    p.addParserKeyword( PCG() );
    p.addParserKeyword( PCG32D() );
    p.addParserKeyword( PCRIT() );
    p.addParserKeyword( PCW() );
    p.addParserKeyword( PCW32D() );
    p.addParserKeyword( PDEW() );
    p.addParserKeyword( PDVD() );
    p.addParserKeyword( PEBI() );
    p.addParserKeyword( PECOEFS() );
    p.addParserKeyword( PEDIMS() );
    p.addParserKeyword( PEGTABX() );
    p.addParserKeyword( PEKTABX() );
    p.addParserKeyword( PENUM() );
    p.addParserKeyword( PERFORMANCE_PROBE() );
    p.addParserKeyword( PERMAVE() );
    p.addParserKeyword( PERMFACT() );
    p.addParserKeyword( PERMJFUN() );
    p.addParserKeyword( PERMR() );
    p.addParserKeyword( PERMTHT() );
    p.addParserKeyword( PERMX() );
    p.addParserKeyword( PERMXY() );
    p.addParserKeyword( PERMY() );
    p.addParserKeyword( PERMYZ() );
    p.addParserKeyword( PERMZ() );
    p.addParserKeyword( PERMZX() );
    p.addParserKeyword( PETGRID() );
    p.addParserKeyword( PETOPTS() );
    p.addParserKeyword( PICOND() );
    p.addParserKeyword( PIMTDIMS() );
    p.addParserKeyword( PIMULTAB() );
    p.addParserKeyword( PINCH() );
    p.addParserKeyword( PINCHNUM() );
    p.addParserKeyword( PINCHOUT() );
    p.addParserKeyword( PINCHREG() );
    p.addParserKeyword( PINCHXY() );
    p.addParserKeyword( PINTDIMS() );
    p.addParserKeyword( PLMIXNUM() );
    p.addParserKeyword( PLMIXPAR() );
    p.addParserKeyword( PLYADS() );
    p.addParserKeyword( PLYADSS() );
    p.addParserKeyword( PLYATEMP() );
    p.addParserKeyword( PLYCAMAX() );
    p.addParserKeyword( PLYDHFLF() );
    p.addParserKeyword( PLYESAL() );
    p.addParserKeyword( PLYKRRF() );
    p.addParserKeyword( PLYMAX() );
    p.addParserKeyword( PLYMWINJ() );
    p.addParserKeyword( PLYOPTS() );
    p.addParserKeyword( PLYRMDEN() );
    p.addParserKeyword( PLYROCK() );
    p.addParserKeyword( PLYROCKM() );
    p.addParserKeyword( PLYSHEAR() );
    p.addParserKeyword( PLYSHLOG() );
    p.addParserKeyword( PLYTRRF() );
    p.addParserKeyword( PLYTRRFA() );
    p.addParserKeyword( PLYVISC() );
    p.addParserKeyword( PLYVISCS() );
    p.addParserKeyword( PLYVISCT() );
    p.addParserKeyword( PLYVMH() );
    p.addParserKeyword( PLYVSCST() );
    p.addParserKeyword( PMAX() );
    p.addParserKeyword( PMISC() );
    p.addParserKeyword( POELCOEF() );
    p.addParserKeyword( POLYMER() );
    p.addParserKeyword( POLYMW() );
    p.addParserKeyword( PORO() );
    p.addParserKeyword( PORV() );
    p.addParserKeyword( PPCWMAX() );
    p.addParserKeyword( PRATIO() );
    p.addParserKeyword( PRECSALT() );
    p.addParserKeyword( PREF() );
    p.addParserKeyword( PREFS() );
    p.addParserKeyword( PRESSURE() );
    p.addParserKeyword( PRIORITY() );
    p.addParserKeyword( PROPS() );
    p.addParserKeyword( PRORDER() );
    p.addParserKeyword( PRVD() );
    p.addParserKeyword( PSTEADY() );
    p.addParserKeyword( PSWRG() );
    p.addParserKeyword( PSWRO() );
    p.addParserKeyword( PVCDO() );
    p.addParserKeyword( PVCO() );
    p.addParserKeyword( PVDG() );
    p.addParserKeyword( PVDO() );
    p.addParserKeyword( PVDS() );
    p.addParserKeyword( PVTG() );
    p.addParserKeyword( PVTGW() );
    p.addParserKeyword( PVTGWO() );
    p.addParserKeyword( PVTNUM() );
    p.addParserKeyword( PVTO() );
    p.addParserKeyword( PVTSOL() );
    p.addParserKeyword( PVTW() );
    p.addParserKeyword( PVTWSALT() );
    p.addParserKeyword( PVT_M() );
    p.addParserKeyword( PVZG() );
    p.addParserKeyword( PYACTION() );
    p.addParserKeyword( PYINPUT() );


}
}
}
