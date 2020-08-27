#ifndef PARSER_KEYWORDS_H_HPP
#define PARSER_KEYWORDS_H_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class HALFTRAN : public ParserKeyword {
   public:
       HALFTRAN();
       static const std::string keywordName;
   };



   class HAxxxxxx : public ParserKeyword {
   public:
       HAxxxxxx();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HBNUM : public ParserKeyword {
   public:
       HBNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HDISP : public ParserKeyword {
   public:
       HDISP();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class HEATCR : public ParserKeyword {
   public:
       HEATCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HEATCRT : public ParserKeyword {
   public:
       HEATCRT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HMAQUCT : public ParserKeyword {
   public:
       HMAQUCT();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DERIVATIES_RESP_PERM_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DERIVATIES_RESP_OPEN_ANGLE_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DERIVATIES_RESP_AQUIFER_DEPTH {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class HMAQUFET : public ParserKeyword {
   public:
       HMAQUFET();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DERIVATIES_RESP_WAT_VOL_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DERIVATIES_RESP_AQUIFER_PROD_INDEX_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DERIVATIES_RESP_AQUIFER_DEPTH {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class HMAQUNUM : public ParserKeyword {
   public:
       HMAQUNUM();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class DERIVATIES_RESP_PORE_VOL_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DERIVATIES_RESP_AQUIFER_PERM_MULT {
       public:
           static const std::string itemName;
       };

       class DERIVATIES_RESP_AQUIFER_GRID_CON_TRANS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class HMDIMS : public ParserKeyword {
   public:
       HMDIMS();
       static const std::string keywordName;

       class MAX_GRAD_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_SUB_REGIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_GRADS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_FAULTS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_AQUIFER_PARAMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WELL_PARAMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UNUSED {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ROCK_GRAD_PARAMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WELL_CONN_PARAMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class HMFAULTS : public ParserKeyword {
   public:
       HMFAULTS();
       static const std::string keywordName;

       class FAULT_SEGMENT {
       public:
           static const std::string itemName;
       };
   };



   class HMMLAQUN : public ParserKeyword {
   public:
       HMMLAQUN();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class AQUIFER_PORE_VOL_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_PORE_PERM_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_GRID_CONN_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class HMMLCTAQ : public ParserKeyword {
   public:
       HMMLCTAQ();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class AQUIFER_PERM_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_ANGLE_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_DEPTH_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class HMMLFTAQ : public ParserKeyword {
   public:
       HMMLFTAQ();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class AQUIFER_WAT_VOL_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_PROD_INDEX_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class AQUIFER_DEPTH_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class HMMLTWCN : public ParserKeyword {
   public:
       HMMLTWCN();
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
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class CTF {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class HMMULTFT : public ParserKeyword {
   public:
       HMMULTFT();
       static const std::string keywordName;

       class FAULT {
       public:
           static const std::string itemName;
       };

       class TRANS_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DIFF_MULT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class HMMULTSG : public ParserKeyword {
   public:
       HMMULTSG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HMMULTxx : public ParserKeyword {
   public:
       HMMULTxx();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HMPROPS : public ParserKeyword {
   public:
       HMPROPS();
       static const std::string keywordName;
   };



   class HMROCK : public ParserKeyword {
   public:
       HMROCK();
       static const std::string keywordName;

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };

       class CALCULATE_GRADIENTS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class HMROCKT : public ParserKeyword {
   public:
       HMROCKT();
       static const std::string keywordName;

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };

       class CALCULATE_GRADIENTS_1 {
       public:
           static const std::string itemName;
       };

       class CALCULATE_GRADIENTS_2 {
       public:
           static const std::string itemName;
       };
   };



   class HMRREF : public ParserKeyword {
   public:
       HMRREF();
       static const std::string keywordName;

       class P_REF {
       public:
           static const std::string itemName;
       };

       class P_DIM {
       public:
           static const std::string itemName;
       };
   };



   class HMWELCON : public ParserKeyword {
   public:
       HMWELCON();
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
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class REQ_TRANS_FACTOR_GRAD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REQ_SKIN_FACTOR_GRAD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class HMWPIMLT : public ParserKeyword {
   public:
       HMWPIMLT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };
   };



   class HMxxxxxx : public ParserKeyword {
   public:
       HMxxxxxx();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HRFIN : public ParserKeyword {
   public:
       HRFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWKRO : public ParserKeyword {
   public:
       HWKRO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWKRORG : public ParserKeyword {
   public:
       HWKRORG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWKRORW : public ParserKeyword {
   public:
       HWKRORW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWKRW : public ParserKeyword {
   public:
       HWKRW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWKRWR : public ParserKeyword {
   public:
       HWKRWR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWPCW : public ParserKeyword {
   public:
       HWPCW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSNUM : public ParserKeyword {
   public:
       HWSNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSOGCR : public ParserKeyword {
   public:
       HWSOGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSOWCR : public ParserKeyword {
   public:
       HWSOWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSWCR : public ParserKeyword {
   public:
       HWSWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSWL : public ParserKeyword {
   public:
       HWSWL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSWLPC : public ParserKeyword {
   public:
       HWSWLPC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HWSWU : public ParserKeyword {
   public:
       HWSWU();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HXFIN : public ParserKeyword {
   public:
       HXFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HYDRHEAD : public ParserKeyword {
   public:
       HYDRHEAD();
       static const std::string keywordName;

       class REF_DEPTH {
       public:
           static const std::string itemName;
       };

       class FRESHWATER_DENSITY {
       public:
           static const std::string itemName;
       };

       class REMOVE_DEPTH_TERMS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class HYFIN : public ParserKeyword {
   public:
       HYFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class HYMOBGDR : public ParserKeyword {
   public:
       HYMOBGDR();
       static const std::string keywordName;
   };



   class HYST : public ParserKeyword {
   public:
       HYST();
       static const std::string keywordName;
   };



   class HYSTCHCK : public ParserKeyword {
   public:
       HYSTCHCK();
       static const std::string keywordName;
   };



   class HZFIN : public ParserKeyword {
   public:
       HZFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
