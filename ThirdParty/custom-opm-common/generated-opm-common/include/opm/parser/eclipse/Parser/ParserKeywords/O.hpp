#ifndef PARSER_KEYWORDS_O_HPP
#define PARSER_KEYWORDS_O_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
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
           static const double defaultValue;
       };

       class EXPANSION_COEFF_LINEAR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXPANSION_COEFF_QUADRATIC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class OILDENT : public ParserKeyword {
   public:
       OILDENT();
       static const std::string keywordName;

       class REFERENCE_TEMPERATURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXPANSION_COEFF_LINEAR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXPANSION_COEFF_QUADRATIC {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const double defaultValue;
       };

       class PARAM2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const double defaultValue;
       };

       class PARAM2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
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
           static const int defaultValue;
       };

       class ITEM2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM3 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM5 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM6 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM7 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM8 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM9 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM10 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM11 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM12 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM13 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM14 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM15 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM16 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM17 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM18 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM19 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM20 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM21 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM22 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM23 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM24 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM25 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM26 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM27 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM28 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM29 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM30 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM31 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM32 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM33 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM34 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM35 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM36 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM37 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM38 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM39 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM40 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM41 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM42 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM43 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM44 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM45 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM46 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM47 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM48 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM49 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM50 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM51 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM52 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM53 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM54 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM55 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM56 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM57 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM58 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM59 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM60 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM61 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM62 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM63 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM64 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM65 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM66 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM67 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM68 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM69 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM70 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM71 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM72 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM73 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM74 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM75 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM76 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM77 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM78 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM79 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM80 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM81 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM82 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM83 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM84 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM85 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM86 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM87 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM88 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM89 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM90 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM91 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM92 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM93 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM94 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM95 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM96 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM97 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM98 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM99 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM100 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM101 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM102 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM103 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM104 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM105 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM106 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM107 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM108 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM109 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM110 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM111 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM112 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM113 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM114 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM115 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM116 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM117 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM118 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM119 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM120 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM121 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM122 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM123 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM124 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM125 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM126 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM127 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM128 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM129 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM130 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM131 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM132 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM133 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM134 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM135 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM136 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM137 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM138 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM139 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM140 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM141 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM142 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM143 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM144 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM145 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM146 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM147 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM148 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM149 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM150 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM151 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM152 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM153 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM154 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM155 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM156 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM157 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM158 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM159 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM160 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM161 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM162 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM163 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM164 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM165 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM166 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM167 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM168 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM169 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM170 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM171 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM172 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM173 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM174 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM175 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM176 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM177 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM178 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM179 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM180 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM181 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM182 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM183 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM184 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM185 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM186 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM187 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM188 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM189 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM190 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM191 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM192 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM193 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM194 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM195 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM196 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM197 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class OPTIONS3 : public ParserKeyword {
   public:
       OPTIONS3();
       static const std::string keywordName;

       class ITEM1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM3 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM4 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM5 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM6 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM7 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM8 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM9 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM10 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM11 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM12 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM13 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM14 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM15 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM16 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM17 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM18 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM19 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM20 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM21 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM22 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM23 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM24 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM25 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM26 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM27 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM28 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM29 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM30 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM31 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM32 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM33 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM34 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM35 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM36 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM37 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM38 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM39 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM40 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM41 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM42 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM43 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM44 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM45 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM46 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM47 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM48 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM49 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM50 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM51 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM52 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM53 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM54 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM55 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM56 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM57 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM58 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM59 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM60 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM61 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM62 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM63 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM64 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM65 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM66 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM67 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM68 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM69 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM70 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM71 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM72 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM73 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM74 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM75 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM76 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM77 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM78 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM79 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM80 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM81 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM82 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM83 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM84 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM85 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM86 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM87 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM88 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM89 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM90 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM91 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM92 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM93 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM94 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM95 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM96 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM97 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM98 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM99 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM100 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM101 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM102 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM103 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM104 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM105 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM106 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM107 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM108 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM109 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM110 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM111 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM112 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM113 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM114 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM115 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM116 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM117 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM118 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM119 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM120 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM121 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM122 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM123 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM124 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM125 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM126 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM127 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM128 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM129 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM130 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM131 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM132 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM133 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM134 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM135 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM136 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM137 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM138 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM139 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM140 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM141 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM142 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM143 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM144 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM145 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM146 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM147 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM148 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM149 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM150 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM151 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM152 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM153 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM154 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM155 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM156 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM157 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM158 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM159 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM160 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM161 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM162 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM163 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM164 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM165 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM166 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM167 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM168 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM169 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM170 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM171 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM172 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM173 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM174 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM175 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM176 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM177 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM178 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM179 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM180 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM181 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM182 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM183 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM184 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM185 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM186 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM187 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM188 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM189 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM190 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM191 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM192 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM193 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM194 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM195 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM196 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM197 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM198 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM199 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM200 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM201 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM202 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM203 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM204 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM205 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM206 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM207 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM208 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM209 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM210 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM211 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM212 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM213 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM214 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM215 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM216 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM217 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM218 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM219 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM220 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM221 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM222 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM223 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM224 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM225 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM226 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM227 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM228 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM229 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM230 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM231 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM232 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM233 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM234 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM235 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM236 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM237 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM238 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM239 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM240 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM241 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM242 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM243 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM244 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM245 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM246 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM247 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM248 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM249 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM250 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM251 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM252 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM253 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM254 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM255 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM256 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM257 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM258 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM259 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM260 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM261 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM262 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM263 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM264 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM265 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM266 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM267 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM268 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM269 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM270 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM271 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM272 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM273 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM274 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM275 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM276 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM277 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM278 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM279 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM280 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM281 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM282 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM283 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM284 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM285 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM286 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM287 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ITEM288 {
       public:
           static const std::string itemName;
           static const int defaultValue;
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
