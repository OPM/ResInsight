#ifndef PARSER_KEYWORDS_Q_HPP
#define PARSER_KEYWORDS_Q_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class QDRILL : public ParserKeyword {
   public:
       QDRILL();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };
   };



   class QHRATING : public ParserKeyword {
   public:
       QHRATING();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class QMOBIL : public ParserKeyword {
   public:
       QMOBIL();
       static const std::string keywordName;

       class MOBILE_END_POINT_CORRECTION {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
