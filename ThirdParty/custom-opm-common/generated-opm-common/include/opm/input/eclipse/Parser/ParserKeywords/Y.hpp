#ifndef PARSER_KEYWORDS_Y_HPP
#define PARSER_KEYWORDS_Y_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class YMF : public ParserKeyword {
   public:
       YMF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class YMODULE : public ParserKeyword {
   public:
       YMODULE();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
