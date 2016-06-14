#ifndef PARSER_KEYWORDS_B_HPP
#define PARSER_KEYWORDS_B_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class BLOCK_PROBE : public ParserKeyword {
   public:
       BLOCK_PROBE();
       static const std::string keywordName;

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class BLOCK_PROBE300 : public ParserKeyword {
   public:
       BLOCK_PROBE300();
       static const std::string keywordName;

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class BOX : public ParserKeyword {
   public:
       BOX();
       static const std::string keywordName;

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
   };



}
}
#endif
