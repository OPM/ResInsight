#ifndef PARSER_KEYWORDS_B_HPP
#define PARSER_KEYWORDS_B_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class BC : public ParserKeyword {
   public:
       BC();
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

       class DIRECTION {
       public:
           static const std::string itemName;
       };

       class TYPE {
       public:
           static const std::string itemName;
       };

       class COMPONENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };
   };



   class BCCON : public ParserKeyword {
   public:
       BCCON();
       static const std::string keywordName;

       class INDEX {
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

       class DIRECTION {
       public:
           static const std::string itemName;
       };
   };



   class BCPROP : public ParserKeyword {
   public:
       BCPROP();
       static const std::string keywordName;

       class INDEX {
       public:
           static const std::string itemName;
       };

       class TYPE {
       public:
           static const std::string itemName;
       };

       class COMPONENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
       };

       class MECHTYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FIXEDX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class FIXEDY {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class FIXEDZ {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class STRESSXX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class STRESSYY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class STRESSZZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class DISPX {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class DISPY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class DISPZ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class BDENSITY : public ParserKeyword {
   public:
       BDENSITY();
       static const std::string keywordName;

       class BRINE_DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class BGGI : public ParserKeyword {
   public:
       BGGI();
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



   class BIC : public ParserKeyword {
   public:
       BIC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class BIGMODEL : public ParserKeyword {
   public:
       BIGMODEL();
       static const std::string keywordName;
   };



   class BIOTCOEF : public ParserKeyword {
   public:
       BIOTCOEF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class BLACKOIL : public ParserKeyword {
   public:
       BLACKOIL();
       static const std::string keywordName;
   };



   class BLOCK_PROBE : public ParserKeyword {
   public:
       BLOCK_PROBE();
       static const std::string keywordName;

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



   class BLOCK_PROBE300 : public ParserKeyword {
   public:
       BLOCK_PROBE300();
       static const std::string keywordName;

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



   class BOGI : public ParserKeyword {
   public:
       BOGI();
       static const std::string keywordName;

       class OIL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class BOUNDARY : public ParserKeyword {
   public:
       BOUNDARY();
       static const std::string keywordName;

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

       class ORIENTATION_INDEX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };

       class DUAL_PORO_FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class BOX : public ParserKeyword {
   public:
       BOX();
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
   };



   class BPARA : public ParserKeyword {
   public:
       BPARA();
       static const std::string keywordName;
   };



   class BPIDIMS : public ParserKeyword {
   public:
       BPIDIMS();
       static const std::string keywordName;

       class MXNBIP {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class MXNLBI {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 1;
       };
   };



   class BRANPROP : public ParserKeyword {
   public:
       BRANPROP();
       static const std::string keywordName;

       class DOWNTREE_NODE {
       public:
           static const std::string itemName;
       };

       class UPTREE_NODE {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
       };

       class ALQ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class ALQ_SURFACE_DENSITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class BRINE : public ParserKeyword {
   public:
       BRINE();
       static const std::string keywordName;

       class SALTS {
       public:
           static const std::string itemName;
       };
   };



   class BTOBALFA : public ParserKeyword {
   public:
       BTOBALFA();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class BTOBALFV : public ParserKeyword {
   public:
       BTOBALFV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
