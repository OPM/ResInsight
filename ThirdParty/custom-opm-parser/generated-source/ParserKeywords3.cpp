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


void addDefaultKeywords3(Parser& p) {
p.addKeyword< ParserKeywords::SSOL >();
p.addKeyword< ParserKeywords::START >();
p.addKeyword< ParserKeywords::STCOND >();
p.addKeyword< ParserKeywords::STONE1 >();
p.addKeyword< ParserKeywords::STONE1EX >();
p.addKeyword< ParserKeywords::SUMMARY >();
p.addKeyword< ParserKeywords::SUMTHIN >();
p.addKeyword< ParserKeywords::SWAT >();
p.addKeyword< ParserKeywords::SWATINIT >();
p.addKeyword< ParserKeywords::SWCR >();
p.addKeyword< ParserKeywords::SWFN >();
p.addKeyword< ParserKeywords::SWL >();
p.addKeyword< ParserKeywords::SWOF >();
p.addKeyword< ParserKeywords::SWU >();
p.addKeyword< ParserKeywords::TABDIMS >();
p.addKeyword< ParserKeywords::TEMP >();
p.addKeyword< ParserKeywords::TEMPI >();
p.addKeyword< ParserKeywords::TEMPVD >();
p.addKeyword< ParserKeywords::THCONR >();
p.addKeyword< ParserKeywords::THERMAL >();
p.addKeyword< ParserKeywords::THERMEX1 >();
p.addKeyword< ParserKeywords::THPRES >();
p.addKeyword< ParserKeywords::TITLE >();
p.addKeyword< ParserKeywords::TLMIXPAR >();
p.addKeyword< ParserKeywords::TLPMIXPA >();
p.addKeyword< ParserKeywords::TOPS >();
p.addKeyword< ParserKeywords::TRACER >();
p.addKeyword< ParserKeywords::TRACERS >();
p.addKeyword< ParserKeywords::TRANX >();
p.addKeyword< ParserKeywords::TRANY >();
p.addKeyword< ParserKeywords::TRANZ >();
p.addKeyword< ParserKeywords::TREF >();
p.addKeyword< ParserKeywords::TREFS >();
p.addKeyword< ParserKeywords::TSTEP >();
p.addKeyword< ParserKeywords::TUNING >();
p.addKeyword< ParserKeywords::TVDP >();
p.addKeyword< ParserKeywords::UDADIMS >();
p.addKeyword< ParserKeywords::UDQDIMS >();
p.addKeyword< ParserKeywords::UNIFIN >();
p.addKeyword< ParserKeywords::UNIFOUT >();
p.addKeyword< ParserKeywords::VAPOIL >();
p.addKeyword< ParserKeywords::VAPPARS >();
p.addKeyword< ParserKeywords::VFPIDIMS >();
p.addKeyword< ParserKeywords::VFPINJ >();
p.addKeyword< ParserKeywords::VFPPDIMS >();
p.addKeyword< ParserKeywords::VFPPROD >();
p.addKeyword< ParserKeywords::VISCREF >();
p.addKeyword< ParserKeywords::WATDENT >();
p.addKeyword< ParserKeywords::WATER >();
p.addKeyword< ParserKeywords::WATVISCT >();
p.addKeyword< ParserKeywords::WCONHIST >();
p.addKeyword< ParserKeywords::WCONINJ >();
p.addKeyword< ParserKeywords::WCONINJE >();
p.addKeyword< ParserKeywords::WCONINJH >();
p.addKeyword< ParserKeywords::WCONPROD >();
p.addKeyword< ParserKeywords::WELLDIMS >();
p.addKeyword< ParserKeywords::WELL_PROBE >();
p.addKeyword< ParserKeywords::WELOPEN >();
p.addKeyword< ParserKeywords::WELSEGS >();
p.addKeyword< ParserKeywords::WELSPECS >();
p.addKeyword< ParserKeywords::WELTARG >();
p.addKeyword< ParserKeywords::WGRUPCON >();
p.addKeyword< ParserKeywords::WHISTCTL >();
p.addKeyword< ParserKeywords::WPAVE >();
p.addKeyword< ParserKeywords::WPIMULT >();
p.addKeyword< ParserKeywords::WPITAB >();
p.addKeyword< ParserKeywords::WPOLYMER >();
p.addKeyword< ParserKeywords::WRFT >();
p.addKeyword< ParserKeywords::WRFTPLT >();
p.addKeyword< ParserKeywords::WSEGDIMS >();
p.addKeyword< ParserKeywords::WSOLVENT >();
p.addKeyword< ParserKeywords::WTEMP >();
p.addKeyword< ParserKeywords::WTEST >();
p.addKeyword< ParserKeywords::WTRACER >();
p.addKeyword< ParserKeywords::ZCORN >();
p.addKeyword< ParserKeywords::ZFACT1 >();
p.addKeyword< ParserKeywords::ZFACT1S >();
p.addKeyword< ParserKeywords::ZFACTOR >();
p.addKeyword< ParserKeywords::ZFACTORS >();
p.addKeyword< ParserKeywords::ZIPPY2 >();
}}}
