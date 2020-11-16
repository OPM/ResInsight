#ifndef PARSER_KEYWORDS_C_HPP
#define PARSER_KEYWORDS_C_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
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
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WCUT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_GOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_WGR {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const double defaultValue;
       };

       class MIN_GAS {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const double defaultValue;
       };

       class MAX_TOTAL_TRACER_CONC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_FREE_TRACER_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_FREE_TRACER_CONC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_SOL_TRACER_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_SOL_TRACER_CONC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class CIRCLE : public ParserKeyword {
   public:
       CIRCLE();
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
           static const int defaultValue;
       };
   };



   class COLUMNS : public ParserKeyword {
   public:
       COLUMNS();
       static const std::string keywordName;

       class LEFT_MARGIN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class RIGHT_MARGIN {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
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
           static const double defaultValue;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
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
           static const double defaultValue;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class F1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class F2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class FLASH_PVTNUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class REL_PERM {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
       };

       class J {
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const double defaultValue;
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
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class R2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
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



   class CREFS : public ParserKeyword {
   public:
       CREFS();
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
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_UPPER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K_LOWER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class CONNECTION_SKIN_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



}
}
#endif
