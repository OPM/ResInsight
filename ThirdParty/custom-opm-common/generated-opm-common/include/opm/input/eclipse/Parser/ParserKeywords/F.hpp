#ifndef PARSER_KEYWORDS_F_HPP
#define PARSER_KEYWORDS_F_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class FAULTDIM : public ParserKeyword {
   public:
       FAULTDIM();
       static const std::string keywordName;

       class MFSEGS {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class FAULTS : public ParserKeyword {
   public:
       FAULTS();
       static const std::string keywordName;

       class NAME {
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

       class IY1 {
       public:
           static const std::string itemName;
       };

       class IY2 {
       public:
           static const std::string itemName;
       };

       class IZ1 {
       public:
           static const std::string itemName;
       };

       class IZ2 {
       public:
           static const std::string itemName;
       };

       class FACE {
       public:
           static const std::string itemName;
       };
   };



   class FBHPDEF : public ParserKeyword {
   public:
       FBHPDEF();
       static const std::string keywordName;

       class TARGET_BHP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.013250;
       };

       class LIMIT_BHP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 6895.000000;
       };
   };



   class FHERCHBL : public ParserKeyword {
   public:
       FHERCHBL();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FIELD : public ParserKeyword {
   public:
       FIELD();
       static const std::string keywordName;
   };



   class FIELDSEP : public ParserKeyword {
   public:
       FIELDSEP();
       static const std::string keywordName;

       class STAGE {
       public:
           static const std::string itemName;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 15.560000;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.013250;
       };

       class LIQ_DESTINATION {
       public:
           static const std::string itemName;
       };

       class VAP_DESTINATION {
       public:
           static const std::string itemName;
       };

       class KVALUE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class EOS_NUM {
       public:
           static const std::string itemName;
       };

       class REF_TEMP {
       public:
           static const std::string itemName;
       };

       class REF_PRESS {
       public:
           static const std::string itemName;
       };
   };



   class FIELD_PROBE : public ParserKeyword {
   public:
       FIELD_PROBE();
       static const std::string keywordName;
   };



   class FIELD_PROBE_OPM : public ParserKeyword {
   public:
       FIELD_PROBE_OPM();
       static const std::string keywordName;
   };



   class FILEUNIT : public ParserKeyword {
   public:
       FILEUNIT();
       static const std::string keywordName;

       class FILE_UNIT_SYSTEM {
       public:
           static const std::string itemName;
       };
   };



   class FILLEPS : public ParserKeyword {
   public:
       FILLEPS();
       static const std::string keywordName;
   };



   class FIPNUM : public ParserKeyword {
   public:
       FIPNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FIPOWG : public ParserKeyword {
   public:
       FIPOWG();
       static const std::string keywordName;
   };



   class FIPSEP : public ParserKeyword {
   public:
       FIPSEP();
       static const std::string keywordName;

       class FLUID_IN_PLACE_REGION {
       public:
           static const std::string itemName;
       };

       class STAGE_INDEX {
       public:
           static const std::string itemName;
       };

       class STAGE_TEMPERATURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 15.560000;
       };

       class STAGE_PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.013250;
       };

       class DESTINATION_OUPUT {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class DESTINATION_STAGE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class K_VAL_TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class GAS_PLANT_TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class SURF_EQ_STATE_NUM {
       public:
           static const std::string itemName;
       };

       class DENSITY_EVAL_GAS_TEMP {
       public:
           static const std::string itemName;
       };

       class DENSITY_EVAL_PRESSURE_TEMP {
       public:
           static const std::string itemName;
       };
   };



   class FIP_PROBE : public ParserKeyword {
   public:
       FIP_PROBE();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FLUXNUM : public ParserKeyword {
   public:
       FLUXNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FLUXREG : public ParserKeyword {
   public:
       FLUXREG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FLUXTYPE : public ParserKeyword {
   public:
       FLUXTYPE();
       static const std::string keywordName;

       class BC_TYPE {
       public:
           static const std::string itemName;
       };
   };



   class FMTHMD : public ParserKeyword {
   public:
       FMTHMD();
       static const std::string keywordName;
   };



   class FMTIN : public ParserKeyword {
   public:
       FMTIN();
       static const std::string keywordName;
   };



   class FMTOUT : public ParserKeyword {
   public:
       FMTOUT();
       static const std::string keywordName;
   };



   class FMWSET : public ParserKeyword {
   public:
       FMWSET();
       static const std::string keywordName;
   };



   class FOAM : public ParserKeyword {
   public:
       FOAM();
       static const std::string keywordName;
   };



   class FOAMADS : public ParserKeyword {
   public:
       FOAMADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMDCYO : public ParserKeyword {
   public:
       FOAMDCYO();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMDCYW : public ParserKeyword {
   public:
       FOAMDCYW();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMFCN : public ParserKeyword {
   public:
       FOAMFCN();
       static const std::string keywordName;

       class CAPILLARY_NUMBER {
       public:
           static const std::string itemName;
       };

       class EXP {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class FOAMFRM : public ParserKeyword {
   public:
       FOAMFRM();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMFSC : public ParserKeyword {
   public:
       FOAMFSC();
       static const std::string keywordName;

       class REF_SURF_CONC {
       public:
           static const std::string itemName;
       };

       class EXPONENT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class MIN_SURF_CONC {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-20;
       };

       class MIN_WAT_SAT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1e-06;
       };
   };



   class FOAMFSO : public ParserKeyword {
   public:
       FOAMFSO();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMFST : public ParserKeyword {
   public:
       FOAMFST();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMFSW : public ParserKeyword {
   public:
       FOAMFSW();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMMOB : public ParserKeyword {
   public:
       FOAMMOB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMMOBP : public ParserKeyword {
   public:
       FOAMMOBP();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMMOBS : public ParserKeyword {
   public:
       FOAMMOBS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class FOAMOPTS : public ParserKeyword {
   public:
       FOAMOPTS();
       static const std::string keywordName;

       class TRANSPORT_PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class FOAMROCK : public ParserKeyword {
   public:
       FOAMROCK();
       static const std::string keywordName;

       class ADSORPTION_INDEX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class ROCK_DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class FORMFEED : public ParserKeyword {
   public:
       FORMFEED();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class FRICTION : public ParserKeyword {
   public:
       FRICTION();
       static const std::string keywordName;

       class NWFRIC {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class NWFRIB {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class FULLIMP : public ParserKeyword {
   public:
       FULLIMP();
       static const std::string keywordName;
   };



}
}
#endif
