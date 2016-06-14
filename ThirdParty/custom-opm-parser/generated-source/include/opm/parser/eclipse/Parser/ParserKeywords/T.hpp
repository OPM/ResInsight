#ifndef PARSER_KEYWORDS_T_HPP
#define PARSER_KEYWORDS_T_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class TABDIMS : public ParserKeyword {
   public:
       TABDIMS();
       static const std::string keywordName;

       class NTSFUN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTPVT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NSSFUN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPPVT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTFIP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRPVT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_RV_NODES {
       public:
           static const std::string itemName;
       };

       class NTENDP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_STATE_EQ {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_EOS_RES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_EOS_SURFACE {
       public:
           static const std::string itemName;
       };

       class MAX_FLUX_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_THERMAL_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTROCC {
       public:
           static const std::string itemName;
       };

       class MAX_PRESSURE_MAINTAINANCE_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_KVALUE_TABLES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTALPHA {
       public:
           static const std::string itemName;
       };

       class ASPHALTENE_ASPKDAM_MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ASPHALTENE_ASPREWG_MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ASPHALTENE_ASPVISO_MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM20_NOT_USED {
       public:
           static const std::string itemName;
       };

       class ASPHALTENE_ASPPW2D_MAX_COLUMNS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ASPHALTENE_ASPPW2D_MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ASPHALTENE_ASPWETF_MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_KVALUE_TABLES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class RESERVED {
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



   class TEMPVD : public ParserKeyword {
   public:
       TEMPVD();
       static const std::string keywordName;

       class DATA {
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



   class THERMAL : public ParserKeyword {
   public:
       THERMAL();
       static const std::string keywordName;
   };



   class THERMEX1 : public ParserKeyword {
   public:
       THERMEX1();
       static const std::string keywordName;

       class EXPANSION_COEFF {
       public:
           static const std::string itemName;
           static const double defaultValue;
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



   class TOPS : public ParserKeyword {
   public:
       TOPS();
       static const std::string keywordName;

       class data {
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



   class TRACERS : public ParserKeyword {
   public:
       TRACERS();
       static const std::string keywordName;

       class MAX_OIL_TRACERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WATER_TRACERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_GAS_TRACERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ENV_TRACERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUMERIC_DIFF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_ITER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MIN_ITER {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
           static const double defaultValue;
       };

       class TSMAXZ {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TSMINZ {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TSMCHP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TSFMAX {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TSFMIN {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TSFCNV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TFDIFF {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THRUPT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TMAXWC {
       public:
           static const std::string itemName;
       };

       class TRGTTE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGCNV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGMBE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGLCV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class XXXTTE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class XXXCNV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class XXXMBE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class XXXLCV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class XXXWFL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGFIP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGSFT {
       public:
           static const std::string itemName;
       };

       class THIONX {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRWGHT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NEWTMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NEWTMN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LITMAX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LITMIN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXWSIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXWPIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DDPLIM {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DDSLIM {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRGDPR {
       public:
           static const std::string itemName;
           static const double defaultValue;
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

       class table {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
