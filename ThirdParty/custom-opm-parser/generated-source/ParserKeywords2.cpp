#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>


namespace Opm {
namespace ParserKeywords {


void addDefaultKeywords2(Parser& p) {
p.addKeyword< ParserKeywords::PINCH >();
p.addKeyword< ParserKeywords::PLMIXPAR >();
p.addKeyword< ParserKeywords::PLYADS >();
p.addKeyword< ParserKeywords::PLYADSS >();
p.addKeyword< ParserKeywords::PLYDHFLF >();
p.addKeyword< ParserKeywords::PLYMAX >();
p.addKeyword< ParserKeywords::PLYROCK >();
p.addKeyword< ParserKeywords::PLYSHEAR >();
p.addKeyword< ParserKeywords::PLYSHLOG >();
p.addKeyword< ParserKeywords::PLYVISC >();
p.addKeyword< ParserKeywords::PMISC >();
p.addKeyword< ParserKeywords::POLYMER >();
p.addKeyword< ParserKeywords::PORO >();
p.addKeyword< ParserKeywords::PORV >();
p.addKeyword< ParserKeywords::PREF >();
p.addKeyword< ParserKeywords::PREFS >();
p.addKeyword< ParserKeywords::PRESSURE >();
p.addKeyword< ParserKeywords::PROPS >();
p.addKeyword< ParserKeywords::PVCDO >();
p.addKeyword< ParserKeywords::PVDG >();
p.addKeyword< ParserKeywords::PVDO >();
p.addKeyword< ParserKeywords::PVDS >();
p.addKeyword< ParserKeywords::PVTG >();
p.addKeyword< ParserKeywords::PVTNUM >();
p.addKeyword< ParserKeywords::PVTO >();
p.addKeyword< ParserKeywords::PVTW >();
p.addKeyword< ParserKeywords::RADFIN4 >();
p.addKeyword< ParserKeywords::REGDIMS >();
p.addKeyword< ParserKeywords::REGIONS >();
p.addKeyword< ParserKeywords::REGION_PROBE >();
p.addKeyword< ParserKeywords::RESTART >();
p.addKeyword< ParserKeywords::RKTRMDIR >();
p.addKeyword< ParserKeywords::ROCK >();
p.addKeyword< ParserKeywords::ROCKCOMP >();
p.addKeyword< ParserKeywords::ROCKOPTS >();
p.addKeyword< ParserKeywords::ROCKTAB >();
p.addKeyword< ParserKeywords::RPTGRID >();
p.addKeyword< ParserKeywords::RPTONLY >();
p.addKeyword< ParserKeywords::RPTONLYO >();
p.addKeyword< ParserKeywords::RPTPROPS >();
p.addKeyword< ParserKeywords::RPTRST >();
p.addKeyword< ParserKeywords::RPTRUNSP >();
p.addKeyword< ParserKeywords::RPTSCHED >();
p.addKeyword< ParserKeywords::RPTSOL >();
p.addKeyword< ParserKeywords::RS >();
p.addKeyword< ParserKeywords::RSVD >();
p.addKeyword< ParserKeywords::RTEMPVD >();
p.addKeyword< ParserKeywords::RUNSPEC >();
p.addKeyword< ParserKeywords::RUNSUM >();
p.addKeyword< ParserKeywords::RV >();
p.addKeyword< ParserKeywords::RVVD >();
p.addKeyword< ParserKeywords::SATNUM >();
p.addKeyword< ParserKeywords::SATOPTS >();
p.addKeyword< ParserKeywords::SAVE >();
p.addKeyword< ParserKeywords::SCALECRS >();
p.addKeyword< ParserKeywords::SCHEDULE >();
p.addKeyword< ParserKeywords::SDENSITY >();
p.addKeyword< ParserKeywords::SEPARATE >();
p.addKeyword< ParserKeywords::SGAS >();
p.addKeyword< ParserKeywords::SGCR >();
p.addKeyword< ParserKeywords::SGCWMIS >();
p.addKeyword< ParserKeywords::SGFN >();
p.addKeyword< ParserKeywords::SGL >();
p.addKeyword< ParserKeywords::SGOF >();
p.addKeyword< ParserKeywords::SGU >();
p.addKeyword< ParserKeywords::SGWFN >();
p.addKeyword< ParserKeywords::SHRATE >();
p.addKeyword< ParserKeywords::SKIP >();
p.addKeyword< ParserKeywords::SKIP100 >();
p.addKeyword< ParserKeywords::SKIP300 >();
p.addKeyword< ParserKeywords::SKIPREST >();
p.addKeyword< ParserKeywords::SLGOF >();
p.addKeyword< ParserKeywords::SMRYDIMS >();
p.addKeyword< ParserKeywords::SOF2 >();
p.addKeyword< ParserKeywords::SOF3 >();
p.addKeyword< ParserKeywords::SOGCR >();
p.addKeyword< ParserKeywords::SOIL >();
p.addKeyword< ParserKeywords::SOLUTION >();
p.addKeyword< ParserKeywords::SOLVENT >();
p.addKeyword< ParserKeywords::SORWMIS >();
p.addKeyword< ParserKeywords::SOWCR >();
p.addKeyword< ParserKeywords::SPECGRID >();
p.addKeyword< ParserKeywords::SPECHEAT >();
}}}
