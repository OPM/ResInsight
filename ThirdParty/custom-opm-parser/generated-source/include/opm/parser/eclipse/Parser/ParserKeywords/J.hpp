#ifndef PARSER_KEYWORDS_J_HPP
#define PARSER_KEYWORDS_J_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class JFUNC : public ParserKeyword {
   public:
       JFUNC();
       static const std::string keywordName;

       class FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OW_SURFACE_TENSION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GO_SURFACE_TENSION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ALPHA_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class BETA_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



}
}
#endif
