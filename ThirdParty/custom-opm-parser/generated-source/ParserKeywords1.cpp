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


void addDefaultKeywords1(Parser& p) {
p.addKeyword< ParserKeywords::GRIDOPTS >();
p.addKeyword< ParserKeywords::GRIDUNIT >();
p.addKeyword< ParserKeywords::GROUP_PROBE >();
p.addKeyword< ParserKeywords::GRUPNET >();
p.addKeyword< ParserKeywords::GRUPTREE >();
p.addKeyword< ParserKeywords::IMBNUM >();
p.addKeyword< ParserKeywords::IMKRVD >();
p.addKeyword< ParserKeywords::IMPES >();
p.addKeyword< ParserKeywords::IMPTVD >();
p.addKeyword< ParserKeywords::INCLUDE >();
p.addKeyword< ParserKeywords::INIT >();
p.addKeyword< ParserKeywords::IPCG >();
p.addKeyword< ParserKeywords::IPCW >();
p.addKeyword< ParserKeywords::ISGCR >();
p.addKeyword< ParserKeywords::ISGL >();
p.addKeyword< ParserKeywords::ISGU >();
p.addKeyword< ParserKeywords::ISOGCR >();
p.addKeyword< ParserKeywords::ISOWCR >();
p.addKeyword< ParserKeywords::ISWCR >();
p.addKeyword< ParserKeywords::ISWL >();
p.addKeyword< ParserKeywords::ISWU >();
p.addKeyword< ParserKeywords::MAPAXES >();
p.addKeyword< ParserKeywords::MAPUNITS >();
p.addKeyword< ParserKeywords::MAXVALUE >();
p.addKeyword< ParserKeywords::MEMORY >();
p.addKeyword< ParserKeywords::MESSAGES >();
p.addKeyword< ParserKeywords::METRIC >();
p.addKeyword< ParserKeywords::MINPV >();
p.addKeyword< ParserKeywords::MINPVFIL >();
p.addKeyword< ParserKeywords::MINVALUE >();
p.addKeyword< ParserKeywords::MISC >();
p.addKeyword< ParserKeywords::MISCIBLE >();
p.addKeyword< ParserKeywords::MISCNUM >();
p.addKeyword< ParserKeywords::MONITOR >();
p.addKeyword< ParserKeywords::MSFN >();
p.addKeyword< ParserKeywords::MSGFILE >();
p.addKeyword< ParserKeywords::MULTFLT >();
p.addKeyword< ParserKeywords::MULTIPLY >();
p.addKeyword< ParserKeywords::MULTIREG >();
p.addKeyword< ParserKeywords::MULTNUM >();
p.addKeyword< ParserKeywords::MULTPV >();
p.addKeyword< ParserKeywords::MULTREGT >();
p.addKeyword< ParserKeywords::MULT_XYZ >();
p.addKeyword< ParserKeywords::MW >();
p.addKeyword< ParserKeywords::MWS >();
p.addKeyword< ParserKeywords::NETBALAN >();
p.addKeyword< ParserKeywords::NEWTRAN >();
p.addKeyword< ParserKeywords::NEXTSTEP >();
p.addKeyword< ParserKeywords::NNC >();
p.addKeyword< ParserKeywords::NOCASC >();
p.addKeyword< ParserKeywords::NOECHO >();
p.addKeyword< ParserKeywords::NOGGF >();
p.addKeyword< ParserKeywords::NOGRAV >();
p.addKeyword< ParserKeywords::NOINSPEC >();
p.addKeyword< ParserKeywords::NOMONITO >();
p.addKeyword< ParserKeywords::NONNC >();
p.addKeyword< ParserKeywords::NORSSPEC >();
p.addKeyword< ParserKeywords::NOSIM >();
p.addKeyword< ParserKeywords::NSTACK >();
p.addKeyword< ParserKeywords::NTG >();
p.addKeyword< ParserKeywords::NUMRES >();
p.addKeyword< ParserKeywords::NUPCOL >();
p.addKeyword< ParserKeywords::OCOMPIDX >();
p.addKeyword< ParserKeywords::OIL >();
p.addKeyword< ParserKeywords::OILCOMPR >();
p.addKeyword< ParserKeywords::OILMW >();
p.addKeyword< ParserKeywords::OILVISCT >();
p.addKeyword< ParserKeywords::OILVTIM >();
p.addKeyword< ParserKeywords::OLDTRAN >();
p.addKeyword< ParserKeywords::OPTIONS >();
p.addKeyword< ParserKeywords::PARALLEL >();
p.addKeyword< ParserKeywords::PATHS >();
p.addKeyword< ParserKeywords::PBVD >();
p.addKeyword< ParserKeywords::PCG >();
p.addKeyword< ParserKeywords::PERFORMANCE_PROBE >();
p.addKeyword< ParserKeywords::PERMX >();
p.addKeyword< ParserKeywords::PERMXY >();
p.addKeyword< ParserKeywords::PERMY >();
p.addKeyword< ParserKeywords::PERMYZ >();
p.addKeyword< ParserKeywords::PERMZ >();
p.addKeyword< ParserKeywords::PERMZX >();
p.addKeyword< ParserKeywords::PIMTDIMS >();
p.addKeyword< ParserKeywords::PIMULTAB >();
}}}
