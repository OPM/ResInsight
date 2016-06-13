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



   class PBVD : public ParserKeyword {
   public:
       PBVD();
       static const std::string keywordName;

       class table {
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



   class PERFORMANCE_PROBE : public ParserKeyword {
   public:
       PERFORMANCE_PROBE();
       static const std::string keywordName;
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



   class PLYDHFLF : public ParserKeyword {
   public:
       PLYDHFLF();
       static const std::string keywordName;

       class DATA {
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



   class PLYVISC : public ParserKeyword {
   public:
       PLYVISC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
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



   class PROPS : public ParserKeyword {
   public:
       PROPS();
       static const std::string keywordName;
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



   class PVDG : public ParserKeyword {
   public:
       PVDG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PVDO : public ParserKeyword {
   public:
       PVDO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class PVDS : public ParserKeyword {
   public:
       PVDS();
       static const std::string keywordName;

       class data {
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



}
}
#endif
