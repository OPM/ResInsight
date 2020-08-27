#ifndef PARSER_KEYWORDS_Z_HPP
#define PARSER_KEYWORDS_Z_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class ZCORN : public ParserKeyword {
   public:
       ZCORN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ZFACT1 : public ParserKeyword {
   public:
       ZFACT1();
       static const std::string keywordName;

       class Z0 {
       public:
           static const std::string itemName;
       };
   };



   class ZFACT1S : public ParserKeyword {
   public:
       ZFACT1S();
       static const std::string keywordName;

       class Z0 {
       public:
           static const std::string itemName;
       };
   };



   class ZFACTOR : public ParserKeyword {
   public:
       ZFACTOR();
       static const std::string keywordName;

       class Z0 {
       public:
           static const std::string itemName;
       };
   };



   class ZFACTORS : public ParserKeyword {
   public:
       ZFACTORS();
       static const std::string keywordName;

       class Z0 {
       public:
           static const std::string itemName;
       };
   };



   class ZIPP2OFF : public ParserKeyword {
   public:
       ZIPP2OFF();
       static const std::string keywordName;
   };



   class ZIPPY2 : public ParserKeyword {
   public:
       ZIPPY2();
       static const std::string keywordName;

       class SETTINGS {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
