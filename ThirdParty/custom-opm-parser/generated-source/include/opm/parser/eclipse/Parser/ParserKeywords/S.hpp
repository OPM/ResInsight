#ifndef PARSER_KEYWORDS_S_HPP
#define PARSER_KEYWORDS_S_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class SATNUM : public ParserKeyword {
   public:
       SATNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SATOPTS : public ParserKeyword {
   public:
       SATOPTS();
       static const std::string keywordName;

       class options {
       public:
           static const std::string itemName;
       };
   };



   class SAVE : public ParserKeyword {
   public:
       SAVE();
       static const std::string keywordName;

       class FILE_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class SCALECRS : public ParserKeyword {
   public:
       SCALECRS();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class SCHEDULE : public ParserKeyword {
   public:
       SCHEDULE();
       static const std::string keywordName;
   };



   class SDENSITY : public ParserKeyword {
   public:
       SDENSITY();
       static const std::string keywordName;

       class SOLVENT_DENSITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SEPARATE : public ParserKeyword {
   public:
       SEPARATE();
       static const std::string keywordName;
   };



   class SGAS : public ParserKeyword {
   public:
       SGAS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SGCR : public ParserKeyword {
   public:
       SGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SGCWMIS : public ParserKeyword {
   public:
       SGCWMIS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SGFN : public ParserKeyword {
   public:
       SGFN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SGL : public ParserKeyword {
   public:
       SGL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SGOF : public ParserKeyword {
   public:
       SGOF();
       static const std::string keywordName;

       class table {
       public:
           static const std::string itemName;
       };
   };



   class SGU : public ParserKeyword {
   public:
       SGU();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SGWFN : public ParserKeyword {
   public:
       SGWFN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SHRATE : public ParserKeyword {
   public:
       SHRATE();
       static const std::string keywordName;

       class SHEAR_RATE {
       public:
           static const std::string itemName;
       };
   };



   class SKIP : public ParserKeyword {
   public:
       SKIP();
       static const std::string keywordName;
   };



   class SKIP100 : public ParserKeyword {
   public:
       SKIP100();
       static const std::string keywordName;
   };



   class SKIP300 : public ParserKeyword {
   public:
       SKIP300();
       static const std::string keywordName;
   };



   class SKIPREST : public ParserKeyword {
   public:
       SKIPREST();
       static const std::string keywordName;
   };



   class SLGOF : public ParserKeyword {
   public:
       SLGOF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SMRYDIMS : public ParserKeyword {
   public:
       SMRYDIMS();
       static const std::string keywordName;

       class DIMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class SOF2 : public ParserKeyword {
   public:
       SOF2();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SOF3 : public ParserKeyword {
   public:
       SOF3();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SOGCR : public ParserKeyword {
   public:
       SOGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOIL : public ParserKeyword {
   public:
       SOIL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOLUTION : public ParserKeyword {
   public:
       SOLUTION();
       static const std::string keywordName;
   };



   class SOLVENT : public ParserKeyword {
   public:
       SOLVENT();
       static const std::string keywordName;
   };



   class SORWMIS : public ParserKeyword {
   public:
       SORWMIS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SOWCR : public ParserKeyword {
   public:
       SOWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SPECGRID : public ParserKeyword {
   public:
       SPECGRID();
       static const std::string keywordName;

       class NX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NY {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NZ {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUMRES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class COORD_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class SPECHEAT : public ParserKeyword {
   public:
       SPECHEAT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SPECROCK : public ParserKeyword {
   public:
       SPECROCK();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SSFN : public ParserKeyword {
   public:
       SSFN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SSOL : public ParserKeyword {
   public:
       SSOL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class START : public ParserKeyword {
   public:
       START();
       static const std::string keywordName;

       class DAY {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MONTH {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class YEAR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class TIME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class STCOND : public ParserKeyword {
   public:
       STCOND();
       static const std::string keywordName;

       class TEMPERATURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class STONE1 : public ParserKeyword {
   public:
       STONE1();
       static const std::string keywordName;
   };



   class STONE1EX : public ParserKeyword {
   public:
       STONE1EX();
       static const std::string keywordName;

       class EXP_VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SUMMARY : public ParserKeyword {
   public:
       SUMMARY();
       static const std::string keywordName;
   };



   class SUMTHIN : public ParserKeyword {
   public:
       SUMTHIN();
       static const std::string keywordName;

       class TIME {
       public:
           static const std::string itemName;
       };
   };



   class SWAT : public ParserKeyword {
   public:
       SWAT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SWATINIT : public ParserKeyword {
   public:
       SWATINIT();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SWCR : public ParserKeyword {
   public:
       SWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SWFN : public ParserKeyword {
   public:
       SWFN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SWL : public ParserKeyword {
   public:
       SWL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SWOF : public ParserKeyword {
   public:
       SWOF();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SWU : public ParserKeyword {
   public:
       SWU();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
