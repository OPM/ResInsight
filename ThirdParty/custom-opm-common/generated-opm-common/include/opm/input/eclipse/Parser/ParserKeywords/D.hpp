#ifndef PARSER_KEYWORDS_D_HPP
#define PARSER_KEYWORDS_D_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class DATE : public ParserKeyword {
   public:
       DATE();
       static const std::string keywordName;
   };



   class DATES : public ParserKeyword {
   public:
       DATES();
       static const std::string keywordName;

       class DAY {
       public:
           static const std::string itemName;
       };

       class MONTH {
       public:
           static const std::string itemName;
       };

       class YEAR {
       public:
           static const std::string itemName;
       };

       class TIME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DATUM : public ParserKeyword {
   public:
       DATUM();
       static const std::string keywordName;

       class DEPTH {
       public:
           static const std::string itemName;
       };
   };



   class DATUMR : public ParserKeyword {
   public:
       DATUMR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DATUMRX : public ParserKeyword {
   public:
       DATUMRX();
       static const std::string keywordName;

       class REGION_FAMILY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };
   };



   class DCQDEFN : public ParserKeyword {
   public:
       DCQDEFN();
       static const std::string keywordName;

       class QUANTITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DEBUG_ : public ParserKeyword {
   public:
       DEBUG_();
       static const std::string keywordName;

       class Item1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item3 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item4 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item5 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item6 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item7 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item8 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item9 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item10 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item11 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item12 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item13 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item14 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item15 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item16 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item17 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item18 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item19 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item20 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item21 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item22 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item23 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item24 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item25 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item26 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item27 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item28 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item29 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item30 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item31 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item32 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item33 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item34 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item35 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item36 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item37 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item38 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item39 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item40 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item41 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item42 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item43 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item44 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item45 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item46 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item47 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item48 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item49 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item50 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item51 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item52 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item53 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item54 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item55 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item56 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item57 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item58 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item59 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item60 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item61 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item62 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item63 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item64 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item65 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item66 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item67 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item68 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item69 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item70 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item71 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item72 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item73 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item74 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item75 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item76 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item77 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item78 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item79 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item80 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item81 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item82 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item83 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item84 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item85 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item86 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class Item87 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class DELAYACT : public ParserKeyword {
   public:
       DELAYACT();
       static const std::string keywordName;

       class ACTION_NAME {
       public:
           static const std::string itemName;
       };

       class ACTION_TRIGGER {
       public:
           static const std::string itemName;
       };

       class DELAY {
       public:
           static const std::string itemName;
       };

       class NUM_TIMES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class INCREMENT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class DENAQA : public ParserKeyword {
   public:
       DENAQA();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class DENSITY : public ParserKeyword {
   public:
       DENSITY();
       static const std::string keywordName;

       class OIL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 600.000000;
       };

       class WATER {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 999.014000;
       };

       class GAS {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DEPTH : public ParserKeyword {
   public:
       DEPTH();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DEPTHTAB : public ParserKeyword {
   public:
       DEPTHTAB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class DEPTHZ : public ParserKeyword {
   public:
       DEPTHZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIAGDISP : public ParserKeyword {
   public:
       DIAGDISP();
       static const std::string keywordName;
   };



   class DIFF : public ParserKeyword {
   public:
       DIFF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFAGAS : public ParserKeyword {
   public:
       DIFFAGAS();
       static const std::string keywordName;

       class CO2_IN_GAS {
       public:
           static const std::string itemName;
       };

       class H20_IN_GAS {
       public:
           static const std::string itemName;
       };
   };



   class DIFFAWAT : public ParserKeyword {
   public:
       DIFFAWAT();
       static const std::string keywordName;

       class CO2_IN_WATER {
       public:
           static const std::string itemName;
       };

       class H20_IN_WATER {
       public:
           static const std::string itemName;
       };
   };



   class DIFFC : public ParserKeyword {
   public:
       DIFFC();
       static const std::string keywordName;

       class OIL_MOL_WEIGHT {
       public:
           static const std::string itemName;
       };

       class GAS_MOL_WEIGHT {
       public:
           static const std::string itemName;
       };

       class GAS_GAS_DIFF_COEFF {
       public:
           static const std::string itemName;
       };

       class OIL_GAS_DIFF_COEFF {
       public:
           static const std::string itemName;
       };

       class GAS_OIL_DIFF_COEFF {
       public:
           static const std::string itemName;
       };

       class OIL_OIL_DIFF_COEFF {
       public:
           static const std::string itemName;
       };

       class GAS_OIL_CROSS_DIFF_COEFF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class OIL_OIL_CROSS_DIFF_COEFF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class DIFFCGAS : public ParserKeyword {
   public:
       DIFFCGAS();
       static const std::string keywordName;

       class CO2_IN_GAS {
       public:
           static const std::string itemName;
       };

       class H20_IN_GAS {
       public:
           static const std::string itemName;
       };
   };



   class DIFFCOAL : public ParserKeyword {
   public:
       DIFFCOAL();
       static const std::string keywordName;

       class GAS_DIFF_COEFF {
       public:
           static const std::string itemName;
       };

       class RE_ADSORB_FRACTION {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class SOL_DIFF_COEFF {
       public:
           static const std::string itemName;
       };
   };



   class DIFFCWAT : public ParserKeyword {
   public:
       DIFFCWAT();
       static const std::string keywordName;

       class CO2_IN_WATER {
       public:
           static const std::string itemName;
       };

       class H20_IN_WATER {
       public:
           static const std::string itemName;
       };
   };



   class DIFFDP : public ParserKeyword {
   public:
       DIFFDP();
       static const std::string keywordName;
   };



   class DIFFMMF : public ParserKeyword {
   public:
       DIFFMMF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFMR : public ParserKeyword {
   public:
       DIFFMR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFMTHT : public ParserKeyword {
   public:
       DIFFMTHT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFMX : public ParserKeyword {
   public:
       DIFFMX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFMY : public ParserKeyword {
   public:
       DIFFMY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFMZ : public ParserKeyword {
   public:
       DIFFMZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class DIFFR : public ParserKeyword {
   public:
       DIFFR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIFFTHT : public ParserKeyword {
   public:
       DIFFTHT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIFFUSE : public ParserKeyword {
   public:
       DIFFUSE();
       static const std::string keywordName;
   };



   class DIFFX : public ParserKeyword {
   public:
       DIFFX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIFFY : public ParserKeyword {
   public:
       DIFFY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIFFZ : public ParserKeyword {
   public:
       DIFFZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIMENS : public ParserKeyword {
   public:
       DIMENS();
       static const std::string keywordName;

       class NX {
       public:
           static const std::string itemName;
       };

       class NY {
       public:
           static const std::string itemName;
       };

       class NZ {
       public:
           static const std::string itemName;
       };
   };



   class DIMPES : public ParserKeyword {
   public:
       DIMPES();
       static const std::string keywordName;

       class DSTARG {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.050000;
       };

       class DSMAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class DPMAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 13.790000;
       };
   };



   class DIMPLICT : public ParserKeyword {
   public:
       DIMPLICT();
       static const std::string keywordName;
   };



   class DISGAS : public ParserKeyword {
   public:
       DISGAS();
       static const std::string keywordName;
   };



   class DISGASW : public ParserKeyword {
   public:
       DISGASW();
       static const std::string keywordName;
   };



   class DISPDIMS : public ParserKeyword {
   public:
       DISPDIMS();
       static const std::string keywordName;

       class NUM_DISP_TABLES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MAX_VELOCITY_NODES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 2;
       };

       class MAX_CONCENTRATION_NODES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class DISPERC : public ParserKeyword {
   public:
       DISPERC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DISPERSE : public ParserKeyword {
   public:
       DISPERSE();
       static const std::string keywordName;

       class VELOCITY {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class DOMAINS : public ParserKeyword {
   public:
       DOMAINS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DPGRID : public ParserKeyword {
   public:
       DPGRID();
       static const std::string keywordName;
   };



   class DPKRMOD : public ParserKeyword {
   public:
       DPKRMOD();
       static const std::string keywordName;

       class MOD_OIL_WAT_PERM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MOD_OIL_GAS_PERM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class SCALE_PERM_FRACTURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DPNUM : public ParserKeyword {
   public:
       DPNUM();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class DR : public ParserKeyword {
   public:
       DR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DREF : public ParserKeyword {
   public:
       DREF();
       static const std::string keywordName;

       class DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class DREFS : public ParserKeyword {
   public:
       DREFS();
       static const std::string keywordName;

       class DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class DRILPRI : public ParserKeyword {
   public:
       DRILPRI();
       static const std::string keywordName;

       class INTERVAL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class A {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class B {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class C {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class D {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class E {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class F {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class G {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class H {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class LOOK_AHEAD {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class CALCULATION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DRSDT : public ParserKeyword {
   public:
       DRSDT();
       static const std::string keywordName;

       class DRSDT_MAX {
       public:
           static const std::string itemName;
       };

       class OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DRSDTCON : public ParserKeyword {
   public:
       DRSDTCON();
       static const std::string keywordName;

       class DRSDT_MAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.040000;
       };

       class PSI {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.340000;
       };

       class OMEGA {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 3e-09;
       };

       class OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DRSDTR : public ParserKeyword {
   public:
       DRSDTR();
       static const std::string keywordName;

       class DRSDT_MAX {
       public:
           static const std::string itemName;
       };

       class OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DRV : public ParserKeyword {
   public:
       DRV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DRVDT : public ParserKeyword {
   public:
       DRVDT();
       static const std::string keywordName;

       class DRVDT_MAX {
       public:
           static const std::string itemName;
       };
   };



   class DRVDTR : public ParserKeyword {
   public:
       DRVDTR();
       static const std::string keywordName;

       class DRVDT_MAX {
       public:
           static const std::string itemName;
       };
   };



   class DSPDEINT : public ParserKeyword {
   public:
       DSPDEINT();
       static const std::string keywordName;
   };



   class DTHETA : public ParserKeyword {
   public:
       DTHETA();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DTHETAV : public ParserKeyword {
   public:
       DTHETAV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DUALPERM : public ParserKeyword {
   public:
       DUALPERM();
       static const std::string keywordName;
   };



   class DUALPORO : public ParserKeyword {
   public:
       DUALPORO();
       static const std::string keywordName;
   };



   class DUMPCUPL : public ParserKeyword {
   public:
       DUMPCUPL();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class DUMPFLUX : public ParserKeyword {
   public:
       DUMPFLUX();
       static const std::string keywordName;
   };



   class DX : public ParserKeyword {
   public:
       DX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DXV : public ParserKeyword {
   public:
       DXV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DY : public ParserKeyword {
   public:
       DY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DYNAMICR : public ParserKeyword {
   public:
       DYNAMICR();
       static const std::string keywordName;

       class code {
       public:
           static const std::string itemName;
       };
   };



   class DYNRDIMS : public ParserKeyword {
   public:
       DYNRDIMS();
       static const std::string keywordName;

       class MNUMDR {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MXDYNF {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MXDYNR {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class DYV : public ParserKeyword {
   public:
       DYV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DZ : public ParserKeyword {
   public:
       DZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DZMATRIX : public ParserKeyword {
   public:
       DZMATRIX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class DZMTRX : public ParserKeyword {
   public:
       DZMTRX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class DZMTRXV : public ParserKeyword {
   public:
       DZMTRXV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class DZNET : public ParserKeyword {
   public:
       DZNET();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DZV : public ParserKeyword {
   public:
       DZV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
