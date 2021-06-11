#ifndef PARSER_KEYWORDS_I_HPP
#define PARSER_KEYWORDS_I_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class IHOST : public ParserKeyword {
   public:
       IHOST();
       static const std::string keywordName;

       class LGR {
       public:
           static const std::string itemName;
       };

       class PROCESS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class IMBNUM : public ParserKeyword {
   public:
       IMBNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class IMBNUMMF : public ParserKeyword {
   public:
       IMBNUMMF();
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



   class IMPCVD : public ParserKeyword {
   public:
       IMPCVD();
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



   class IMPLICIT : public ParserKeyword {
   public:
       IMPLICIT();
       static const std::string keywordName;
   };



   class IMPORT : public ParserKeyword {
   public:
       IMPORT();
       static const std::string keywordName;

       class FILE {
       public:
           static const std::string itemName;
       };

       class FORMATTED {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
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



   class IMSPCVD : public ParserKeyword {
   public:
       IMSPCVD();
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



   class INRAD : public ParserKeyword {
   public:
       INRAD();
       static const std::string keywordName;

       class RADIUS {
       public:
           static const std::string itemName;
       };
   };



   class INSPEC : public ParserKeyword {
   public:
       INSPEC();
       static const std::string keywordName;
   };



   class INTPC : public ParserKeyword {
   public:
       INTPC();
       static const std::string keywordName;

       class PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class IONROCK : public ParserKeyword {
   public:
       IONROCK();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class IONXROCK : public ParserKeyword {
   public:
       IONXROCK();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class IONXSURF : public ParserKeyword {
   public:
       IONXSURF();
       static const std::string keywordName;

       class MOLECULAR_WEIGHT {
       public:
           static const std::string itemName;
       };

       class ION_EXCH_CONST {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
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



   class ISGLPC : public ParserKeyword {
   public:
       ISGLPC();
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



   class ISOLNUM : public ParserKeyword {
   public:
       ISOLNUM();
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



   class ISWLPC : public ParserKeyword {
   public:
       ISWLPC();
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
