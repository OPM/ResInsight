#ifndef PARSER_KEYWORDS_P_HPP
#define PARSER_KEYWORDS_P_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class PARALLEL : public ParserKeyword {
   public:
       PARALLEL();
       static const std::string keywordName;

       class NDMAIN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MACHINE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PARAOPTS : public ParserKeyword {
   public:
       PARAOPTS();
       static const std::string keywordName;

       class METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SET_PRINT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class SIZE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_BUFFERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VALUE_MEM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VALUE_COARSE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VALUE_NNC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VALUE_PRT_FILE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class RESERVED {
       public:
           static const std::string itemName;
       };
   };



   class PARTTRAC : public ParserKeyword {
   public:
       PARTTRAC();
       static const std::string keywordName;

       class NPARTT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NKPTMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPKPMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PATHS : public ParserKeyword {
   public:
       PATHS();
       static const std::string keywordName;

       class PathName {
       public:
           static const std::string itemName;
       };

       class PathValue {
       public:
           static const std::string itemName;
       };
   };



   class PBUB : public ParserKeyword {
   public:
       PBUB();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PBVD : public ParserKeyword {
   public:
       PBVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PCG : public ParserKeyword {
   public:
       PCG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PCG32D : public ParserKeyword {
   public:
       PCG32D();
       static const std::string keywordName;

       class SOME_DATA {
       public:
           static const std::string itemName;
       };
   };



   class PCW : public ParserKeyword {
   public:
       PCW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PCW32D : public ParserKeyword {
   public:
       PCW32D();
       static const std::string keywordName;

       class SOME_DATA {
       public:
           static const std::string itemName;
       };
   };



   class PDEW : public ParserKeyword {
   public:
       PDEW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PDVD : public ParserKeyword {
   public:
       PDVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PEBI : public ParserKeyword {
   public:
       PEBI();
       static const std::string keywordName;

       class NEG_TRANSMISSIBILITIES {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class AVOID_GRID_CALC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PECOEFS : public ParserKeyword {
   public:
       PECOEFS();
       static const std::string keywordName;

       class WAT_SALINITY {
       public:
           static const std::string itemName;
       };

       class TEMP {
       public:
           static const std::string itemName;
       };

       class MINERAL_DENSITY {
       public:
           static const std::string itemName;
       };

       class PHI_EFF_0 {
       public:
           static const std::string itemName;
       };

       class PHI_EFF_1 {
       public:
           static const std::string itemName;
       };

       class C_0 {
       public:
           static const std::string itemName;
       };

       class C_K {
       public:
           static const std::string itemName;
       };

       class SHEAR_MOD {
       public:
           static const std::string itemName;
       };

       class ALPHA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class E {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class METHOD {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PEDIMS : public ParserKeyword {
   public:
       PEDIMS();
       static const std::string keywordName;

       class NUM_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_PRESSURE_POINTS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PEGTABX : public ParserKeyword {
   public:
       PEGTABX();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PEKTABX : public ParserKeyword {
   public:
       PEKTABX();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PENUM : public ParserKeyword {
   public:
       PENUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERFORMANCE_PROBE : public ParserKeyword {
   public:
       PERFORMANCE_PROBE();
       static const std::string keywordName;
   };



   class PERMAVE : public ParserKeyword {
   public:
       PERMAVE();
       static const std::string keywordName;

       class EXPO_0 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class EXPO_1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class EXPO_2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PERMFACT : public ParserKeyword {
   public:
       PERMFACT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PERMJFUN : public ParserKeyword {
   public:
       PERMJFUN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMR : public ParserKeyword {
   public:
       PERMR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMTHT : public ParserKeyword {
   public:
       PERMTHT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMX : public ParserKeyword {
   public:
       PERMX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMXY : public ParserKeyword {
   public:
       PERMXY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMY : public ParserKeyword {
   public:
       PERMY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PERMYZ : public ParserKeyword {
   public:
       PERMYZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PERMZ : public ParserKeyword {
   public:
       PERMZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PERMZX : public ParserKeyword {
   public:
       PERMZX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PETGRID : public ParserKeyword {
   public:
       PETGRID();
       static const std::string keywordName;

       class FILE_NAME {
       public:
           static const std::string itemName;
       };
   };



   class PETOPTS : public ParserKeyword {
   public:
       PETOPTS();
       static const std::string keywordName;

       class OPTIONS {
       public:
           static const std::string itemName;
       };
   };



   class PICOND : public ParserKeyword {
   public:
       PICOND();
       static const std::string keywordName;

       class MAX_INTERVAL_BELOW_DEWPOINT {
       public:
           static const std::string itemName;
       };

       class MAX_INTERVAL_ABOVE_DEWPOINT {
       public:
           static const std::string itemName;
       };

       class D_F {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class INCLUDE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class F_L {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class F_U {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DELTA_WAT_SAT {
       public:
           static const std::string itemName;
       };

       class DELTA_PRESSURE {
       public:
           static const std::string itemName;
       };

       class DELTA_FRAC_COMP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_DELTA_TIME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ADAPTIVE_ORD_CONTROL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ADAPTIVE_ORD_MIN_SPACING {
       public:
           static const std::string itemName;
       };
   };



   class PIMTDIMS : public ParserKeyword {
   public:
       PIMTDIMS();
       static const std::string keywordName;

       class NTPIMT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPPIMT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PIMULTAB : public ParserKeyword {
   public:
       PIMULTAB();
       static const std::string keywordName;

       class TABLE {
       public:
           static const std::string itemName;
       };
   };



   class PINCH : public ParserKeyword {
   public:
       PINCH();
       static const std::string keywordName;

       class THRESHOLD_THICKNESS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CONTROL_OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_EMPTY_GAP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PINCHOUT_OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MULTZ_OPTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PINCHNUM : public ParserKeyword {
   public:
       PINCHNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PINCHOUT : public ParserKeyword {
   public:
       PINCHOUT();
       static const std::string keywordName;
   };



   class PINCHREG : public ParserKeyword {
   public:
       PINCHREG();
       static const std::string keywordName;

       class THRESHOLD_THICKNESS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OPTION1 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_GAP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OPTION2 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OPTION3 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PINCHXY : public ParserKeyword {
   public:
       PINCHXY();
       static const std::string keywordName;

       class THRESHOLD_XR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THRESHOLD_YTHETA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PINTDIMS : public ParserKeyword {
   public:
       PINTDIMS();
       static const std::string keywordName;

       class NTSKWAT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTSKPOLY {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTPMWINJ {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PLMIXNUM : public ParserKeyword {
   public:
       PLMIXNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PLMIXPAR : public ParserKeyword {
   public:
       PLMIXPAR();
       static const std::string keywordName;

       class TODD_LONGSTAFF {
       public:
           static const std::string itemName;
       };
   };



   class PLYADS : public ParserKeyword {
   public:
       PLYADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYADSS : public ParserKeyword {
   public:
       PLYADSS();
       static const std::string keywordName;

       class POLYMER_C {
       public:
           static const std::string itemName;
       };

       class POLYMER_ADS_C {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYATEMP : public ParserKeyword {
   public:
       PLYATEMP();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PLYCAMAX : public ParserKeyword {
   public:
       PLYCAMAX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PLYDHFLF : public ParserKeyword {
   public:
       PLYDHFLF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYESAL : public ParserKeyword {
   public:
       PLYESAL();
       static const std::string keywordName;

       class ALPHAP {
       public:
           static const std::string itemName;
       };
   };



   class PLYKRRF : public ParserKeyword {
   public:
       PLYKRRF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PLYMAX : public ParserKeyword {
   public:
       PLYMAX();
       static const std::string keywordName;

       class MAX_POLYMER_CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class MAX_SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class PLYMWINJ : public ParserKeyword {
   public:
       PLYMWINJ();
       static const std::string keywordName;

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };

       class THROUGHPUT {
       public:
           static const std::string itemName;
       };

       class VELOCITY {
       public:
           static const std::string itemName;
       };

       class MOLECULARWEIGHT {
       public:
           static const std::string itemName;
       };
   };



   class PLYOPTS : public ParserKeyword {
   public:
       PLYOPTS();
       static const std::string keywordName;

       class MIN_SWAT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PLYRMDEN : public ParserKeyword {
   public:
       PLYRMDEN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PLYROCK : public ParserKeyword {
   public:
       PLYROCK();
       static const std::string keywordName;

       class IPV {
       public:
           static const std::string itemName;
       };

       class RRF {
       public:
           static const std::string itemName;
       };

       class ROCK_DENSITY {
       public:
           static const std::string itemName;
       };

       class AI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_ADSORPTION {
       public:
           static const std::string itemName;
       };
   };



   class PLYROCKM : public ParserKeyword {
   public:
       PLYROCKM();
       static const std::string keywordName;

       class IPV {
       public:
           static const std::string itemName;
       };

       class RRF {
       public:
           static const std::string itemName;
       };

       class ROCK_DENSITY {
       public:
           static const std::string itemName;
       };

       class AI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_ADSORPTION {
       public:
           static const std::string itemName;
       };
   };



   class PLYSHEAR : public ParserKeyword {
   public:
       PLYSHEAR();
       static const std::string keywordName;

       class WATER_VELOCITY {
       public:
           static const std::string itemName;
       };

       class VRF {
       public:
           static const std::string itemName;
       };
   };



   class PLYSHLOG : public ParserKeyword {
   public:
       PLYSHLOG();
       static const std::string keywordName;

       class REF_POLYMER_CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class REF_SALINITY {
       public:
           static const std::string itemName;
       };

       class REF_TEMPERATURE {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYTRRF : public ParserKeyword {
   public:
       PLYTRRF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYTRRFA : public ParserKeyword {
   public:
       PLYTRRFA();
       static const std::string keywordName;

       class NBTRRF {
       public:
           static const std::string itemName;
       };
   };



   class PLYVISC : public ParserKeyword {
   public:
       PLYVISC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYVISCS : public ParserKeyword {
   public:
       PLYVISCS();
       static const std::string keywordName;

       class PC {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYVISCT : public ParserKeyword {
   public:
       PLYVISCT();
       static const std::string keywordName;

       class PC {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PLYVMH : public ParserKeyword {
   public:
       PLYVMH();
       static const std::string keywordName;

       class K_MH {
       public:
           static const std::string itemName;
       };

       class A_MH {
       public:
           static const std::string itemName;
       };

       class GAMMA {
       public:
           static const std::string itemName;
       };

       class KAPPA {
       public:
           static const std::string itemName;
       };
   };



   class PLYVSCST : public ParserKeyword {
   public:
       PLYVSCST();
       static const std::string keywordName;

       class PC1 {
       public:
           static const std::string itemName;
       };

       class MULT {
       public:
           static const std::string itemName;
       };
   };



   class PMAX : public ParserKeyword {
   public:
       PMAX();
       static const std::string keywordName;

       class MAX_PRESSURE {
       public:
           static const std::string itemName;
       };

       class MAX_PRESSURE_CHECK {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_PRESSURE_CHECK {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class NUM_NODES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class PMISC : public ParserKeyword {
   public:
       PMISC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class POLYMER : public ParserKeyword {
   public:
       POLYMER();
       static const std::string keywordName;
   };



   class POLYMW : public ParserKeyword {
   public:
       POLYMW();
       static const std::string keywordName;
   };



   class PORO : public ParserKeyword {
   public:
       PORO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PORV : public ParserKeyword {
   public:
       PORV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PPCWMAX : public ParserKeyword {
   public:
       PPCWMAX();
       static const std::string keywordName;

       class MAXIMUM_CAPILLARY_PRESSURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MODIFY_CONNATE_SATURATION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PRECSALT : public ParserKeyword {
   public:
       PRECSALT();
       static const std::string keywordName;
   };



   class PREF : public ParserKeyword {
   public:
       PREF();
       static const std::string keywordName;

       class PRESSURE {
       public:
           static const std::string itemName;
       };
   };



   class PREFS : public ParserKeyword {
   public:
       PREFS();
       static const std::string keywordName;

       class PRESSURE {
       public:
           static const std::string itemName;
       };
   };



   class PRESSURE : public ParserKeyword {
   public:
       PRESSURE();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PRIORITY : public ParserKeyword {
   public:
       PRIORITY();
       static const std::string keywordName;

       class MIN_CALC_TIME {
       public:
           static const std::string itemName;
       };

       class A1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class B1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class C1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class D1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class E1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class F1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class G1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class H1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class A2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class B2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class C2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class D2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class E2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class F2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class G2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class H2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PROPS : public ParserKeyword {
   public:
       PROPS();
       static const std::string keywordName;
   };



   class PRORDER : public ParserKeyword {
   public:
       PRORDER();
       static const std::string keywordName;

       class NO1 {
       public:
           static const std::string itemName;
       };

       class NO2 {
       public:
           static const std::string itemName;
       };

       class NO3 {
       public:
           static const std::string itemName;
       };

       class NO4 {
       public:
           static const std::string itemName;
       };

       class NO5 {
       public:
           static const std::string itemName;
       };

       class OPT1 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OPT2 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OPT3 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OPT4 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OPT5 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PRVD : public ParserKeyword {
   public:
       PRVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PSTEADY : public ParserKeyword {
   public:
       PSTEADY();
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

       class DIFF {
       public:
           static const std::string itemName;
       };

       class PRESSURE_TOL {
       public:
           static const std::string itemName;
       };

       class OIL_TOL {
       public:
           static const std::string itemName;
       };

       class WATER_TOL {
       public:
           static const std::string itemName;
       };

       class GAS_TOL {
       public:
           static const std::string itemName;
       };

       class BRINE_TOL {
       public:
           static const std::string itemName;
       };

       class MAX_TIME {
       public:
           static const std::string itemName;
       };

       class MIN_TIME {
       public:
           static const std::string itemName;
       };

       class PIM_AQUIFERS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class PSWRG : public ParserKeyword {
   public:
       PSWRG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PSWRO : public ParserKeyword {
   public:
       PSWRO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PVCDO : public ParserKeyword {
   public:
       PVCDO();
       static const std::string keywordName;

       class P_REF {
       public:
           static const std::string itemName;
       };

       class OIL_VOL_FACTOR {
       public:
           static const std::string itemName;
       };

       class OIL_COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };

       class OIL_VISCOSITY {
       public:
           static const std::string itemName;
       };

       class OIL_VISCOSIBILITY {
       public:
           static const std::string itemName;
       };
   };



   class PVCO : public ParserKeyword {
   public:
       PVCO();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVDG : public ParserKeyword {
   public:
       PVDG();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVDO : public ParserKeyword {
   public:
       PVDO();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVDS : public ParserKeyword {
   public:
       PVDS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVTG : public ParserKeyword {
   public:
       PVTG();
       static const std::string keywordName;

       class GAS_PRESSURE {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVTGW : public ParserKeyword {
   public:
       PVTGW();
       static const std::string keywordName;

       class GAS_PRESSURE {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVTGWO : public ParserKeyword {
   public:
       PVTGWO();
       static const std::string keywordName;

       class GAS_PRESSURE {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVTNUM : public ParserKeyword {
   public:
       PVTNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PVTO : public ParserKeyword {
   public:
       PVTO();
       static const std::string keywordName;

       class RS {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVTW : public ParserKeyword {
   public:
       PVTW();
       static const std::string keywordName;

       class P_REF {
       public:
           static const std::string itemName;
       };

       class WATER_VOL_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_COMPRESSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_VISCOSITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_VISCOSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class PVTWSALT : public ParserKeyword {
   public:
       PVTWSALT();
       static const std::string keywordName;

       class P_REF {
       public:
           static const std::string itemName;
       };

       class SALT_CONCENTRATION_REF {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class PVT_M : public ParserKeyword {
   public:
       PVT_M();
       static const std::string keywordName;
   };



   class PVZG : public ParserKeyword {
   public:
       PVZG();
       static const std::string keywordName;

       class REF_TEMP {
       public:
           static const std::string itemName;
       };

       class table {
       public:
           static const std::string itemName;
       };
   };



   class PYACTION : public ParserKeyword {
   public:
       PYACTION();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class RUN_COUNT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FILENAME {
       public:
           static const std::string itemName;
       };
   };



   class PYINPUT : public ParserKeyword {
   public:
       PYINPUT();
       static const std::string keywordName;

       class code {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
