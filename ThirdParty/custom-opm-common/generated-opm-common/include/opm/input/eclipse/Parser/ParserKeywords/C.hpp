#ifndef PARSER_KEYWORDS_C_HPP
#define PARSER_KEYWORDS_C_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class CALTRAC : public ParserKeyword {
   public:
       CALTRAC();
       static const std::string keywordName;

       class IX1 {
       public:
           static const std::string itemName;
       };
   };



   class CARFIN : public ParserKeyword {
   public:
       CARFIN();
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

       class NWMAX {
       public:
           static const std::string itemName;
       };

       class PARENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class CART : public ParserKeyword {
   public:
       CART();
       static const std::string keywordName;
   };



   class CBMOPTS : public ParserKeyword {
   public:
       CBMOPTS();
       static const std::string keywordName;

       class ADSORPTION_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ALLOW_WATER_FLOW {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ALLOW_PERMEAB {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class COUNT_PASSES {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SCALING_VALUE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class APPLICATION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PRESSURE_CHOP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIN_PORE_VOLUME {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 5e-06;
       };
   };



   class CECON : public ParserKeyword {
   public:
       CECON();
       static const std::string keywordName;

       class WELLNAME {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MAX_WCUT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_GOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_WGR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class WORKOVER_PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHECK_STOPPED {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIN_OIL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -100000000000000000000.000000;
       };

       class MIN_GAS {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -100000000000000000000.000000;
       };

       class FOLLOW_ON_WELL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class CECONT : public ParserKeyword {
   public:
       CECONT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHECK_STOPPED_WELLS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TRACER {
       public:
           static const std::string itemName;
       };

       class MAX_TOTAL_TRACER_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_TOTAL_TRACER_CONC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_FREE_TRACER_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_FREE_TRACER_CONC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_SOL_TRACER_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_SOL_TRACER_CONC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };
   };



   class CIRCLE : public ParserKeyword {
   public:
       CIRCLE();
       static const std::string keywordName;
   };



   class CNAMES : public ParserKeyword {
   public:
       CNAMES();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class CO2SOL : public ParserKeyword {
   public:
       CO2SOL();
       static const std::string keywordName;
   };



   class CO2STOR : public ParserKeyword {
   public:
       CO2STOR();
       static const std::string keywordName;
   };



   class CO2STORE : public ParserKeyword {
   public:
       CO2STORE();
       static const std::string keywordName;
   };



   class COAL : public ParserKeyword {
   public:
       COAL();
       static const std::string keywordName;
   };



   class COALADS : public ParserKeyword {
   public:
       COALADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class COALNUM : public ParserKeyword {
   public:
       COALNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class COALPP : public ParserKeyword {
   public:
       COALPP();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class COARSEN : public ParserKeyword {
   public:
       COARSEN();
       static const std::string keywordName;

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



   class COLLAPSE : public ParserKeyword {
   public:
       COLLAPSE();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class COLUMNS : public ParserKeyword {
   public:
       COLUMNS();
       static const std::string keywordName;

       class LEFT_MARGIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class RIGHT_MARGIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 132;
       };
   };



   class COMPDAT : public ParserKeyword {
   public:
       COMPDAT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class STATE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SAT_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class CONNECTION_TRANSMISSIBILITY_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIAMETER {
       public:
           static const std::string itemName;
       };

       class Kh {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -1.000000;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class D_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class DIR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PR {
       public:
           static const std::string itemName;
       };
   };



   class COMPDATX : public ParserKeyword {
   public:
       COMPDATX();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class LGR {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class STATE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SAT_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class CONNECTION_TRANSMISSIBILITY_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIAMETER {
       public:
           static const std::string itemName;
       };

       class Kh {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -1.000000;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class D_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PR {
       public:
           static const std::string itemName;
       };
   };



   class COMPFLSH : public ParserKeyword {
   public:
       COMPFLSH();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class F1 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class F2 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class FLASH_PVTNUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class COMPIMB : public ParserKeyword {
   public:
       COMPIMB();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class COMPINJK : public ParserKeyword {
   public:
       COMPINJK();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class REL_PERM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class COMPLMPL : public ParserKeyword {
   public:
       COMPLMPL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GRID {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class COMPLETION_NUMBER {
       public:
           static const std::string itemName;
       };
   };



   class COMPLUMP : public ParserKeyword {
   public:
       COMPLUMP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class N {
       public:
           static const std::string itemName;
       };
   };



   class COMPOFF : public ParserKeyword {
   public:
       COMPOFF();
       static const std::string keywordName;
   };



   class COMPORD : public ParserKeyword {
   public:
       COMPORD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class ORDER_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class COMPRIV : public ParserKeyword {
   public:
       COMPRIV();
       static const std::string keywordName;

       class RIVER {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class COMPRP : public ParserKeyword {
   public:
       COMPRP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class SWMIN {
       public:
           static const std::string itemName;
       };

       class SWMAX {
       public:
           static const std::string itemName;
       };

       class SGMIN {
       public:
           static const std::string itemName;
       };

       class SGMAX {
       public:
           static const std::string itemName;
       };
   };



   class COMPRPL : public ParserKeyword {
   public:
       COMPRPL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class LOCAL_GRID {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class SWMIN {
       public:
           static const std::string itemName;
       };

       class SWMAX {
       public:
           static const std::string itemName;
       };

       class SGMIN {
       public:
           static const std::string itemName;
       };

       class SGMAX {
       public:
           static const std::string itemName;
       };
   };



   class COMPS : public ParserKeyword {
   public:
       COMPS();
       static const std::string keywordName;

       class NUM_COMPS {
       public:
           static const std::string itemName;
       };
   };



   class COMPSEGL : public ParserKeyword {
   public:
       COMPSEGL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GRID {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class DISTANCE_START {
       public:
           static const std::string itemName;
       };

       class DISTANCE_END {
       public:
           static const std::string itemName;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
       };

       class END_IJK {
       public:
           static const std::string itemName;
       };

       class CENTER_DEPTH {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class THERMAL_LENGTH {
       public:
           static const std::string itemName;
       };

       class SEGMENT_NUMBER {
       public:
           static const std::string itemName;
       };
   };



   class COMPSEGS : public ParserKeyword {
   public:
       COMPSEGS();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class DISTANCE_START {
       public:
           static const std::string itemName;
       };

       class DISTANCE_END {
       public:
           static const std::string itemName;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
       };

       class END_IJK {
       public:
           static const std::string itemName;
       };

       class CENTER_DEPTH {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class THERMAL_LENGTH {
       public:
           static const std::string itemName;
       };

       class SEGMENT_NUMBER {
       public:
           static const std::string itemName;
       };
   };



   class COMPTRAJ : public ParserKeyword {
   public:
       COMPTRAJ();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class BRANCH_NUMBER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class PERF_TOP {
       public:
           static const std::string itemName;
       };

       class PERF_BOT {
       public:
           static const std::string itemName;
       };

       class PERF_REF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class COMPLETION_NUMBER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class STATE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SAT_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class CONNECTION_TRANSMISSIBILITY_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIAMETER {
       public:
           static const std::string itemName;
       };

       class Kh {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -1.000000;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class D_FACTOR {
       public:
           static const std::string itemName;
       };
   };



   class COMPVE : public ParserKeyword {
   public:
       COMPVE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class CVEFRAC {
       public:
           static const std::string itemName;
       };

       class DTOP {
       public:
           static const std::string itemName;
       };

       class DBOT {
       public:
           static const std::string itemName;
       };

       class FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class S_D {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GTOP {
       public:
           static const std::string itemName;
       };

       class GBOT {
       public:
           static const std::string itemName;
       };
   };



   class COMPVEL : public ParserKeyword {
   public:
       COMPVEL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class LOCAL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class CVEFRAC {
       public:
           static const std::string itemName;
       };

       class DTOP {
       public:
           static const std::string itemName;
       };

       class DBOT {
       public:
           static const std::string itemName;
       };

       class FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class S_D {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GTOP {
       public:
           static const std::string itemName;
       };

       class GBOT {
       public:
           static const std::string itemName;
       };
   };



   class CONNECTION_PROBE : public ParserKeyword {
   public:
       CONNECTION_PROBE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class CONNECTION_PROBE_OPM : public ParserKeyword {
   public:
       CONNECTION_PROBE_OPM();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class COORD : public ParserKeyword {
   public:
       COORD();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class COORDSYS : public ParserKeyword {
   public:
       COORDSYS();
       static const std::string keywordName;

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class CIRCLE_COMPLETION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CONNECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class R1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class R2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class COPY : public ParserKeyword {
   public:
       COPY();
       static const std::string keywordName;

       class src {
       public:
           static const std::string itemName;
       };

       class target {
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
   };



   class COPYBOX : public ParserKeyword {
   public:
       COPYBOX();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class IX1S {
       public:
           static const std::string itemName;
       };

       class IX2S {
       public:
           static const std::string itemName;
       };

       class JY1S {
       public:
           static const std::string itemName;
       };

       class JY2S {
       public:
           static const std::string itemName;
       };

       class KY1S {
       public:
           static const std::string itemName;
       };

       class KY2S {
       public:
           static const std::string itemName;
       };

       class IX1D {
       public:
           static const std::string itemName;
       };

       class IX2D {
       public:
           static const std::string itemName;
       };

       class JY1D {
       public:
           static const std::string itemName;
       };

       class JY2D {
       public:
           static const std::string itemName;
       };

       class KY1D {
       public:
           static const std::string itemName;
       };

       class KY2D {
       public:
           static const std::string itemName;
       };
   };



   class COPYREG : public ParserKeyword {
   public:
       COPYREG();
       static const std::string keywordName;

       class SRC_ARRAY {
       public:
           static const std::string itemName;
       };

       class TARGET_ARRAY {
       public:
           static const std::string itemName;
       };

       class REGION_NUMBER {
       public:
           static const std::string itemName;
       };

       class REGION_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class CPIFACT : public ParserKeyword {
   public:
       CPIFACT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MULT {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class C1 {
       public:
           static const std::string itemName;
       };

       class C2 {
       public:
           static const std::string itemName;
       };
   };



   class CPIFACTL : public ParserKeyword {
   public:
       CPIFACTL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MULT {
       public:
           static const std::string itemName;
       };

       class LGR {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class C1 {
       public:
           static const std::string itemName;
       };

       class C2 {
       public:
           static const std::string itemName;
       };
   };



   class CPR : public ParserKeyword {
   public:
       CPR();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class CREF : public ParserKeyword {
   public:
       CREF();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };
   };



   class CREFW : public ParserKeyword {
   public:
       CREFW();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };
   };



   class CREFWS : public ParserKeyword {
   public:
       CREFWS();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };
   };



   class CRITPERM : public ParserKeyword {
   public:
       CRITPERM();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class CSKIN : public ParserKeyword {
   public:
       CSKIN();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class J {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class CONNECTION_SKIN_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



}
}
#endif
