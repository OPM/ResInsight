#ifndef PARSER_KEYWORDS_R_HPP
#define PARSER_KEYWORDS_R_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class RADFIN4 : public ParserKeyword {
   public:
       RADFIN4();
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

       class NR {
       public:
           static const std::string itemName;
       };

       class NTHETA {
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
           static const int defaultValue;
       };
   };



   class REGDIMS : public ParserKeyword {
   public:
       REGDIMS();
       static const std::string keywordName;

       class NTFIP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NMFIPR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRFREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTFREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ETRACK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTCREG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERNUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERATE_DWORK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_OPERATE_IWORK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPLMIX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class REGIONS : public ParserKeyword {
   public:
       REGIONS();
       static const std::string keywordName;
   };



   class REGION_PROBE : public ParserKeyword {
   public:
       REGION_PROBE();
       static const std::string keywordName;

       class REGIONS {
       public:
           static const std::string itemName;
       };
   };



   class RESTART : public ParserKeyword {
   public:
       RESTART();
       static const std::string keywordName;

       class ROOTNAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REPORTNUMBER {
       public:
           static const std::string itemName;
       };

       class SAVEFILE {
       public:
           static const std::string itemName;
       };

       class SAVEFILE_FORMAT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class RKTRMDIR : public ParserKeyword {
   public:
       RKTRMDIR();
       static const std::string keywordName;
   };



   class ROCK : public ParserKeyword {
   public:
       ROCK();
       static const std::string keywordName;

       class PREF {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ROCKCOMP : public ParserKeyword {
   public:
       ROCKCOMP();
       static const std::string keywordName;

       class HYSTERESIS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NTROCC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class WATER_COMPACTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class ROCKOPTS : public ParserKeyword {
   public:
       ROCKOPTS();
       static const std::string keywordName;

       class METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REF_PRESSURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TABLE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class HYST_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class ROCKTAB : public ParserKeyword {
   public:
       ROCKTAB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RPTGRID : public ParserKeyword {
   public:
       RPTGRID();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RPTONLY : public ParserKeyword {
   public:
       RPTONLY();
       static const std::string keywordName;
   };



   class RPTONLYO : public ParserKeyword {
   public:
       RPTONLYO();
       static const std::string keywordName;
   };



   class RPTPROPS : public ParserKeyword {
   public:
       RPTPROPS();
       static const std::string keywordName;

       class mnemonics {
       public:
           static const std::string itemName;
       };
   };



   class RPTREGS : public ParserKeyword {
   public:
       RPTREGS();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTRST : public ParserKeyword {
   public:
       RPTRST();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTRUNSP : public ParserKeyword {
   public:
       RPTRUNSP();
       static const std::string keywordName;
   };



   class RPTSCHED : public ParserKeyword {
   public:
       RPTSCHED();
       static const std::string keywordName;

       class MNEMONIC_LIST {
       public:
           static const std::string itemName;
       };
   };



   class RPTSOL : public ParserKeyword {
   public:
       RPTSOL();
       static const std::string keywordName;

       class mnemonics {
       public:
           static const std::string itemName;
       };
   };



   class RS : public ParserKeyword {
   public:
       RS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RSVD : public ParserKeyword {
   public:
       RSVD();
       static const std::string keywordName;

       class table {
       public:
           static const std::string itemName;
       };
   };



   class RTEMPVD : public ParserKeyword {
   public:
       RTEMPVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class RUNSPEC : public ParserKeyword {
   public:
       RUNSPEC();
       static const std::string keywordName;
   };



   class RUNSUM : public ParserKeyword {
   public:
       RUNSUM();
       static const std::string keywordName;
   };



   class RV : public ParserKeyword {
   public:
       RV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class RVVD : public ParserKeyword {
   public:
       RVVD();
       static const std::string keywordName;

       class table {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
