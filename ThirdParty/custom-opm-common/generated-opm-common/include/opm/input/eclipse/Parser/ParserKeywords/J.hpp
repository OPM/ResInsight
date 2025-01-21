#ifndef PARSER_KEYWORDS_J_HPP
#define PARSER_KEYWORDS_J_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
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
           static constexpr double defaultValue = -1.000000;
       };

       class GO_SURFACE_TENSION {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = -1.000000;
       };

       class ALPHA_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class BETA_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class JFUNCR : public ParserKeyword {
   public:
       JFUNCR();
       static const std::string keywordName;

       class J_FUNCTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OIL_WAT_SURF_TENSTION {
       public:
           static const std::string itemName;
       };

       class OIL_GAS_SURF_TENSTION {
       public:
           static const std::string itemName;
       };

       class POROSITY_POWER {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class PERMEABILITY_POWER {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.500000;
       };

       class PERM_DIRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



}
}
#endif
