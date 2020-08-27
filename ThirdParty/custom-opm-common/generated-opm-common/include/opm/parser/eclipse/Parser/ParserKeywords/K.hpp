#ifndef PARSER_KEYWORDS_K_HPP
#define PARSER_KEYWORDS_K_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class KRNUM : public ParserKeyword {
   public:
       KRNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class KRNUMMF : public ParserKeyword {
   public:
       KRNUMMF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
