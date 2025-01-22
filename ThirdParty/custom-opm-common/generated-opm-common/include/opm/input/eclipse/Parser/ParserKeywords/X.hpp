#ifndef PARSER_KEYWORDS_X_HPP
#define PARSER_KEYWORDS_X_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class XMF : public ParserKeyword {
   public:
       XMF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
