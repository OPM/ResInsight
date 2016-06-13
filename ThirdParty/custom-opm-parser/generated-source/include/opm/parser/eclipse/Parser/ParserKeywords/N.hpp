#ifndef PARSER_KEYWORDS_N_HPP
#define PARSER_KEYWORDS_N_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class NETBALAN : public ParserKeyword {
   public:
       NETBALAN();
       static const std::string keywordName;

       class TIME_INTERVAL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRESSURE_CONVERGENCE_LIMT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_ITER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class THP_CONVERGENCE_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_ITER_THP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class TARGET_BALANCE_ERROR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_BALANCE_ERROR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_TIME_STEP {
       public:
           static const std::string itemName;
       };
   };



   class NEWTRAN : public ParserKeyword {
   public:
       NEWTRAN();
       static const std::string keywordName;
   };



   class NEXTSTEP : public ParserKeyword {
   public:
       NEXTSTEP();
       static const std::string keywordName;

       class MAX_STEP {
       public:
           static const std::string itemName;
       };

       class APPLY_TO_ALL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class NNC : public ParserKeyword {
   public:
       NNC();
       static const std::string keywordName;

       class I1 {
       public:
           static const std::string itemName;
       };

       class J1 {
       public:
           static const std::string itemName;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class I2 {
       public:
           static const std::string itemName;
       };

       class J2 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class TRAN {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SIM_DEPENDENT1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SIM_DEPENDENT2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRESSURE_TABLE1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class PRESSURE_TABLE2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VE_FACE1 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class VE_FACE2 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class DIFFUSIVITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SIM_DEPENDENT3 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class VDFLOW_AREA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class VDFLOW_PERM {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class NOCASC : public ParserKeyword {
   public:
       NOCASC();
       static const std::string keywordName;
   };



   class NOECHO : public ParserKeyword {
   public:
       NOECHO();
       static const std::string keywordName;
   };



   class NOGGF : public ParserKeyword {
   public:
       NOGGF();
       static const std::string keywordName;
   };



   class NOGRAV : public ParserKeyword {
   public:
       NOGRAV();
       static const std::string keywordName;
   };



   class NOINSPEC : public ParserKeyword {
   public:
       NOINSPEC();
       static const std::string keywordName;
   };



   class NOMONITO : public ParserKeyword {
   public:
       NOMONITO();
       static const std::string keywordName;
   };



   class NONNC : public ParserKeyword {
   public:
       NONNC();
       static const std::string keywordName;
   };



   class NORSSPEC : public ParserKeyword {
   public:
       NORSSPEC();
       static const std::string keywordName;
   };



   class NOSIM : public ParserKeyword {
   public:
       NOSIM();
       static const std::string keywordName;
   };



   class NSTACK : public ParserKeyword {
   public:
       NSTACK();
       static const std::string keywordName;

       class LINEAR_SOLVER_SIZE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class NTG : public ParserKeyword {
   public:
       NTG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class NUMRES : public ParserKeyword {
   public:
       NUMRES();
       static const std::string keywordName;

       class num {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class NUPCOL : public ParserKeyword {
   public:
       NUPCOL();
       static const std::string keywordName;

       class NUM_ITER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



}
}
#endif
