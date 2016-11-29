#ifndef PARSER_KEYWORDS_F_HPP
#define PARSER_KEYWORDS_F_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class FAULTDIM : public ParserKeyword {
   public:
       FAULTDIM();
       static const std::string keywordName;

       class MFSEGS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class FAULTS : public ParserKeyword {
   public:
       FAULTS();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class IX1 {
       public:
           static const std::string itemName;
       };

       class IX2 {
       public:
           static const std::string itemName;
       };

       class IY1 {
       public:
           static const std::string itemName;
       };

       class IY2 {
       public:
           static const std::string itemName;
       };

       class IZ1 {
       public:
           static const std::string itemName;
       };

       class IZ2 {
       public:
           static const std::string itemName;
       };

       class FACE {
       public:
           static const std::string itemName;
       };
   };



   class FIELD : public ParserKeyword {
   public:
       FIELD();
       static const std::string keywordName;
   };



   class FIELD_PROBE : public ParserKeyword {
   public:
       FIELD_PROBE();
       static const std::string keywordName;
   };



   class FILLEPS : public ParserKeyword {
   public:
       FILLEPS();
       static const std::string keywordName;
   };



   class FIPNUM : public ParserKeyword {
   public:
       FIPNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FLUXNUM : public ParserKeyword {
   public:
       FLUXNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class FMTIN : public ParserKeyword {
   public:
       FMTIN();
       static const std::string keywordName;
   };



   class FMTOUT : public ParserKeyword {
   public:
       FMTOUT();
       static const std::string keywordName;
   };



   class FULLIMP : public ParserKeyword {
   public:
       FULLIMP();
       static const std::string keywordName;
   };



}
}
#endif
