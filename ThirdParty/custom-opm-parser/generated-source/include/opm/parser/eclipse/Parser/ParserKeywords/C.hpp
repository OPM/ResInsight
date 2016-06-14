#ifndef PARSER_KEYWORDS_C_HPP
#define PARSER_KEYWORDS_C_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class COMPDAT : public ParserKeyword {
   public:
       COMPDAT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class STATE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SAT_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class CONNECTION_TRANSMISSIBILITY_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIAMETER {
       public:
           static const std::string itemName;
       };

       class Kh {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SKIN {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class D_FACTOR {
       public:
           static const std::string itemName;
       };

       class DIR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PR {
       public:
           static const std::string itemName;
       };
   };



   class COMPLUMP : public ParserKeyword {
   public:
       COMPLUMP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
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

       class N {
       public:
           static const std::string itemName;
       };
   };



   class COMPORD : public ParserKeyword {
   public:
       COMPORD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class ORDER_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class COMPS : public ParserKeyword {
   public:
       COMPS();
       static const std::string keywordName;

       class NUM_COMPS {
       public:
           static const std::string itemName;
       };
   };



   class COMPSEGS : public ParserKeyword {
   public:
       COMPSEGS();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

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

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class DISTANCE_START {
       public:
           static const std::string itemName;
       };

       class DISTANCE_END {
       public:
           static const std::string itemName;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
       };

       class END_IJK {
       public:
           static const std::string itemName;
       };

       class CENTER_DEPTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THERMAL_LENGTH {
       public:
           static const std::string itemName;
       };

       class SEGMENT_NUMBER {
       public:
           static const std::string itemName;
       };
   };



   class CONNECTION_PROBE : public ParserKeyword {
   public:
       CONNECTION_PROBE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

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



   class COORD : public ParserKeyword {
   public:
       COORD();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class COPY : public ParserKeyword {
   public:
       COPY();
       static const std::string keywordName;

       class src {
       public:
           static const std::string itemName;
       };

       class target {
       public:
           static const std::string itemName;
       };

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



   class COPYREG : public ParserKeyword {
   public:
       COPYREG();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class TARGET_ARRAY {
       public:
           static const std::string itemName;
       };

       class REGION_NUMBER {
       public:
           static const std::string itemName;
       };

       class REGION_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class CPR : public ParserKeyword {
   public:
       CPR();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

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



   class CREF : public ParserKeyword {
   public:
       CREF();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };
   };



   class CREFS : public ParserKeyword {
   public:
       CREFS();
       static const std::string keywordName;

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
