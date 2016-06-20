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


void addDefaultKeywords0(Parser& p) {
p.addKeyword< ParserKeywords::ACTDIMS >();
p.addKeyword< ParserKeywords::ACTNUM >();
p.addKeyword< ParserKeywords::ADD >();
p.addKeyword< ParserKeywords::ADDREG >();
p.addKeyword< ParserKeywords::ADSALNOD >();
p.addKeyword< ParserKeywords::ALL >();
p.addKeyword< ParserKeywords::API >();
p.addKeyword< ParserKeywords::AQUCON >();
p.addKeyword< ParserKeywords::AQUDIMS >();
p.addKeyword< ParserKeywords::AQUNUM >();
p.addKeyword< ParserKeywords::BLOCK_PROBE >();
p.addKeyword< ParserKeywords::BLOCK_PROBE300 >();
p.addKeyword< ParserKeywords::BOX >();
p.addKeyword< ParserKeywords::COMPDAT >();
p.addKeyword< ParserKeywords::COMPLUMP >();
p.addKeyword< ParserKeywords::COMPORD >();
p.addKeyword< ParserKeywords::COMPS >();
p.addKeyword< ParserKeywords::COMPSEGS >();
p.addKeyword< ParserKeywords::CONNECTION_PROBE >();
p.addKeyword< ParserKeywords::COORD >();
p.addKeyword< ParserKeywords::COPY >();
p.addKeyword< ParserKeywords::COPYREG >();
p.addKeyword< ParserKeywords::CPR >();
p.addKeyword< ParserKeywords::CREF >();
p.addKeyword< ParserKeywords::CREFS >();
p.addKeyword< ParserKeywords::DATE >();
p.addKeyword< ParserKeywords::DATES >();
p.addKeyword< ParserKeywords::DENSITY >();
p.addKeyword< ParserKeywords::DEPTH >();
p.addKeyword< ParserKeywords::DEPTHZ >();
p.addKeyword< ParserKeywords::DIMENS >();
p.addKeyword< ParserKeywords::DISGAS >();
p.addKeyword< ParserKeywords::DREF >();
p.addKeyword< ParserKeywords::DREFS >();
p.addKeyword< ParserKeywords::DRSDT >();
p.addKeyword< ParserKeywords::DRVDT >();
p.addKeyword< ParserKeywords::DX >();
p.addKeyword< ParserKeywords::DXV >();
p.addKeyword< ParserKeywords::DY >();
p.addKeyword< ParserKeywords::DYV >();
p.addKeyword< ParserKeywords::DZ >();
p.addKeyword< ParserKeywords::DZV >();
p.addKeyword< ParserKeywords::ECHO >();
p.addKeyword< ParserKeywords::EDIT >();
p.addKeyword< ParserKeywords::EDITNNC >();
p.addKeyword< ParserKeywords::EHYSTR >();
p.addKeyword< ParserKeywords::END >();
p.addKeyword< ParserKeywords::ENDBOX >();
p.addKeyword< ParserKeywords::ENDINC >();
p.addKeyword< ParserKeywords::ENDNUM >();
p.addKeyword< ParserKeywords::ENDPOINT_SPECIFIERS >();
p.addKeyword< ParserKeywords::ENDSCALE >();
p.addKeyword< ParserKeywords::ENDSKIP >();
p.addKeyword< ParserKeywords::ENKRVD >();
p.addKeyword< ParserKeywords::ENPTVD >();
p.addKeyword< ParserKeywords::EQLDIMS >();
p.addKeyword< ParserKeywords::EQLNUM >();
p.addKeyword< ParserKeywords::EQLOPTS >();
p.addKeyword< ParserKeywords::EQUALREG >();
p.addKeyword< ParserKeywords::EQUALS >();
p.addKeyword< ParserKeywords::EQUIL >();
p.addKeyword< ParserKeywords::EXCEL >();
p.addKeyword< ParserKeywords::EXTRAPMS >();
p.addKeyword< ParserKeywords::FAULTDIM >();
p.addKeyword< ParserKeywords::FAULTS >();
p.addKeyword< ParserKeywords::FIELD >();
p.addKeyword< ParserKeywords::FIELD_PROBE >();
p.addKeyword< ParserKeywords::FILLEPS >();
p.addKeyword< ParserKeywords::FIPNUM >();
p.addKeyword< ParserKeywords::FLUXNUM >();
p.addKeyword< ParserKeywords::FMTIN >();
p.addKeyword< ParserKeywords::FMTOUT >();
p.addKeyword< ParserKeywords::FULLIMP >();
p.addKeyword< ParserKeywords::GAS >();
p.addKeyword< ParserKeywords::GASVISCT >();
p.addKeyword< ParserKeywords::GCOMPIDX >();
p.addKeyword< ParserKeywords::GCONINJE >();
p.addKeyword< ParserKeywords::GCONPROD >();
p.addKeyword< ParserKeywords::GDORIENT >();
p.addKeyword< ParserKeywords::GECON >();
p.addKeyword< ParserKeywords::GEFAC >();
p.addKeyword< ParserKeywords::GRID >();
p.addKeyword< ParserKeywords::GRIDFILE >();
p.addKeyword< ParserKeywords::GRIDOPTS >();
}}}
