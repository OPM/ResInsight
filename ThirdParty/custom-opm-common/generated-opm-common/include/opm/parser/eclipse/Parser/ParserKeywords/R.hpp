#ifndef PARSER_KEYWORDS_R_HPP
#define PARSER_KEYWORDS_R_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class RADFIN4 : public ParserKeyword {
   public:
       RADFIN4();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class I1 {
       public:
           static const std::string itemName;
       };

       class I2 {
       public:
           static const std::string itemName;
       };

       class J1 {
       public:
           static const std::string itemName;
       };

       class J2 {
       public:
           static const std::string itemName;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class NR {
       public:
           static const std::string itemName;
       };

       class NTHETA {
       public:
           static const std::string itemName;
       };

       class NZ {
       public:
           static const std::string itemName;
       };

       class NWMAX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class RADIAL : public ParserKeyword {
   public:
       RADIAL();
       static const std::string keywordName;
   };



   class RAINFALL : public ParserKeyword {
   public:
       RAINFALL();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class JAN_FLUX {
       public:
           static const std::string itemName;
       };

       class FEB_FLUX {
       public:
           static const std::string itemName;
       };

       class MAR_FLUX {
       public:
           static const std::string itemName;
       };

       class APR_FLUX {
       public:
           static const std::string itemName;
       };

       class MAI_FLUX {
       public:
           static const std::string itemName;
       };

       class JUN_FLUX {
       public:
           static const std::string itemName;
       };

       class JUL_FLUX {
       public:
           static const std::string itemName;
       };

       class AUG_FLUX {
       public:
           static const std::string itemName;
       };

       class SEP_FLUX {
       public:
           static const std::string itemName;
       };

       class OCT_FLUX {
       public:
           static const std::string itemName;
       };

       class NOV_FLUX {
       public:
           static const std::string itemName;
       };

       class DES_FLUX {
       public:
           static const std::string itemName;
       };
   };



   class RBEDCONT : public ParserKeyword {
   public:
       RBEDCONT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RCMASTS : public ParserKeyword {
   public:
       RCMASTS();
       static const std::string keywordName;

       class MIN_TSTEP {
       public:
           static const std::string itemName;
       };
   };



   class REACHES : public ParserKeyword {
   public:
       REACHES();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class XPOS {
       public:
           static const std::string itemName;
       };

       class YPOS {
       public:
           static const std::string itemName;
       };

       class ZPOS {
       public:
           static const std::string itemName;
       };

       class LENGTH1 {
       public:
           static const std::string itemName;
       };

       class INPUT_TYPE {
       public:
           static const std::string itemName;
       };

       class START_REACH {
       public:
           static const std::string itemName;
       };

       class END_REACH {
       public:
           static const std::string itemName;
       };

       class OUTLET_REACH {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class LENGTH2 {
       public:
           static const std::string itemName;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };

       class PROFILE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class XLENGTH {
       public:
           static const std::string itemName;
       };

       class YLENGTH {
       public:
           static const std::string itemName;
       };

       class REACH_LENGTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class NUM_REACHES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DEPTH_SOMETHING {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class READDATA : public ParserKeyword {
   public:
       READDATA();
       static const std::string keywordName;

       class INPUT_METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class REFINE : public ParserKeyword {
   public:
       REFINE();
       static const std::string keywordName;

       class LGR {
       public:
           static const std::string itemName;
       };
   };



   class REGDIMS : public ParserKeyword {
   public:
       REGDIMS();
       static const std::string keywordName;

       class NTFIP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NMFIPR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRFREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTFREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ETRACK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTCREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERNUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERATE_DWORK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERATE_IWORK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPLMIX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class REGION2REGION_PROBE : public ParserKeyword {
   public:
       REGION2REGION_PROBE();
       static const std::string keywordName;

       class REGION1 {
       public:
           static const std::string itemName;
       };

       class REGION2 {
       public:
           static const std::string itemName;
       };
   };



   class REGIONS : public ParserKeyword {
   public:
       REGIONS();
       static const std::string keywordName;
   };



   class REGION_PROBE : public ParserKeyword {
   public:
       REGION_PROBE();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RESIDNUM : public ParserKeyword {
   public:
       RESIDNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RESTART : public ParserKeyword {
   public:
       RESTART();
       static const std::string keywordName;

       class ROOTNAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REPORTNUMBER {
       public:
           static const std::string itemName;
       };

       class SAVEFILE {
       public:
           static const std::string itemName;
       };

       class SAVEFILE_FORMAT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class RESVNUM : public ParserKeyword {
   public:
       RESVNUM();
       static const std::string keywordName;

       class NEXT_RES {
       public:
           static const std::string itemName;
       };
   };



   class RHO : public ParserKeyword {
   public:
       RHO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class RIVDEBUG : public ParserKeyword {
   public:
       RIVDEBUG();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class DEBUG_CONTROL {
       public:
           static const std::string itemName;
       };
   };



   class RIVERSYS : public ParserKeyword {
   public:
       RIVERSYS();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class EQUATION {
       public:
           static const std::string itemName;
       };

       class BRANCH_NR {
       public:
           static const std::string itemName;
       };

       class BRANCH_NAME {
       public:
           static const std::string itemName;
       };

       class DOWNSTREAM_BC {
       public:
           static const std::string itemName;
       };
   };



   class RIVRDIMS : public ParserKeyword {
   public:
       RIVRDIMS();
       static const std::string keywordName;

       class MAX_RIVERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_REACHES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_BRANCHES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_BLOCKS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXTBPR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXDPTB {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXTBGR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NMDEPT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXDEPT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NMMAST {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXMAST {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRATTA {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXRATE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class RIVRPROP : public ParserKeyword {
   public:
       RIVRPROP();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class REACH1 {
       public:
           static const std::string itemName;
       };

       class REACH2 {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };
   };



   class RIVRXSEC : public ParserKeyword {
   public:
       RIVRXSEC();
       static const std::string keywordName;

       class DEPTH {
       public:
           static const std::string itemName;
       };

       class WET_PERIMTER {
       public:
           static const std::string itemName;
       };

       class AREA {
       public:
           static const std::string itemName;
       };
   };



   class RIVSALT : public ParserKeyword {
   public:
       RIVSALT();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class SALINITY {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class REACH {
       public:
           static const std::string itemName;
       };
   };



   class RIVTRACE : public ParserKeyword {
   public:
       RIVTRACE();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class TRACER {
       public:
           static const std::string itemName;
       };

       class TC {
       public:
           static const std::string itemName;
       };

       class TCUM {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class REACH {
       public:
           static const std::string itemName;
       };
   };



   class RKTRMDIR : public ParserKeyword {
   public:
       RKTRMDIR();
       static const std::string keywordName;
   };



   class ROCK : public ParserKeyword {
   public:
       ROCK();
       static const std::string keywordName;

       class PREF {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ROCK2D : public ParserKeyword {
   public:
       ROCK2D();
       static const std::string keywordName;

       class PRESSURE {
       public:
           static const std::string itemName;
       };

       class PVMULT {
       public:
           static const std::string itemName;
       };
   };



   class ROCK2DTR : public ParserKeyword {
   public:
       ROCK2DTR();
       static const std::string keywordName;

       class PRESSURE {
       public:
           static const std::string itemName;
       };

       class TRANSMULT {
       public:
           static const std::string itemName;
       };
   };



   class ROCKCOMP : public ParserKeyword {
   public:
       ROCKCOMP();
       static const std::string keywordName;

       class HYSTERESIS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NTROCC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class WATER_COMPACTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class ROCKFRAC : public ParserKeyword {
   public:
       ROCKFRAC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ROCKNUM : public ParserKeyword {
   public:
       ROCKNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ROCKOPTS : public ParserKeyword {
   public:
       ROCKOPTS();
       static const std::string keywordName;

       class METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REF_PRESSURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TABLE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class HYST_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class ROCKPAMA : public ParserKeyword {
   public:
       ROCKPAMA();
       static const std::string keywordName;

       class K {
       public:
           static const std::string itemName;
       };

       class M {
       public:
           static const std::string itemName;
       };

       class G {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class B {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class E1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class f {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class n {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class g {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class Bs {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class Es {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ROCKTAB : public ParserKeyword {
   public:
       ROCKTAB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ROCKTABH : public ParserKeyword {
   public:
       ROCKTABH();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ROCKTABW : public ParserKeyword {
   public:
       ROCKTABW();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ROCKTHSG : public ParserKeyword {
   public:
       ROCKTHSG();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ROCKTSIG : public ParserKeyword {
   public:
       ROCKTSIG();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ROCKV : public ParserKeyword {
   public:
       ROCKV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ROCKWNOD : public ParserKeyword {
   public:
       ROCKWNOD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RPTCPL : public ParserKeyword {
   public:
       RPTCPL();
       static const std::string keywordName;
   };



   class RPTGRID : public ParserKeyword {
   public:
       RPTGRID();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RPTGRIDL : public ParserKeyword {
   public:
       RPTGRIDL();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RPTHMD : public ParserKeyword {
   public:
       RPTHMD();
       static const std::string keywordName;

       class ITEM1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM3 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM5 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM6 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class RPTHMG : public ParserKeyword {
   public:
       RPTHMG();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class INCLUDE {
       public:
           static const std::string itemName;
       };
   };



   class RPTHMW : public ParserKeyword {
   public:
       RPTHMW();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class INCLUDE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class INCLUDE_RFT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class RPTINIT : public ParserKeyword {
   public:
       RPTINIT();
       static const std::string keywordName;

       class MNEMONICS_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTISOL : public ParserKeyword {
   public:
       RPTISOL();
       static const std::string keywordName;
   };



   class RPTONLY : public ParserKeyword {
   public:
       RPTONLY();
       static const std::string keywordName;
   };



   class RPTONLYO : public ParserKeyword {
   public:
       RPTONLYO();
       static const std::string keywordName;
   };



   class RPTPROPS : public ParserKeyword {
   public:
       RPTPROPS();
       static const std::string keywordName;

       class mnemonics {
       public:
           static const std::string itemName;
       };
   };



   class RPTREGS : public ParserKeyword {
   public:
       RPTREGS();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTRST : public ParserKeyword {
   public:
       RPTRST();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTRUNSP : public ParserKeyword {
   public:
       RPTRUNSP();
       static const std::string keywordName;
   };



   class RPTSCHED : public ParserKeyword {
   public:
       RPTSCHED();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTSMRY : public ParserKeyword {
   public:
       RPTSMRY();
       static const std::string keywordName;

       class WRITE {
       public:
           static const std::string itemName;
       };
   };



   class RPTSOL : public ParserKeyword {
   public:
       RPTSOL();
       static const std::string keywordName;

       class mnemonics {
       public:
           static const std::string itemName;
       };
   };



   class RS : public ParserKeyword {
   public:
       RS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RSCONST : public ParserKeyword {
   public:
       RSCONST();
       static const std::string keywordName;

       class RS {
       public:
           static const std::string itemName;
       };

       class PB {
       public:
           static const std::string itemName;
       };
   };



   class RSCONSTT : public ParserKeyword {
   public:
       RSCONSTT();
       static const std::string keywordName;

       class RS_CONSTT {
       public:
           static const std::string itemName;
       };

       class PB_CONSTT {
       public:
           static const std::string itemName;
       };
   };



   class RSGI : public ParserKeyword {
   public:
       RSGI();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RSSPEC : public ParserKeyword {
   public:
       RSSPEC();
       static const std::string keywordName;
   };



   class RSVD : public ParserKeyword {
   public:
       RSVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RTEMP : public ParserKeyword {
   public:
       RTEMP();
       static const std::string keywordName;

       class TEMP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class RTEMPA : public ParserKeyword {
   public:
       RTEMPA();
       static const std::string keywordName;

       class TEMP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class RTEMPVD : public ParserKeyword {
   public:
       RTEMPVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RUNSPEC : public ParserKeyword {
   public:
       RUNSPEC();
       static const std::string keywordName;
   };



   class RUNSUM : public ParserKeyword {
   public:
       RUNSUM();
       static const std::string keywordName;
   };



   class RV : public ParserKeyword {
   public:
       RV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RVCONST : public ParserKeyword {
   public:
       RVCONST();
       static const std::string keywordName;

       class RV {
       public:
           static const std::string itemName;
       };

       class DEWP {
       public:
           static const std::string itemName;
       };
   };



   class RVCONSTT : public ParserKeyword {
   public:
       RVCONSTT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RVGI : public ParserKeyword {
   public:
       RVGI();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RVVD : public ParserKeyword {
   public:
       RVVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RWGSALT : public ParserKeyword {
   public:
       RWGSALT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
