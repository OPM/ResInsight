#ifndef PARSER_KEYWORDS_U_HPP
#define PARSER_KEYWORDS_U_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class UDADIMS : public ParserKeyword {
   public:
       UDADIMS();
       static const std::string keywordName;

       class NUM_UDQ_REPLACE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class IGNORED {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class TOTAL_UDQ_UNIQUE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class UDQDIMS : public ParserKeyword {
   public:
       UDQDIMS();
       static const std::string keywordName;

       class MAX_FUNCTIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ITEMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_CONNECTIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_FIELDS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_GROUP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_REGION {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_SEGMENT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WELL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_AQUIFER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_BLOCK {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class RESTART_NEW_SEED {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class UNIFIN : public ParserKeyword {
   public:
       UNIFIN();
       static const std::string keywordName;
   };



   class UNIFOUT : public ParserKeyword {
   public:
       UNIFOUT();
       static const std::string keywordName;
   };



}
}
#endif
