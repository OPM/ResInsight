#ifndef PARSER_KEYWORDS_I_HPP
#define PARSER_KEYWORDS_I_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class IMBNUM : public ParserKeyword {
   public:
       IMBNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class IMKRVD : public ParserKeyword {
   public:
       IMKRVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class IMPES : public ParserKeyword {
   public:
       IMPES();
       static const std::string keywordName;
   };



   class IMPTVD : public ParserKeyword {
   public:
       IMPTVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class INCLUDE : public ParserKeyword {
   public:
       INCLUDE();
       static const std::string keywordName;

       class IncludeFile {
       public:
           static const std::string itemName;
       };
   };



   class INIT : public ParserKeyword {
   public:
       INIT();
       static const std::string keywordName;
   };



   class IPCG : public ParserKeyword {
   public:
       IPCG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class IPCW : public ParserKeyword {
   public:
       IPCW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISGCR : public ParserKeyword {
   public:
       ISGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISGL : public ParserKeyword {
   public:
       ISGL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISGU : public ParserKeyword {
   public:
       ISGU();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISOGCR : public ParserKeyword {
   public:
       ISOGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISOWCR : public ParserKeyword {
   public:
       ISOWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISWCR : public ParserKeyword {
   public:
       ISWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISWL : public ParserKeyword {
   public:
       ISWL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ISWU : public ParserKeyword {
   public:
       ISWU();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
