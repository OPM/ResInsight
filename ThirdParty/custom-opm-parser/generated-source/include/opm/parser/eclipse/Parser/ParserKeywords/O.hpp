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



   class OIL : public ParserKeyword {
   public:
       OIL();
       static const std::string keywordName;
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



}
}
#endif
