#ifndef PARSER_KEYWORDS_N_HPP
#define PARSER_KEYWORDS_N_HPP
#include <opm/input/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class NARROW : public ParserKeyword {
   public:
       NARROW();
       static const std::string keywordName;
   };



   class NCOMPS : public ParserKeyword {
   public:
       NCOMPS();
       static const std::string keywordName;

       class NUM_COMPS {
       public:
           static const std::string itemName;
       };
   };



   class NCONSUMP : public ParserKeyword {
   public:
       NCONSUMP();
       static const std::string keywordName;

       class NODE {
       public:
           static const std::string itemName;
       };

       class GAS_CONSUMPTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class REMOVAL_GROUP {
       public:
           static const std::string itemName;
       };
   };



   class NEFAC : public ParserKeyword {
   public:
       NEFAC();
       static const std::string keywordName;

       class NODE {
       public:
           static const std::string itemName;
       };

       class EFF_FACTOR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class NETBALAN : public ParserKeyword {
   public:
       NETBALAN();
       static const std::string keywordName;

       class TIME_INTERVAL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class PRESSURE_CONVERGENCE_LIMIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class MAX_ITER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class THP_CONVERGENCE_LIMIT {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.010000;
       };

       class MAX_ITER_THP {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
       };

       class TARGET_BALANCE_ERROR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class MAX_BALANCE_ERROR {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 100000000000000000000.000000;
       };

       class MIN_TIME_STEP {
       public:
           static const std::string itemName;
       };
   };



   class NETCOMPA : public ParserKeyword {
   public:
       NETCOMPA();
       static const std::string keywordName;

       class INLET {
       public:
           static const std::string itemName;
       };

       class OUTLET {
       public:
           static const std::string itemName;
       };

       class GROUP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class VFT_TABLE_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class ALQ {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class GAS_CONSUMPTION_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class EXTRACTION_GROUP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class COMPRESSOR_TYPE {
       public:
           static const std::string itemName;
       };

       class NUM_COMPRESSION_LEVELS {
       public:
           static const std::string itemName;
       };

       class ALQ_LEVEL1 {
       public:
           static const std::string itemName;
       };

       class COMP_SWITCH_SEQ_NUM {
       public:
           static const std::string itemName;
       };
   };



   class NETWORK : public ParserKeyword {
   public:
       NETWORK();
       static const std::string keywordName;

       class NODMAX {
       public:
           static const std::string itemName;
       };

       class NBRMAX {
       public:
           static const std::string itemName;
       };

       class NBCMAX {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 20;
       };
   };



   class NETWORK_PROBE : public ParserKeyword {
   public:
       NETWORK_PROBE();
       static const std::string keywordName;

       class NETWORK_NODES {
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



   class NEXTSTPL : public ParserKeyword {
   public:
       NEXTSTPL();
       static const std::string keywordName;

       class MAX_LENGTH {
       public:
           static const std::string itemName;
       };

       class APPLY_TO_ALL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class NINENUM : public ParserKeyword {
   public:
       NINENUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class NINEPOIN : public ParserKeyword {
   public:
       NINEPOIN();
       static const std::string keywordName;
   };



   class NMATOPTS : public ParserKeyword {
   public:
       NMATOPTS();
       static const std::string keywordName;

       class GEOMETRY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FRACTION_PORE_VOL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0.100000;
       };

       class METHOD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class NMATRIX : public ParserKeyword {
   public:
       NMATRIX();
       static const std::string keywordName;

       class NUM_SUB_CELLS {
       public:
           static const std::string itemName;
       };
   };



   class NMESSAGE : public ParserKeyword {
   public:
       NMESSAGE();
       static const std::string keywordName;
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
           static constexpr double defaultValue = 0;
       };

       class IST1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class IST2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class PRESSURE_TABLE1 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class PRESSURE_TABLE2 {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 0;
       };

       class VE_FACE1 {
       public:
           static const std::string itemName;
       };

       class VE_FACE2 {
       public:
           static const std::string itemName;
       };

       class DIFFUSIVITY {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class SIM_DEPENDENT3 {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class VDFLOW_AREA {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };

       class VDFLOW_PERM {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 0;
       };
   };



   class NNEWTF : public ParserKeyword {
   public:
       NNEWTF();
       static const std::string keywordName;

       class NTHRBL {
       public:
           static const std::string itemName;
       };

       class NLNHBL {
       public:
           static const std::string itemName;
       };
   };



   class NOCASC : public ParserKeyword {
   public:
       NOCASC();
       static const std::string keywordName;
   };



   class NODEPROP : public ParserKeyword {
   public:
       NODEPROP();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
       };

       class AS_CHOKE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ADD_GAS_LIFT_GAS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHOKE_GROUP {
       public:
           static const std::string itemName;
       };

       class SOURCE_SINK_GROUP {
       public:
           static const std::string itemName;
       };

       class NETWORK_VALUE_TYPE {
       public:
           static const std::string itemName;
       };
   };



   class NODPPM : public ParserKeyword {
   public:
       NODPPM();
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



   class NOHMD : public ParserKeyword {
   public:
       NOHMD();
       static const std::string keywordName;

       class GRAD_PARAMS {
       public:
           static const std::string itemName;
       };
   };



   class NOHMO : public ParserKeyword {
   public:
       NOHMO();
       static const std::string keywordName;

       class GRAD_PARAMS {
       public:
           static const std::string itemName;
       };
   };



   class NOHYST : public ParserKeyword {
   public:
       NOHYST();
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



   class NOWARN : public ParserKeyword {
   public:
       NOWARN();
       static const std::string keywordName;
   };



   class NOWARNEP : public ParserKeyword {
   public:
       NOWARNEP();
       static const std::string keywordName;
   };



   class NRSOUT : public ParserKeyword {
   public:
       NRSOUT();
       static const std::string keywordName;

       class MAX_NUM {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 3600;
       };
   };



   class NSTACK : public ParserKeyword {
   public:
       NSTACK();
       static const std::string keywordName;

       class LINEAR_SOLVER_SIZE {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 10;
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
           static constexpr int defaultValue = 1;
       };
   };



   class NUPCOL : public ParserKeyword {
   public:
       NUPCOL();
       static const std::string keywordName;

       class NUM_ITER {
       public:
           static const std::string itemName;
           static constexpr int defaultValue = 12;
       };
   };



   class NWATREM : public ParserKeyword {
   public:
       NWATREM();
       static const std::string keywordName;

       class NODE {
       public:
           static const std::string itemName;
       };

       class WAX_RATE {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104.000000;
       };

       class MAX_FRAC_REMOVAL {
       public:
           static const std::string itemName;
           static constexpr double defaultValue = 1.000000;
       };
   };



   class NXFIN : public ParserKeyword {
   public:
       NXFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class NYFIN : public ParserKeyword {
   public:
       NYFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class NZFIN : public ParserKeyword {
   public:
       NZFIN();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
