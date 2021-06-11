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



   class UDQ : public ParserKeyword {
   public:
       UDQ();
       static const std::string keywordName;

       class ACTION {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
       };

       class DATA {
       public:
           static const std::string itemName;
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



   class UDQPARAM : public ParserKeyword {
   public:
       UDQPARAM();
       static const std::string keywordName;

       class RANDOM_SEED {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class RANGE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class UNDEFINED_VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CMP_EPSILON {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class UDT : public ParserKeyword {
   public:
       UDT();
       static const std::string keywordName;
   };



   class UDTDIMS : public ParserKeyword {
   public:
       UDTDIMS();
       static const std::string keywordName;

       class MAX_TABLES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ROWS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_INTERPOLATION_POINTS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_DIMENSIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class UNCODHMD : public ParserKeyword {
   public:
       UNCODHMD();
       static const std::string keywordName;
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



   class UNIFOUTS : public ParserKeyword {
   public:
       UNIFOUTS();
       static const std::string keywordName;
   };



   class UNIFSAVE : public ParserKeyword {
   public:
       UNIFSAVE();
       static const std::string keywordName;
   };



   class USECUPL : public ParserKeyword {
   public:
       USECUPL();
       static const std::string keywordName;

       class BASE {
       public:
           static const std::string itemName;
       };

       class FMT {
       public:
           static const std::string itemName;
       };
   };



   class USEFLUX : public ParserKeyword {
   public:
       USEFLUX();
       static const std::string keywordName;
   };



   class USENOFLO : public ParserKeyword {
   public:
       USENOFLO();
       static const std::string keywordName;
   };



}
}
#endif
