#ifndef PARSER_KEYWORDS_G_HPP
#define PARSER_KEYWORDS_G_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class GAS : public ParserKeyword {
   public:
       GAS();
       static const std::string keywordName;
   };



   class GASBEGIN : public ParserKeyword {
   public:
       GASBEGIN();
       static const std::string keywordName;
   };



   class GASCONC : public ParserKeyword {
   public:
       GASCONC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class GASDENT : public ParserKeyword {
   public:
       GASDENT();
       static const std::string keywordName;

       class REFERENCE_TEMPERATURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 293.150000;
       };

       class EXPANSION_COEFF_LINEAR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class EXPANSION_COEFF_QUADRATIC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GASEND : public ParserKeyword {
   public:
       GASEND();
       static const std::string keywordName;
   };



   class GASFCOMP : public ParserKeyword {
   public:
       GASFCOMP();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ARTFICIAL_LIFT_QNTY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GAS_CONSUMPTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class COMPRESSION_LVL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class ACTION_SEQ_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class GASFDECR : public ParserKeyword {
   public:
       GASFDECR();
       static const std::string keywordName;

       class JAN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class FEB {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class APR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class JUN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class JUL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class AUG {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class SEP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class OCT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class NOV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class DEC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GASFDELC : public ParserKeyword {
   public:
       GASFDELC();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class GASFIELD : public ParserKeyword {
   public:
       GASFIELD();
       static const std::string keywordName;

       class FLAG_COMP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLAG_IT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GASFTARG : public ParserKeyword {
   public:
       GASFTARG();
       static const std::string keywordName;

       class JAN {
       public:
           static const std::string itemName;
       };

       class FEB {
       public:
           static const std::string itemName;
       };

       class MAR {
       public:
           static const std::string itemName;
       };

       class APR {
       public:
           static const std::string itemName;
       };

       class MAY {
       public:
           static const std::string itemName;
       };

       class JUN {
       public:
           static const std::string itemName;
       };

       class JUL {
       public:
           static const std::string itemName;
       };

       class AUG {
       public:
           static const std::string itemName;
       };

       class SEP {
       public:
           static const std::string itemName;
       };

       class OCT {
       public:
           static const std::string itemName;
       };

       class NOV {
       public:
           static const std::string itemName;
       };

       class DEC {
       public:
           static const std::string itemName;
       };
   };



   class GASJT : public ParserKeyword {
   public:
       GASJT();
       static const std::string keywordName;

       class PREF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.013200;
       };

       class JOULE_THOMSON_COEFFICIENT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GASMONTH : public ParserKeyword {
   public:
       GASMONTH();
       static const std::string keywordName;

       class MONTH {
       public:
           static const std::string itemName;
       };

       class WRITE_REPORT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GASPERIO : public ParserKeyword {
   public:
       GASPERIO();
       static const std::string keywordName;

       class NUM_PERIODS {
       public:
           static const std::string itemName;
       };

       class NUM_MONTHS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };

       class INITIAL_DCQ {
       public:
           static const std::string itemName;
       };

       class SWING_REQ {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIMIT_TIMESTEPS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIMIT_DCQ_RED_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class ANTICIPATED_DCQ_RED_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class MAX_ITERATIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 3;
       };

       class DCQ_CONV_TOLERANCE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };
   };



   class GASSATC : public ParserKeyword {
   public:
       GASSATC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class GASVISCT : public ParserKeyword {
   public:
       GASVISCT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class GASWAT : public ParserKeyword {
   public:
       GASWAT();
       static const std::string keywordName;
   };



   class GASYEAR : public ParserKeyword {
   public:
       GASYEAR();
       static const std::string keywordName;

       class NUM_YEARS {
       public:
           static const std::string itemName;
       };

       class INITIAL_DCQ {
       public:
           static const std::string itemName;
       };

       class SWING_REQ {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIMIT_TIMESTEPS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIMIT_DCQ_RED_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class ANTICIPATED_DCQ_RED_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class MAX_ITERATIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 3;
       };

       class DCQ_CONV_TOLERANCE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };
   };



   class GCALECON : public ParserKeyword {
   public:
       GCALECON();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class MIN_ENERGY_PROD_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MIN_CALORIFIC_VAL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class FLAG_END_RUN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GCOMPIDX : public ParserKeyword {
   public:
       GCOMPIDX();
       static const std::string keywordName;

       class GAS_COMPONENT_INDEX {
       public:
           static const std::string itemName;
       };
   };



   class GCONCAL : public ParserKeyword {
   public:
       GCONCAL();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class MEAN_CALORIFIC_VAL {
       public:
           static const std::string itemName;
       };

       class ACTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RATE_RED_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.900000;
       };
   };



   class GCONENG : public ParserKeyword {
   public:
       GCONENG();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class ENERGY_PROD_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };
   };



   class GCONINJE : public ParserKeyword {
   public:
       GCONINJE();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class CONTROL_MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SURFACE_TARGET {
       public:
           static const std::string itemName;
       };

       class RESV_TARGET {
       public:
           static const std::string itemName;
       };

       class REINJ_TARGET {
       public:
           static const std::string itemName;
       };

       class VOIDAGE_TARGET {
       public:
           static const std::string itemName;
       };

       class RESPOND_TO_PARENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GUIDE_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GUIDE_RATE_DEF {
       public:
           static const std::string itemName;
       };

       class REINJECT_GROUP {
       public:
           static const std::string itemName;
       };

       class VOIDAGE_GROUP {
       public:
           static const std::string itemName;
       };

       class WETGAS_TARGET {
       public:
           static const std::string itemName;
       };
   };



   class GCONPRI : public ParserKeyword {
   public:
       GCONPRI();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class OIL_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_OIL_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WAT_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_WAT_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_GAS_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIQ_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_LIQ_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RES_FLUID_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class RES_VOL_BALANCING_FRAC_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class WET_GAS_PROD_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_WET_GAS_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SURF_GAS_BALANCING_FRAC_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class SURF_WAT_BALANCING_FRAC_UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class PROCEDURE_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GCONPROD : public ParserKeyword {
   public:
       GCONPROD();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class CONTROL_MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OIL_TARGET {
       public:
           static const std::string itemName;
       };

       class WATER_TARGET {
       public:
           static const std::string itemName;
       };

       class GAS_TARGET {
       public:
           static const std::string itemName;
       };

       class LIQUID_TARGET {
       public:
           static const std::string itemName;
       };

       class EXCEED_PROC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RESPOND_TO_PARENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GUIDE_RATE {
       public:
           static const std::string itemName;
       };

       class GUIDE_RATE_DEF {
       public:
           static const std::string itemName;
       };

       class WATER_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIQUID_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RESERVOIR_FLUID_TARGET {
       public:
           static const std::string itemName;
       };

       class RESERVOIR_VOLUME_BALANCE {
       public:
           static const std::string itemName;
       };

       class WET_GAS_TARGET {
       public:
           static const std::string itemName;
       };

       class CALORIFIC_TARGET {
       public:
           static const std::string itemName;
       };

       class SURFACE_GAS_FRACTION {
       public:
           static const std::string itemName;
       };

       class SURFACE_WAT_FRACTION {
       public:
           static const std::string itemName;
       };

       class LINEAR_COMBINED_TARGET {
       public:
           static const std::string itemName;
       };

       class LIN_TARGET_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
       };
   };



   class GCONSALE : public ParserKeyword {
   public:
       GCONSALE();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class SALES_TARGET {
       public:
           static const std::string itemName;
       };

       class MAX_SALES_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MIN_SALES_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_PROC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GCONSUMP : public ParserKeyword {
   public:
       GCONSUMP();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class GAS_CONSUMP_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class GAS_IMPORT_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class NETWORK_NODE {
       public:
           static const std::string itemName;
       };
   };



   class GCONTOL : public ParserKeyword {
   public:
       GCONTOL();
       static const std::string keywordName;

       class TOLERANCE_FRACTION {
       public:
           static const std::string itemName;
       };

       class NUPCOL_VALUE {
       public:
           static const std::string itemName;
       };

       class TOLERANCE_FRACTION_INJ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class MAX_IT_INJ {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 5;
       };
   };



   class GCUTBACK : public ParserKeyword {
   public:
       GCUTBACK();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class WATER_CUT_UPPER_LIM {
       public:
           static const std::string itemName;
       };

       class GAS_OIL_UPPER_LIM {
       public:
           static const std::string itemName;
       };

       class GAS_LIQ_UPPER_LIM {
       public:
           static const std::string itemName;
       };

       class WAT_GAS_UPPER_LIM {
       public:
           static const std::string itemName;
       };

       class RATE_CUTBACK_FACTOR {
       public:
           static const std::string itemName;
       };

       class CONTROL_PHASE {
       public:
           static const std::string itemName;
       };
   };



   class GCUTBACT : public ParserKeyword {
   public:
       GCUTBACT();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class RATE_CUTBACK {
       public:
           static const std::string itemName;
       };

       class CONTROL_PHASE {
       public:
           static const std::string itemName;
       };

       class TRACER {
       public:
           static const std::string itemName;
       };

       class UPPER_RATE_LIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class LOWER_RATE_LIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class UPPER_CONC_LIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class LOWER_CONC_LIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };
   };



   class GCVD : public ParserKeyword {
   public:
       GCVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class GDCQ : public ParserKeyword {
   public:
       GDCQ();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class INIT_DCQ {
       public:
           static const std::string itemName;
       };

       class DCQ_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GDCQECON : public ParserKeyword {
   public:
       GDCQECON();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class MIN_DCQ {
       public:
           static const std::string itemName;
       };
   };



   class GDFILE : public ParserKeyword {
   public:
       GDFILE();
       static const std::string keywordName;

       class filename {
       public:
           static const std::string itemName;
       };

       class formatted {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GDIMS : public ParserKeyword {
   public:
       GDIMS();
       static const std::string keywordName;

       class MAX_NUM_GRAD_PARAMS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class GDORIENT : public ParserKeyword {
   public:
       GDORIENT();
       static const std::string keywordName;

       class I {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class K {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class Z {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class HAND {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GDRILPOT : public ParserKeyword {
   public:
       GDRILPOT();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class QNTY_TYPE {
       public:
           static const std::string itemName;
       };

       class MIN_POTENTIAL_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GECON : public ParserKeyword {
   public:
       GECON();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class MIN_OIL_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MIN_GAS_RATE {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_WCT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_GOR {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_WATER_GAS_RATIO {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class WORKOVER {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class END_RUN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_OPEN_WELLS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class GECONT : public ParserKeyword {
   public:
       GECONT();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class END_RUN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_OPEN_WELLS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
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



   class GEFAC : public ParserKeyword {
   public:
       GEFAC();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class EFFICIENCY_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TRANSFER_EXT_NET {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GETDATA : public ParserKeyword {
   public:
       GETDATA();
       static const std::string keywordName;

       class FILENAME {
       public:
           static const std::string itemName;
       };

       class FORMATTED {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ZNAME {
       public:
           static const std::string itemName;
       };

       class ZALT {
       public:
           static const std::string itemName;
       };
   };



   class GETGLOB : public ParserKeyword {
   public:
       GETGLOB();
       static const std::string keywordName;
   };



   class GI : public ParserKeyword {
   public:
       GI();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class GIALL : public ParserKeyword {
   public:
       GIALL();
       static const std::string keywordName;

       class PRESSURE {
       public:
           static const std::string itemName;
       };

       class TABLE {
       public:
           static const std::string itemName;
       };
   };



   class GIMODEL : public ParserKeyword {
   public:
       GIMODEL();
       static const std::string keywordName;
   };



   class GINODE : public ParserKeyword {
   public:
       GINODE();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class GLIFTLIM : public ParserKeyword {
   public:
       GLIFTLIM();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class MAX_LIFT_CAPACITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_NUM_WELL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class GLIFTOPT : public ParserKeyword {
   public:
       GLIFTOPT();
       static const std::string keywordName;

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class MAX_LIFT_GAS_SUPPLY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -100000000000000000000.000000;
       };

       class MAX_TOTAL_GAS_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -100000000000000000000.000000;
       };
   };



   class GMWSET : public ParserKeyword {
   public:
       GMWSET();
       static const std::string keywordName;
   };



   class GNETDP : public ParserKeyword {
   public:
       GNETDP();
       static const std::string keywordName;

       class FIXED_PRESSURE_GROUP {
       public:
           static const std::string itemName;
       };

       class PHASE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIN_RATE_TRIGGER {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_RATE_TRIGGER {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class PRESSURE_INCR_SUBTRACT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PRESSURE_INCR_ADD {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MIN_ALLOW_PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_ALLOW_PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };
   };



   class GNETINJE : public ParserKeyword {
   public:
       GNETINJE();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class GNETPUMP : public ParserKeyword {
   public:
       GNETPUMP();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PROD_RATE_SWITCH {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PHASE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NEW_VFT_TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 4;
       };

       class NEW_ARTIFICIAL_LIFT_QNTY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class NEW_GAS_CONUMPTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GPMAINT : public ParserKeyword {
   public:
       GPMAINT();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class FLOW_TARGET {
       public:
           static const std::string itemName;
       };

       class REGION {
       public:
           static const std::string itemName;
       };

       class FIP_FAMILY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PRESSURE_TARGET {
       public:
           static const std::string itemName;
       };

       class PROP_CONSTANT {
       public:
           static const std::string itemName;
       };

       class TIME_CONSTANT {
       public:
           static const std::string itemName;
       };
   };



   class GRADGRUP : public ParserKeyword {
   public:
       GRADGRUP();
       static const std::string keywordName;

       class MNENONIC {
       public:
           static const std::string itemName;
       };
   };



   class GRADRESV : public ParserKeyword {
   public:
       GRADRESV();
       static const std::string keywordName;

       class MNENONIC {
       public:
           static const std::string itemName;
       };
   };



   class GRADRFT : public ParserKeyword {
   public:
       GRADRFT();
       static const std::string keywordName;

       class MNENONIC {
       public:
           static const std::string itemName;
       };
   };



   class GRADWELL : public ParserKeyword {
   public:
       GRADWELL();
       static const std::string keywordName;

       class MNENONIC {
       public:
           static const std::string itemName;
       };
   };



   class GRAVCONS : public ParserKeyword {
   public:
       GRAVCONS();
       static const std::string keywordName;

       class MNENONIC {
       public:
           static const std::string itemName;
       };
   };



   class GRAVDR : public ParserKeyword {
   public:
       GRAVDR();
       static const std::string keywordName;
   };



   class GRAVDRB : public ParserKeyword {
   public:
       GRAVDRB();
       static const std::string keywordName;
   };



   class GRAVDRM : public ParserKeyword {
   public:
       GRAVDRM();
       static const std::string keywordName;

       class ALLOW_RE_INFL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRAVITY : public ParserKeyword {
   public:
       GRAVITY();
       static const std::string keywordName;

       class API_GRAVITY {
       public:
           static const std::string itemName;
       };

       class WATER_SP_GRAVITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class GAS_SP_GRAVITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.777300;
       };
   };



   class GRDREACH : public ParserKeyword {
   public:
       GRDREACH();
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

       class BRANCH_NAME {
       public:
           static const std::string itemName;
       };

       class DISTANCE_TO_START {
       public:
           static const std::string itemName;
       };

       class DISTANCE_TO_END {
       public:
           static const std::string itemName;
       };

       class RCH_CONNECT_TO {
       public:
           static const std::string itemName;
       };

       class PENETRATION_DIRECTION {
       public:
           static const std::string itemName;
       };

       class GRID_BLOCK_COORD {
       public:
           static const std::string itemName;
       };

       class CONTACT_AREA {
       public:
           static const std::string itemName;
       };

       class TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class PRODUCTIVITY_INDEX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class LENGTH_DEAD_GRID_BLOCK {
       public:
           static const std::string itemName;
       };

       class OPTION_CONNECT_REACH {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ADJUSTMENT_REACH {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class REMOVE_CAP_PRESSURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class INFILTR_EQ {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class HYDRAULIC_CONDUCTIVITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class RIVER_BED_THICKNESS {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GRID : public ParserKeyword {
   public:
       GRID();
       static const std::string keywordName;
   };



   class GRIDFILE : public ParserKeyword {
   public:
       GRIDFILE();
       static const std::string keywordName;

       class GRID {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class EGRID {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class GRIDOPTS : public ParserKeyword {
   public:
       GRIDOPTS();
       static const std::string keywordName;

       class TRANMULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NRMULT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class NRPINC {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class GRIDUNIT : public ParserKeyword {
   public:
       GRIDUNIT();
       static const std::string keywordName;

       class LengthUnit {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAP {
       public:
           static const std::string itemName;
       };
   };



   class GROUP_PROBE : public ParserKeyword {
   public:
       GROUP_PROBE();
       static const std::string keywordName;

       class GROUPS {
       public:
           static const std::string itemName;
       };
   };



   class GROUP_PROBE_OPM : public ParserKeyword {
   public:
       GROUP_PROBE_OPM();
       static const std::string keywordName;

       class GROUPS {
       public:
           static const std::string itemName;
       };
   };



   class GRUPMAST : public ParserKeyword {
   public:
       GRUPMAST();
       static const std::string keywordName;

       class MASTER_GROUP {
       public:
           static const std::string itemName;
       };

       class SLAVE_RESERVOIR {
       public:
           static const std::string itemName;
       };

       class SLAVE_GROUP {
       public:
           static const std::string itemName;
       };

       class LIMITING_FRACTION {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };
   };



   class GRUPNET : public ParserKeyword {
   public:
       GRUPNET();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class TERMINAL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ALQ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class SUB_SEA_MANIFOLD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ADD_GAS_LIFT_GAS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ALQ_SURFACE_DENSITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRUPRIG : public ParserKeyword {
   public:
       GRUPRIG();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class WORKOVER_RIG_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class DRILLING_RIG_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ADD_OR_REMOVE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRUPSLAV : public ParserKeyword {
   public:
       GRUPSLAV();
       static const std::string keywordName;

       class SLAVE_GROUP {
       public:
           static const std::string itemName;
       };

       class MASTER_GROUP {
       public:
           static const std::string itemName;
       };

       class OIL_PROD_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WAT_PROD_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_PROD_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLUID_VOL_PROD_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OIL_INJ_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WAT_INJ_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_INJ_CONSTRAINTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRUPTARG : public ParserKeyword {
   public:
       GRUPTARG();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class TARGET {
       public:
           static const std::string itemName;
       };

       class NEW_VALUE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 999999999999999949387135297074018866963645011013410073083904.000000;
       };
   };



   class GRUPTREE : public ParserKeyword {
   public:
       GRUPTREE();
       static const std::string keywordName;

       class CHILD_GROUP {
       public:
           static const std::string itemName;
       };

       class PARENT_GROUP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GSATINJE : public ParserKeyword {
   public:
       GSATINJE();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class SURF_INJ_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class RES_INJ_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MEAN_CALORIFIC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GSATPROD : public ParserKeyword {
   public:
       GSATPROD();
       static const std::string keywordName;

       class SATELLITE_GROUP_NAME_OR_GROUP_NAME_ROOT {
       public:
           static const std::string itemName;
       };

       class OIL_PRODUCTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class WATER_PRODUCTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GAS_PRODUCTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class RES_FLUID_VOL_PRODUCTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class LIFT_GAS_SUPPLY_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MEAN_CALORIFIC_VALUE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class GSEPCOND : public ParserKeyword {
   public:
       GSEPCOND();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class SEPARATOR {
       public:
           static const std::string itemName;
       };
   };



   class GSF : public ParserKeyword {
   public:
       GSF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class GSSCPTST : public ParserKeyword {
   public:
       GSSCPTST();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class CONTROL_MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TARGET_PROD_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class TARGET_PROD_PERIOD {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAX_PROD_RATE_FLAG {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class CONV_TOLERANCE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MAT_IT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 6;
       };

       class SUB_GRP_CONTROL_FLAG {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class GSWINGF : public ParserKeyword {
   public:
       GSWINGF();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class SWING_JAN {
       public:
           static const std::string itemName;
       };

       class SWING_FEB {
       public:
           static const std::string itemName;
       };

       class SWING_MAR {
       public:
           static const std::string itemName;
       };

       class SWING_APR {
       public:
           static const std::string itemName;
       };

       class SWING_MAY {
       public:
           static const std::string itemName;
       };

       class SWING_JUN {
       public:
           static const std::string itemName;
       };

       class SWING_JUL {
       public:
           static const std::string itemName;
       };

       class SWING_AUG {
       public:
           static const std::string itemName;
       };

       class SWING_SEP {
       public:
           static const std::string itemName;
       };

       class SWING_OCT {
       public:
           static const std::string itemName;
       };

       class SWING_NOV {
       public:
           static const std::string itemName;
       };

       class SWING_DEC {
       public:
           static const std::string itemName;
       };

       class PROFILE_JAN {
       public:
           static const std::string itemName;
       };

       class PROFILE_FEB {
       public:
           static const std::string itemName;
       };

       class PROFILE_MAR {
       public:
           static const std::string itemName;
       };

       class PROFILE_APR {
       public:
           static const std::string itemName;
       };

       class PROFILE_MAY {
       public:
           static const std::string itemName;
       };

       class PROFILE_JUN {
       public:
           static const std::string itemName;
       };

       class PROFILE_JUL {
       public:
           static const std::string itemName;
       };

       class PROFILE_AUG {
       public:
           static const std::string itemName;
       };

       class PROFILE_SEP {
       public:
           static const std::string itemName;
       };

       class PROFILE_OCT {
       public:
           static const std::string itemName;
       };

       class PROFILE_NOV {
       public:
           static const std::string itemName;
       };

       class PROFILE_DEC {
       public:
           static const std::string itemName;
       };
   };



   class GTADD : public ParserKeyword {
   public:
       GTADD();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class TARGET {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class NUM_ADDITIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class GTMULT : public ParserKeyword {
   public:
       GTMULT();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class TARGET {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class NUM_MULT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class GUIDECAL : public ParserKeyword {
   public:
       GUIDECAL();
       static const std::string keywordName;

       class COEFF_A {
       public:
           static const std::string itemName;
       };

       class COEFF_B {
       public:
           static const std::string itemName;
       };
   };



   class GUIDERAT : public ParserKeyword {
   public:
       GUIDERAT();
       static const std::string keywordName;

       class MIN_CALC_TIME {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class NOMINATED_PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
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

       class ALLOW_INCREASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DAMPING_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class USE_FREE_GAS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIN_GUIDE_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };
   };



   class GUPFREQ : public ParserKeyword {
   public:
       GUPFREQ();
       static const std::string keywordName;

       class UPDATE_FREQ_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GWRTWCV : public ParserKeyword {
   public:
       GWRTWCV();
       static const std::string keywordName;

       class WELLS {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
