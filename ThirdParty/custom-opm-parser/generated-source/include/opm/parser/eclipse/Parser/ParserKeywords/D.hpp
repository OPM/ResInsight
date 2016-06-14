#ifndef PARSER_KEYWORDS_D_HPP
#define PARSER_KEYWORDS_D_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class DATE : public ParserKeyword {
   public:
       DATE();
       static const std::string keywordName;
   };



   class DATES : public ParserKeyword {
   public:
       DATES();
       static const std::string keywordName;

       class DAY {
       public:
           static const std::string itemName;
       };

       class MONTH {
       public:
           static const std::string itemName;
       };

       class YEAR {
       public:
           static const std::string itemName;
       };

       class TIME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DENSITY : public ParserKeyword {
   public:
       DENSITY();
       static const std::string keywordName;

       class OIL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class DEPTH : public ParserKeyword {
   public:
       DEPTH();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DEPTHZ : public ParserKeyword {
   public:
       DEPTHZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DIMENS : public ParserKeyword {
   public:
       DIMENS();
       static const std::string keywordName;

       class NX {
       public:
           static const std::string itemName;
       };

       class NY {
       public:
           static const std::string itemName;
       };

       class NZ {
       public:
           static const std::string itemName;
       };
   };



   class DISGAS : public ParserKeyword {
   public:
       DISGAS();
       static const std::string keywordName;
   };



   class DREF : public ParserKeyword {
   public:
       DREF();
       static const std::string keywordName;

       class DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class DREFS : public ParserKeyword {
   public:
       DREFS();
       static const std::string keywordName;

       class DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class DRSDT : public ParserKeyword {
   public:
       DRSDT();
       static const std::string keywordName;

       class DRSDT_MAX {
       public:
           static const std::string itemName;
       };

       class Option {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class DRVDT : public ParserKeyword {
   public:
       DRVDT();
       static const std::string keywordName;

       class DRVDT_MAX {
       public:
           static const std::string itemName;
       };
   };



   class DX : public ParserKeyword {
   public:
       DX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DXV : public ParserKeyword {
   public:
       DXV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DY : public ParserKeyword {
   public:
       DY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DYV : public ParserKeyword {
   public:
       DYV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DZ : public ParserKeyword {
   public:
       DZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class DZV : public ParserKeyword {
   public:
       DZV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
