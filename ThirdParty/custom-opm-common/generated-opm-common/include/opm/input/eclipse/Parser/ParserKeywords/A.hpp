#ifndef PARSER_KEYWORDS_A_HPP
#define PARSER_KEYWORDS_A_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class ACF : public ParserKeyword {
   public:
       ACF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ACTCO2S : public ParserKeyword {
   public:
       ACTCO2S();
       static const std::string keywordName;

       class ACTIVITY_MODEL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 3;
       };
   };



   class ACTDIMS : public ParserKeyword {
   public:
       ACTDIMS();
       static const std::string keywordName;

       class MAX_ACTION {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 2;
       };

       class MAX_ACTION_LINES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 50;
       };

       class MAX_ACTION_LINE_CHARACTERS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 80;
       };

       class MAX_ACTION_COND {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 3;
       };
   };



   class ACTION : public ParserKeyword {
   public:
       ACTION();
       static const std::string keywordName;

       class ACTION_NAME {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class OPERATOR {
       public:
           static const std::string itemName;
       };

       class TRIGGER_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class ACTIONG : public ParserKeyword {
   public:
       ACTIONG();
       static const std::string keywordName;

       class ACTION {
       public:
           static const std::string itemName;
       };

       class GROUP_NAME {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class OPERATOR {
       public:
           static const std::string itemName;
       };

       class TRIGGER_VALUE {
       public:
           static const std::string itemName;
       };

       class REPETITIONS {
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



   class ACTIONR : public ParserKeyword {
   public:
       ACTIONR();
       static const std::string keywordName;

       class ACTION {
       public:
           static const std::string itemName;
       };

       class FLUID_IN_PLACE_NR {
       public:
           static const std::string itemName;
       };

       class FLUID_IN_PLACE_REG_FAM {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class OPERATOR {
       public:
           static const std::string itemName;
       };

       class TRIGGER_VALUE {
       public:
           static const std::string itemName;
       };

       class REPETITIONS {
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



   class ACTIONS : public ParserKeyword {
   public:
       ACTIONS();
       static const std::string keywordName;

       class ACTION {
       public:
           static const std::string itemName;
       };

       class WELL {
       public:
           static const std::string itemName;
       };

       class WELL_SEGMENT {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class OPERATOR {
       public:
           static const std::string itemName;
       };

       class TRIGGER_VALUE {
       public:
           static const std::string itemName;
       };

       class REPETITIONS {
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



   class ACTIONW : public ParserKeyword {
   public:
       ACTIONW();
       static const std::string keywordName;

       class ACTION {
       public:
           static const std::string itemName;
       };

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class OPERATOR {
       public:
           static const std::string itemName;
       };

       class TRIGGER_VALUE {
       public:
           static const std::string itemName;
       };

       class REPETITIONS {
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



   class ACTIONX : public ParserKeyword {
   public:
       ACTIONX();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MIN_WAIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class CONDITION {
       public:
           static const std::string itemName;
       };
   };



   class ACTNUM : public ParserKeyword {
   public:
       ACTNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ACTPARAM : public ParserKeyword {
   public:
       ACTPARAM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.000100;
       };
   };



   class ADD : public ParserKeyword {
   public:
       ADD();
       static const std::string keywordName;

       class field {
       public:
           static const std::string itemName;
       };

       class shift {
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



   class ADDREG : public ParserKeyword {
   public:
       ADDREG();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class SHIFT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
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



   class ADDZCORN : public ParserKeyword {
   public:
       ADDZCORN();
       static const std::string keywordName;

       class ADDED_VALUE {
       public:
           static const std::string itemName;
       };

       class IX1 {
       public:
           static const std::string itemName;
       };

       class IX2 {
       public:
           static const std::string itemName;
       };

       class JY1 {
       public:
           static const std::string itemName;
       };

       class JY2 {
       public:
           static const std::string itemName;
       };

       class KZ1 {
       public:
           static const std::string itemName;
       };

       class KZ2 {
       public:
           static const std::string itemName;
       };

       class IX1A {
       public:
           static const std::string itemName;
       };

       class IX2A {
       public:
           static const std::string itemName;
       };

       class JY1A {
       public:
           static const std::string itemName;
       };

       class JY2A {
       public:
           static const std::string itemName;
       };

       class ACTION {
       public:
           static const std::string itemName;
       };
   };



   class ADSALNOD : public ParserKeyword {
   public:
       ADSALNOD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ADSORP : public ParserKeyword {
   public:
       ADSORP();
       static const std::string keywordName;

       class ADSORBING_COMP {
       public:
           static const std::string itemName;
       };

       class ADORPTION_ISOTHERM {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class A1 {
       public:
           static const std::string itemName;
       };

       class A2 {
       public:
           static const std::string itemName;
       };

       class B {
       public:
           static const std::string itemName;
       };

       class M {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class N {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class K_REF {
       public:
           static const std::string itemName;
       };
   };



   class AITS : public ParserKeyword {
   public:
       AITS();
       static const std::string keywordName;
   };



   class AITSOFF : public ParserKeyword {
   public:
       AITSOFF();
       static const std::string keywordName;
   };



   class ALKADS : public ParserKeyword {
   public:
       ALKADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ALKALINE : public ParserKeyword {
   public:
       ALKALINE();
       static const std::string keywordName;
   };



   class ALKROCK : public ParserKeyword {
   public:
       ALKROCK();
       static const std::string keywordName;

       class ROCK_ADS_INDEX {
       public:
           static const std::string itemName;
       };
   };



   class ALL : public ParserKeyword {
   public:
       ALL();
       static const std::string keywordName;
   };



   class ALPOLADS : public ParserKeyword {
   public:
       ALPOLADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ALSURFAD : public ParserKeyword {
   public:
       ALSURFAD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ALSURFST : public ParserKeyword {
   public:
       ALSURFST();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class AMALGAM : public ParserKeyword {
   public:
       AMALGAM();
       static const std::string keywordName;

       class LGR_GROUPS {
       public:
           static const std::string itemName;
       };
   };



   class API : public ParserKeyword {
   public:
       API();
       static const std::string keywordName;
   };



   class APIGROUP : public ParserKeyword {
   public:
       APIGROUP();
       static const std::string keywordName;

       class MAX_OIL_PVT_GROUP_COUNT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class APILIM : public ParserKeyword {
   public:
       APILIM();
       static const std::string keywordName;

       class LIMITER {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SCOPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LOWER_API_LIMIT {
       public:
           static const std::string itemName;
       };

       class UPPER_API_LIMIT {
       public:
           static const std::string itemName;
       };

       class NUM_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };
   };



   class APIVD : public ParserKeyword {
   public:
       APIVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class AQANCONL : public ParserKeyword {
   public:
       AQANCONL();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class NAME {
       public:
           static const std::string itemName;
       };

       class LOWER_I {
       public:
           static const std::string itemName;
       };

       class UPPER_I {
       public:
           static const std::string itemName;
       };

       class LOWER_J {
       public:
           static const std::string itemName;
       };

       class UPPER_J {
       public:
           static const std::string itemName;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
       };

       class FACE_INDX {
       public:
           static const std::string itemName;
       };

       class AQUIFER_INFLUX_COEFF {
       public:
           static const std::string itemName;
       };

       class AQUIFER_INFLUX_MULT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class ALLOW {
       public:
           static const std::string itemName;
       };
   };



   class AQANNC : public ParserKeyword {
   public:
       AQANNC();
       static const std::string keywordName;

       class AQUIFER_NUMBER {
       public:
           static const std::string itemName;
       };

       class IX {
       public:
           static const std::string itemName;
       };

       class IY {
       public:
           static const std::string itemName;
       };

       class IZ {
       public:
           static const std::string itemName;
       };

       class AREA {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class AQANTRC : public ParserKeyword {
   public:
       AQANTRC();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class TRACER {
       public:
           static const std::string itemName;
       };

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class AQUALIST : public ParserKeyword {
   public:
       AQUALIST();
       static const std::string keywordName;

       class AQUIFER_LIST {
       public:
           static const std::string itemName;
       };

       class LIST {
       public:
           static const std::string itemName;
       };
   };



   class AQUANCON : public ParserKeyword {
   public:
       AQUANCON();
       static const std::string keywordName;

       class AQUIFER_ID {
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

       class FACE {
       public:
           static const std::string itemName;
       };

       class INFLUX_COEFF {
       public:
           static const std::string itemName;
       };

       class INFLUX_MULT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class CONNECT_ADJOINING_ACTIVE_CELL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class AQUCHGAS : public ParserKeyword {
   public:
       AQUCHGAS();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
       };

       class GAS_PRESSURE {
       public:
           static const std::string itemName;
       };

       class AQUIFER_PROD_INDEX {
       public:
           static const std::string itemName;
       };

       class TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };
   };



   class AQUCHWAT : public ParserKeyword {
   public:
       AQUCHWAT();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
       };

       class INPUT_4 {
       public:
           static const std::string itemName;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
       };

       class AQUIFER_PROD_INDEX {
       public:
           static const std::string itemName;
       };

       class TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class INIT_SALT_CONC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class MINIMUM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAXIMUM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class IGNORE_CAP_PRESSURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIN_FLOW_PR_CONN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class MAX_FLOW_PR_CONN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
       };

       class REMOVE_DEPTH_TERM {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class IMPORT_MAX_MIN_FLOW_RATE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };
   };



   class AQUCON : public ParserKeyword {
   public:
       AQUCON();
       static const std::string keywordName;

       class ID {
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

       class CONNECT_FACE {
       public:
           static const std::string itemName;
       };

       class TRANS_MULT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TRANS_OPTION {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ALLOW_INTERNAL_CELLS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class VEFRAC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class VEFRACP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class AQUCT : public ParserKeyword {
   public:
       AQUCT();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DAT_DEPTH {
       public:
           static const std::string itemName;
       };

       class P_INI {
       public:
           static const std::string itemName;
       };

       class PERM_AQ {
       public:
           static const std::string itemName;
       };

       class PORO_AQ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class C_T {
       public:
           static const std::string itemName;
       };

       class RAD {
       public:
           static const std::string itemName;
       };

       class THICKNESS_AQ {
       public:
           static const std::string itemName;
       };

       class INFLUENCE_ANGLE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 360.000000;
       };

       class TABLE_NUM_WATER_PRESS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class TABLE_NUM_INFLUENCE_FN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class INI_SALT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class TEMP_AQUIFER {
       public:
           static const std::string itemName;
       };
   };



   class AQUCWFAC : public ParserKeyword {
   public:
       AQUCWFAC();
       static const std::string keywordName;

       class ADD_TO_DEPTH {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class MULTIPLY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class AQUDIMS : public ParserKeyword {
   public:
       AQUDIMS();
       static const std::string keywordName;

       class MXNAQN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MXNAQC {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NIFTBL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NRIFTB {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 36;
       };

       class NANAQU {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NCAMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MXNALI {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MXAAQL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class AQUFET : public ParserKeyword {
   public:
       AQUFET();
       static const std::string keywordName;

       class DAT_DEPTH {
       public:
           static const std::string itemName;
       };

       class P0 {
       public:
           static const std::string itemName;
       };

       class V0 {
       public:
           static const std::string itemName;
       };

       class C_T {
       public:
           static const std::string itemName;
       };

       class PI {
       public:
           static const std::string itemName;
       };

       class TABLE_NUM_WATER_PRESS {
       public:
           static const std::string itemName;
       };

       class LOWER_I {
       public:
           static const std::string itemName;
       };

       class UPPER_I {
       public:
           static const std::string itemName;
       };

       class LOWER_J {
       public:
           static const std::string itemName;
       };

       class UPPER_J {
       public:
           static const std::string itemName;
       };

       class LOWER_K {
       public:
           static const std::string itemName;
       };

       class UPPER_K {
       public:
           static const std::string itemName;
       };

       class FACE_INDX {
       public:
           static const std::string itemName;
       };

       class SC_0 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class AQUFETP : public ParserKeyword {
   public:
       AQUFETP();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DAT_DEPTH {
       public:
           static const std::string itemName;
       };

       class P0 {
       public:
           static const std::string itemName;
       };

       class V0 {
       public:
           static const std::string itemName;
       };

       class C_T {
       public:
           static const std::string itemName;
       };

       class PI {
       public:
           static const std::string itemName;
       };

       class TABLE_NUM_WATER_PRESS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class SALINITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class TEMP {
       public:
           static const std::string itemName;
       };
   };



   class AQUFLUX : public ParserKeyword {
   public:
       AQUFLUX();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class FLUX {
       public:
           static const std::string itemName;
       };

       class SC_0 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class TEMP {
       public:
           static const std::string itemName;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
       };
   };



   class AQUIFER_PROBE_ANALYTIC : public ParserKeyword {
   public:
       AQUIFER_PROBE_ANALYTIC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class AQUIFER_PROBE_ANALYTIC_NAMED : public ParserKeyword {
   public:
       AQUIFER_PROBE_ANALYTIC_NAMED();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class AQUIFER_PROBE_NUMERIC : public ParserKeyword {
   public:
       AQUIFER_PROBE_NUMERIC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class AQUNNC : public ParserKeyword {
   public:
       AQUNNC();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class IX {
       public:
           static const std::string itemName;
       };

       class IY {
       public:
           static const std::string itemName;
       };

       class IZ {
       public:
           static const std::string itemName;
       };

       class JX {
       public:
           static const std::string itemName;
       };

       class JY {
       public:
           static const std::string itemName;
       };

       class JZ {
       public:
           static const std::string itemName;
       };

       class TRAN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class IST1 {
       public:
           static const std::string itemName;
       };

       class IST2 {
       public:
           static const std::string itemName;
       };

       class IPT1 {
       public:
           static const std::string itemName;
       };

       class IPT2 {
       public:
           static const std::string itemName;
       };

       class ZF1 {
       public:
           static const std::string itemName;
       };

       class ZF2 {
       public:
           static const std::string itemName;
       };

       class DIFF {
       public:
           static const std::string itemName;
       };
   };



   class AQUNUM : public ParserKeyword {
   public:
       AQUNUM();
       static const std::string keywordName;

       class AQUIFER_ID {
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

       class CROSS_SECTION {
       public:
           static const std::string itemName;
       };

       class LENGTH {
       public:
           static const std::string itemName;
       };

       class PORO {
       public:
           static const std::string itemName;
       };

       class PERM {
       public:
           static const std::string itemName;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };

       class INITIAL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class PVT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };
   };



   class AQUTAB : public ParserKeyword {
   public:
       AQUTAB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class AUTOCOAR : public ParserKeyword {
   public:
       AUTOCOAR();
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



   class AUTOREF : public ParserKeyword {
   public:
       AUTOREF();
       static const std::string keywordName;

       class NX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NY {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NZ {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class OPTION_TRANS_MULT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



}
}
#endif
