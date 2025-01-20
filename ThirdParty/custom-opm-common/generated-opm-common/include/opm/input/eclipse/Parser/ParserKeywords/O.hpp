#ifndef PARSER_KEYWORDS_O_HPP
#define PARSER_KEYWORDS_O_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class OCOMPIDX : public ParserKeyword {
   public:
       OCOMPIDX();
       static const std::string keywordName;

       class OIL_COMPONENT_INDEX {
       public:
           static const std::string itemName;
       };
   };



   class OFM : public ParserKeyword {
   public:
       OFM();
       static const std::string keywordName;
   };



   class OIL : public ParserKeyword {
   public:
       OIL();
       static const std::string keywordName;
   };



   class OILAPI : public ParserKeyword {
   public:
       OILAPI();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class OILCOMPR : public ParserKeyword {
   public:
       OILCOMPR();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
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



   class OILDENT : public ParserKeyword {
   public:
       OILDENT();
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



   class OILJT : public ParserKeyword {
   public:
       OILJT();
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



   class OILMW : public ParserKeyword {
   public:
       OILMW();
       static const std::string keywordName;

       class MOLAR_WEIGHT {
       public:
           static const std::string itemName;
       };
   };



   class OILVISCT : public ParserKeyword {
   public:
       OILVISCT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class OILVTIM : public ParserKeyword {
   public:
       OILVTIM();
       static const std::string keywordName;

       class INTERPOLATION_METHOD {
       public:
           static const std::string itemName;
       };
   };



   class OLDTRAN : public ParserKeyword {
   public:
       OLDTRAN();
       static const std::string keywordName;
   };



   class OLDTRANR : public ParserKeyword {
   public:
       OLDTRANR();
       static const std::string keywordName;
   };



   class OPERATE : public ParserKeyword {
   public:
       OPERATE();
       static const std::string keywordName;

       class TARGET_ARRAY {
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

       class OPERATION {
       public:
           static const std::string itemName;
       };

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class PARAM1 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PARAM2 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class OPERATER : public ParserKeyword {
   public:
       OPERATER();
       static const std::string keywordName;

       class TARGET_ARRAY {
       public:
           static const std::string itemName;
       };

       class REGION_NUMBER {
       public:
           static const std::string itemName;
       };

       class OPERATION {
       public:
           static const std::string itemName;
       };

       class ARRAY_PARAMETER {
       public:
           static const std::string itemName;
       };

       class PARAM1 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PARAM2 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class REGION_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class OPERNUM : public ParserKeyword {
   public:
       OPERNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class OPTIONS : public ParserKeyword {
   public:
       OPTIONS();
       static const std::string keywordName;

       class ITEM1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM3 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM5 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM6 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM7 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM8 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM9 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM10 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM11 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM12 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM13 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM14 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM15 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM16 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM17 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM18 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM19 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM20 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM21 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM22 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM23 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM24 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM25 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM26 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM27 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM28 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM29 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM30 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM31 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM32 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM33 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM34 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM35 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM36 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM37 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM38 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM39 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM40 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM41 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM42 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM43 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM44 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM45 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM46 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM47 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM48 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM49 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM50 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM51 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM52 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM53 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM54 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM55 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM56 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM57 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM58 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM59 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM60 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM61 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM62 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM63 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM64 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM65 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM66 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM67 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM68 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM69 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM70 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM71 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM72 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM73 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM74 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM75 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM76 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM77 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM78 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM79 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM80 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM81 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM82 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM83 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM84 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM85 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM86 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM87 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM88 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM89 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM90 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM91 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM92 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM93 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM94 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM95 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM96 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM97 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM98 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM99 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM100 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM101 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM102 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM103 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM104 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM105 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM106 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM107 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM108 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM109 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM110 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM111 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM112 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM113 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM114 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM115 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM116 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM117 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM118 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM119 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM120 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM121 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM122 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM123 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM124 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM125 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM126 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM127 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM128 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM129 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM130 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM131 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM132 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM133 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM134 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM135 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM136 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM137 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM138 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM139 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM140 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM141 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM142 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM143 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM144 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM145 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM146 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM147 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM148 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM149 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM150 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM151 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM152 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM153 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM154 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM155 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM156 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM157 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM158 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM159 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM160 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM161 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM162 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM163 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM164 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM165 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM166 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM167 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM168 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM169 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM170 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM171 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM172 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM173 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM174 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM175 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM176 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM177 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM178 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM179 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM180 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM181 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM182 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM183 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM184 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM185 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM186 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM187 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM188 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM189 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM190 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM191 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM192 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM193 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM194 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM195 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM196 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM197 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM198 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM199 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM200 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM201 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM202 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM203 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM204 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM205 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM206 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM207 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM208 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM209 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM210 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM211 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM212 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM213 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM214 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM215 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM216 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM217 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM218 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM219 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM220 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM221 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM222 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM223 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM224 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM225 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM226 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM227 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM228 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM229 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM230 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM231 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM232 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM233 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM234 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM235 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM236 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM237 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM238 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM239 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM240 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM241 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM242 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM243 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM244 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM245 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM246 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM247 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM248 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM249 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM250 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM251 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM252 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM253 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM254 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM255 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM256 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM257 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM258 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM259 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM260 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM261 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM262 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM263 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM264 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM265 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM266 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM267 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM268 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM269 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM270 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM271 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM272 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM273 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM274 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM275 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM276 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM277 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM278 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM279 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM280 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM281 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM282 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM283 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM284 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM285 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM286 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM287 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM288 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM289 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM290 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM291 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM292 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM293 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM294 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM295 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM296 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM297 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM298 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM299 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM300 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM301 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM302 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM303 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM304 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM305 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM306 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM307 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM308 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM309 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM310 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM311 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM312 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM313 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM314 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM315 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM316 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM317 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM318 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM319 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class OPTIONS3 : public ParserKeyword {
   public:
       OPTIONS3();
       static const std::string keywordName;

       class ITEM1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM3 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM5 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM6 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM7 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM8 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM9 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM10 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM11 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM12 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM13 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM14 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM15 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM16 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM17 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM18 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM19 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM20 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM21 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM22 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM23 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM24 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM25 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM26 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM27 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM28 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM29 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM30 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM31 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM32 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM33 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM34 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM35 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM36 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM37 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM38 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM39 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM40 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM41 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM42 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM43 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM44 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM45 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM46 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM47 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM48 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM49 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM50 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM51 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM52 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM53 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM54 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM55 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM56 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM57 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM58 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM59 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM60 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM61 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM62 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM63 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM64 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM65 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM66 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM67 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM68 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM69 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM70 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM71 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM72 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM73 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM74 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM75 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM76 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM77 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM78 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM79 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM80 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM81 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM82 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM83 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM84 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM85 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM86 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM87 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM88 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM89 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM90 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM91 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM92 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM93 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM94 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM95 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM96 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM97 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM98 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM99 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM100 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM101 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM102 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM103 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM104 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM105 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM106 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM107 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM108 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM109 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM110 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM111 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM112 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM113 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM114 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM115 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM116 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM117 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM118 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM119 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM120 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM121 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM122 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM123 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM124 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM125 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM126 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM127 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM128 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM129 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM130 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM131 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM132 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM133 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM134 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM135 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM136 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM137 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM138 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM139 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM140 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM141 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM142 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM143 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM144 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM145 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM146 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM147 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM148 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM149 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM150 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM151 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM152 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM153 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM154 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM155 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM156 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM157 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM158 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM159 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM160 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM161 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM162 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM163 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM164 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM165 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM166 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM167 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM168 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM169 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM170 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM171 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM172 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM173 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM174 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM175 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM176 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM177 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM178 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM179 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM180 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM181 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM182 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM183 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM184 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM185 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM186 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM187 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM188 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM189 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM190 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM191 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM192 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM193 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM194 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM195 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM196 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM197 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM198 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM199 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM200 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM201 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM202 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM203 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM204 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM205 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM206 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM207 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM208 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM209 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM210 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM211 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM212 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM213 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM214 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM215 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM216 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM217 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM218 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM219 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM220 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM221 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM222 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM223 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM224 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM225 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM226 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM227 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM228 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM229 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM230 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM231 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM232 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM233 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM234 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM235 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM236 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM237 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM238 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM239 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM240 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM241 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM242 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM243 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM244 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM245 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM246 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM247 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM248 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM249 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM250 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM251 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM252 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM253 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM254 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM255 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM256 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM257 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM258 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM259 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM260 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM261 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM262 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM263 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM264 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM265 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM266 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM267 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM268 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM269 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM270 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM271 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM272 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM273 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM274 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM275 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM276 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM277 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM278 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM279 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM280 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM281 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM282 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM283 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM284 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM285 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM286 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM287 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ITEM288 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };
   };



   class OUTRAD : public ParserKeyword {
   public:
       OUTRAD();
       static const std::string keywordName;

       class RADIUS {
       public:
           static const std::string itemName;
       };
   };



   class OUTSOL : public ParserKeyword {
   public:
       OUTSOL();
       static const std::string keywordName;
   };



   class OVERBURD : public ParserKeyword {
   public:
       OVERBURD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
