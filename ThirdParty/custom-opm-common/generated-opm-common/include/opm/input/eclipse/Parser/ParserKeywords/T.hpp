#ifndef PARSER_KEYWORDS_T_HPP
#define PARSER_KEYWORDS_T_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class TABDIMS : public ParserKeyword {
   public:
       TABDIMS();
       static const std::string keywordName;

       class NTSFUN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NTPVT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NSSFUN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 20;
       };

       class NPPVT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 20;
       };

       class NTFIP {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NRPVT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 20;
       };

       class MAX_RV_NODES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 20;
       };

       class NTENDP {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NUM_EOS_RES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NUM_EOS_SURFACE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MAX_FLUX_REGIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class MAX_THERMAL_REGIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NTROCC {
       public:
           static const std::string itemName;
       };

       class MAX_PRESSURE_MAINTAINANCE_REGIONS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MAX_KVALUE_TABLES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class NTALPHA {
       public:
           static const std::string itemName;
       };

       class ASPHALTENE_ASPKDAM_MAX_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class ASPHALTENE_ASPREWG_MAX_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class ASPHALTENE_ASPVISO_MAX_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class ITEM20_NOT_USED {
       public:
           static const std::string itemName;
       };

       class ASPHALTENE_ASPPW2D_MAX_COLUMNS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 5;
       };

       class ASPHALTENE_ASPPW2D_MAX_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 5;
       };

       class ASPHALTENE_ASPWETF_MAX_ROWS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 5;
       };

       class NUM_KVALUE_TABLES {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class RESERVED {
       public:
           static const std::string itemName;
       };
   };



   class TBLK : public ParserKeyword {
   public:
       TBLK();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TCRIT : public ParserKeyword {
   public:
       TCRIT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TEMP : public ParserKeyword {
   public:
       TEMP();
       static const std::string keywordName;
   };



   class TEMPI : public ParserKeyword {
   public:
       TEMPI();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TEMPNODE : public ParserKeyword {
   public:
       TEMPNODE();
       static const std::string keywordName;

       class TABLE_DATA {
       public:
           static const std::string itemName;
       };
   };



   class TEMPTVD : public ParserKeyword {
   public:
       TEMPTVD();
       static const std::string keywordName;
   };



   class TEMPVD : public ParserKeyword {
   public:
       TEMPVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class THCGAS : public ParserKeyword {
   public:
       THCGAS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THCO2MIX : public ParserKeyword {
   public:
       THCO2MIX();
       static const std::string keywordName;

       class MIXING_MODEL_SALT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIXING_MODEL_LIQUID {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MIXING_MODEL_GAS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class THCOIL : public ParserKeyword {
   public:
       THCOIL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THCONR : public ParserKeyword {
   public:
       THCONR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THCONSF : public ParserKeyword {
   public:
       THCONSF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THCROCK : public ParserKeyword {
   public:
       THCROCK();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THCWATER : public ParserKeyword {
   public:
       THCWATER();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THELCOEF : public ParserKeyword {
   public:
       THELCOEF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THERMAL : public ParserKeyword {
   public:
       THERMAL();
       static const std::string keywordName;
   };



   class THERMEXR : public ParserKeyword {
   public:
       THERMEXR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class THPRES : public ParserKeyword {
   public:
       THPRES();
       static const std::string keywordName;

       class REGION1 {
       public:
           static const std::string itemName;
       };

       class REGION2 {
       public:
           static const std::string itemName;
       };

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class THPRESFT : public ParserKeyword {
   public:
       THPRESFT();
       static const std::string keywordName;

       class FAULT_NAME {
       public:
           static const std::string itemName;
       };

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class TIGHTEN : public ParserKeyword {
   public:
       TIGHTEN();
       static const std::string keywordName;

       class FACTOR {
       public:
           static const std::string itemName;
       };
   };



   class TIGHTENP : public ParserKeyword {
   public:
       TIGHTENP();
       static const std::string keywordName;

       class LINEAR_FACTOR {
       public:
           static const std::string itemName;
       };

       class MAX_LINEAR {
       public:
           static const std::string itemName;
       };

       class NONLINEAR_FACTOR {
       public:
           static const std::string itemName;
       };

       class MAX_NONLINEAR {
       public:
           static const std::string itemName;
       };
   };



   class TIME : public ParserKeyword {
   public:
       TIME();
       static const std::string keywordName;

       class step_list {
       public:
           static const std::string itemName;
       };
   };



   class TITLE : public ParserKeyword {
   public:
       TITLE();
       static const std::string keywordName;

       class TitleText {
       public:
           static const std::string itemName;
       };
   };



   class TLMIXPAR : public ParserKeyword {
   public:
       TLMIXPAR();
       static const std::string keywordName;

       class TL_VISCOSITY_PARAMETER {
       public:
           static const std::string itemName;
       };

       class TL_DENSITY_PARAMETER {
       public:
           static const std::string itemName;
       };
   };



   class TLPMIXPA : public ParserKeyword {
   public:
       TLPMIXPA();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TNUM : public ParserKeyword {
   public:
       TNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TOLCRIT : public ParserKeyword {
   public:
       TOLCRIT();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };
   };



   class TOPS : public ParserKeyword {
   public:
       TOPS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TPAMEPS : public ParserKeyword {
   public:
       TPAMEPS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TPAMEPSS : public ParserKeyword {
   public:
       TPAMEPSS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TRACER : public ParserKeyword {
   public:
       TRACER();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class FLUID {
       public:
           static const std::string itemName;
       };

       class UNIT {
       public:
           static const std::string itemName;
       };

       class SOLUTION_PHASE {
       public:
           static const std::string itemName;
       };

       class NUM_PART_TABLE {
       public:
           static const std::string itemName;
       };

       class ADSORB_PHASE {
       public:
           static const std::string itemName;
       };
   };



   class TRACERKM : public ParserKeyword {
   public:
       TRACERKM();
       static const std::string keywordName;

       class TRACER_NAME {
       public:
           static const std::string itemName;
       };

       class PARTITION_FUNCTION_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PHASES {
       public:
           static const std::string itemName;
       };

       class TABLE {
       public:
           static const std::string itemName;
       };
   };



   class TRACERKP : public ParserKeyword {
   public:
       TRACERKP();
       static const std::string keywordName;

       class TABLE {
       public:
           static const std::string itemName;
       };
   };



   class TRACERS : public ParserKeyword {
   public:
       TRACERS();
       static const std::string keywordName;

       class MAX_OIL_TRACERS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MAX_WATER_TRACERS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MAX_GAS_TRACERS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class MAX_ENV_TRACERS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class NUMERIC_DIFF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_ITER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };

       class MIN_ITER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class PASSIVE_NONLINEAR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ONEOFF_LIN_TIGHT {
       public:
           static const std::string itemName;
       };

       class ONEOFF_NLIN_TIGHT {
       public:
           static const std::string itemName;
       };

       class TIGHTENING_FACTORS {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class NTIGHTFACTORS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class TRACITVD : public ParserKeyword {
   public:
       TRACITVD();
       static const std::string keywordName;

       class FLUX_LIMITER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class BOTH_TIMESTEP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class TRACTVD : public ParserKeyword {
   public:
       TRACTVD();
       static const std::string keywordName;
   };



   class TRADS : public ParserKeyword {
   public:
       TRADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TRANGL : public ParserKeyword {
   public:
       TRANGL();
       static const std::string keywordName;

       class IL {
       public:
           static const std::string itemName;
       };

       class JL {
       public:
           static const std::string itemName;
       };

       class KL {
       public:
           static const std::string itemName;
       };

       class IG {
       public:
           static const std::string itemName;
       };

       class JG {
       public:
           static const std::string itemName;
       };

       class KG {
       public:
           static const std::string itemName;
       };

       class TRAN {
       public:
           static const std::string itemName;
       };
   };



   class TRANR : public ParserKeyword {
   public:
       TRANR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRANTHT : public ParserKeyword {
   public:
       TRANTHT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRANX : public ParserKeyword {
   public:
       TRANX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRANY : public ParserKeyword {
   public:
       TRANY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRANZ : public ParserKeyword {
   public:
       TRANZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRDCY : public ParserKeyword {
   public:
       TRDCY();
       static const std::string keywordName;

       class HALF_TIME {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };
   };



   class TRDIF : public ParserKeyword {
   public:
       TRDIF();
       static const std::string keywordName;

       class HALF_TIME {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };
   };



   class TRDIS : public ParserKeyword {
   public:
       TRDIS();
       static const std::string keywordName;

       class D1TABLE {
       public:
           static const std::string itemName;
       };

       class D2TABLE {
       public:
           static const std::string itemName;
       };

       class D3TABLE {
       public:
           static const std::string itemName;
       };

       class D4TABLE {
       public:
           static const std::string itemName;
       };

       class D5TABLE {
       public:
           static const std::string itemName;
       };

       class D6TABLE {
       public:
           static const std::string itemName;
       };

       class D7TABLE {
       public:
           static const std::string itemName;
       };

       class D8TABLE {
       public:
           static const std::string itemName;
       };

       class D9TABLE {
       public:
           static const std::string itemName;
       };
   };



   class TREF : public ParserKeyword {
   public:
       TREF();
       static const std::string keywordName;

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };
   };



   class TREFS : public ParserKeyword {
   public:
       TREFS();
       static const std::string keywordName;

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };
   };



   class TRKPF : public ParserKeyword {
   public:
       TRKPF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class TRNHD : public ParserKeyword {
   public:
       TRNHD();
       static const std::string keywordName;
   };



   class TRPLPORO : public ParserKeyword {
   public:
       TRPLPORO();
       static const std::string keywordName;

       class NUM_MATRIX {
       public:
           static const std::string itemName;
       };
   };



   class TRROCK : public ParserKeyword {
   public:
       TRROCK();
       static const std::string keywordName;

       class ADSORPTION_INDEX {
       public:
           static const std::string itemName;
       };

       class MASS_DENSITY {
       public:
           static const std::string itemName;
       };

       class INIT_MODEL {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class TSTEP : public ParserKeyword {
   public:
       TSTEP();
       static const std::string keywordName;

       class step_list {
       public:
           static const std::string itemName;
       };
   };



   class TUNING : public ParserKeyword {
   public:
       TUNING();
       static const std::string keywordName;

       class TSINIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TSMAXZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 365.000000;
       };

       class TSMINZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TSMCHP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.150000;
       };

       class TSFMAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 3.000000;
       };

       class TSFMIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.300000;
       };

       class TSFCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TFDIFF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.250000;
       };

       class THRUPT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class TMAXWC {
       public:
           static const std::string itemName;
       };

       class TRGTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TRGCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-07;
       };

       class TRGLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.000100;
       };

       class XXXTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 10.000000;
       };

       class XXXCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class XXXMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };

       class XXXLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class XXXWFL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGFIP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.025000;
       };

       class TRGSFT {
       public:
           static const std::string itemName;
       };

       class THIONX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class TRWGHT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NEWTMX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };

       class NEWTMN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class LITMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 25;
       };

       class LITMIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MXWSIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class MXWPIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class DDPLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class DDSLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class TRGDPR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class XXXDPR {
       public:
           static const std::string itemName;
       };
   };



   class TUNINGDP : public ParserKeyword {
   public:
       TUNINGDP();
       static const std::string keywordName;

       class TRGLCV {
       public:
           static const std::string itemName;
       };

       class XXXLCV {
       public:
           static const std::string itemName;
       };

       class TRGDDP {
       public:
           static const std::string itemName;
       };

       class TRGDDS {
       public:
           static const std::string itemName;
       };
   };



   class TUNINGH : public ParserKeyword {
   public:
       TUNINGH();
       static const std::string keywordName;

       class GRGLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.000100;
       };

       class GXXLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class GMSLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-20;
       };

       class LGTMIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class LGTMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 25;
       };
   };



   class TUNINGL : public ParserKeyword {
   public:
       TUNINGL();
       static const std::string keywordName;

       class TSINIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TSMAXZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 365.000000;
       };

       class TSMINZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TSMCHP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.150000;
       };

       class TSFMAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 3.000000;
       };

       class TSFMIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.300000;
       };

       class TSFCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TFDIFF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.250000;
       };

       class THRUPT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class TMAXWC {
       public:
           static const std::string itemName;
       };

       class TRGTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TRGCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-07;
       };

       class TRGLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.000100;
       };

       class XXXTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 10.000000;
       };

       class XXXCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class XXXMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };

       class XXXLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class XXXWFL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGFIP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.025000;
       };

       class TRGSFT {
       public:
           static const std::string itemName;
       };

       class THIONX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class TRWGHT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NEWTMX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };

       class NEWTMN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class LITMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 25;
       };

       class LITMIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MXWSIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class MXWPIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class DDPLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class DDSLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class TRGDPR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class XXXDPR {
       public:
           static const std::string itemName;
       };
   };



   class TUNINGS : public ParserKeyword {
   public:
       TUNINGS();
       static const std::string keywordName;

       class LGR {
       public:
           static const std::string itemName;
       };

       class TSINIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TSMAXZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 365.000000;
       };

       class TSMINZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TSMCHP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.150000;
       };

       class TSFMAX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 3.000000;
       };

       class TSFMIN {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.300000;
       };

       class TSFCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TFDIFF {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.250000;
       };

       class THRUPT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class TMAXWC {
       public:
           static const std::string itemName;
       };

       class TRGTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class TRGCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-07;
       };

       class TRGLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.000100;
       };

       class XXXTTE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 10.000000;
       };

       class XXXCNV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class XXXMBE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };

       class XXXLCV {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class XXXWFL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.001000;
       };

       class TRGFIP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.025000;
       };

       class TRGSFT {
       public:
           static const std::string itemName;
       };

       class THIONX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class TRWGHT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class NEWTMX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };

       class NEWTMN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class LITMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 25;
       };

       class LITMIN {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class MXWSIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class MXWPIT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 8;
       };

       class DDPLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class DDSLIM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class TRGDPR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1000000.000000;
       };

       class XXXDPR {
       public:
           static const std::string itemName;
       };
   };



   class TVDP : public ParserKeyword {
   public:
       TVDP();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class TZONE : public ParserKeyword {
   public:
       TZONE();
       static const std::string keywordName;

       class OIL_SWITCH {
       public:
           static const std::string itemName;
       };

       class WATER_SWITCH {
       public:
           static const std::string itemName;
       };

       class GAS_SWITCH {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
