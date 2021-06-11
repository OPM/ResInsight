#ifndef PARSER_KEYWORDS_S_HPP
#define PARSER_KEYWORDS_S_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class SALT : public ParserKeyword {
   public:
       SALT();
       static const std::string keywordName;

       class SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class SALTNODE : public ParserKeyword {
   public:
       SALTNODE();
       static const std::string keywordName;

       class SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class SALTPVD : public ParserKeyword {
   public:
       SALTPVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SALTREST : public ParserKeyword {
   public:
       SALTREST();
       static const std::string keywordName;

       class SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class SALTSOL : public ParserKeyword {
   public:
       SALTSOL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SALTVD : public ParserKeyword {
   public:
       SALTVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SAMG : public ParserKeyword {
   public:
       SAMG();
       static const std::string keywordName;

       class EPS {
       public:
           static const std::string itemName;
       };

       class REUSE {
       public:
           static const std::string itemName;
       };
   };



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



   class SCALELIM : public ParserKeyword {
   public:
       SCALELIM();
       static const std::string keywordName;

       class SAT_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SCDATAB : public ParserKeyword {
   public:
       SCDATAB();
       static const std::string keywordName;

       class SCALE_DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SCDETAB : public ParserKeyword {
   public:
       SCDETAB();
       static const std::string keywordName;

       class SCALE_DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SCDPTAB : public ParserKeyword {
   public:
       SCDPTAB();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SCDPTRAC : public ParserKeyword {
   public:
       SCDPTRAC();
       static const std::string keywordName;

       class TRACER {
       public:
           static const std::string itemName;
       };
   };



   class SCHEDULE : public ParserKeyword {
   public:
       SCHEDULE();
       static const std::string keywordName;
   };



   class SCPDIMS : public ParserKeyword {
   public:
       SCPDIMS();
       static const std::string keywordName;

       class NTSCDP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NPSCDP {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTSCDA {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class PSCDA {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UNUSED1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UNUSED2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTSCDE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class SCVD : public ParserKeyword {
   public:
       SCVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
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



   class SEGMENT_PROBE : public ParserKeyword {
   public:
       SEGMENT_PROBE();
       static const std::string keywordName;

       class Well {
       public:
           static const std::string itemName;
       };

       class Segment {
       public:
           static const std::string itemName;
       };
   };



   class SEPARATE : public ParserKeyword {
   public:
       SEPARATE();
       static const std::string keywordName;
   };



   class SEPVALS : public ParserKeyword {
   public:
       SEPVALS();
       static const std::string keywordName;

       class SEPARATOR {
       public:
           static const std::string itemName;
       };

       class BO {
       public:
           static const std::string itemName;
       };

       class RS {
       public:
           static const std::string itemName;
       };
   };



   class SFOAM : public ParserKeyword {
   public:
       SFOAM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
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



   class SGF32D : public ParserKeyword {
   public:
       SGF32D();
       static const std::string keywordName;

       class SOIL {
       public:
           static const std::string itemName;
       };

       class SWAT {
       public:
           static const std::string itemName;
       };

       class KRG {
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



   class SGLPC : public ParserKeyword {
   public:
       SGLPC();
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

       class DATA {
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



   class SIGMA : public ParserKeyword {
   public:
       SIGMA();
       static const std::string keywordName;

       class COUPLING {
       public:
           static const std::string itemName;
       };
   };



   class SIGMAGDV : public ParserKeyword {
   public:
       SIGMAGDV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SIGMATH : public ParserKeyword {
   public:
       SIGMATH();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SIGMAV : public ParserKeyword {
   public:
       SIGMAV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SIMULATE : public ParserKeyword {
   public:
       SIMULATE();
       static const std::string keywordName;
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



   class SKPRPOLY : public ParserKeyword {
   public:
       SKPRPOLY();
       static const std::string keywordName;

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };

       class POLYMERCONCENTRATION {
       public:
           static const std::string itemName;
       };

       class THROUGHPUT {
       public:
           static const std::string itemName;
       };

       class VELOCITY {
       public:
           static const std::string itemName;
       };

       class SKINPRESSURE {
       public:
           static const std::string itemName;
       };
   };



   class SKPRWAT : public ParserKeyword {
   public:
       SKPRWAT();
       static const std::string keywordName;

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };

       class THROUGHPUT {
       public:
           static const std::string itemName;
       };

       class VELOCITY {
       public:
           static const std::string itemName;
       };

       class SKINPRESSURE {
       public:
           static const std::string itemName;
       };
   };



   class SKRO : public ParserKeyword {
   public:
       SKRO();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SKRORG : public ParserKeyword {
   public:
       SKRORG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SKRORW : public ParserKeyword {
   public:
       SKRORW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SKRW : public ParserKeyword {
   public:
       SKRW();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SKRWR : public ParserKeyword {
   public:
       SKRWR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SLAVES : public ParserKeyword {
   public:
       SLAVES();
       static const std::string keywordName;

       class SLAVE_RESERVOIR {
       public:
           static const std::string itemName;
       };

       class SLAVE_ECLBASE {
       public:
           static const std::string itemName;
       };

       class HOST_NAME {
       public:
           static const std::string itemName;
       };

       class DIRECTORY {
       public:
           static const std::string itemName;
       };

       class NUM_PE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
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



   class SMULTX : public ParserKeyword {
   public:
       SMULTX();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SMULTY : public ParserKeyword {
   public:
       SMULTY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SMULTZ : public ParserKeyword {
   public:
       SMULTZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOCRS : public ParserKeyword {
   public:
       SOCRS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
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



   class SOF32D : public ParserKeyword {
   public:
       SOF32D();
       static const std::string keywordName;

       class SOIL {
       public:
           static const std::string itemName;
       };

       class SGAS {
       public:
           static const std::string itemName;
       };

       class KRW {
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



   class SOLVCONC : public ParserKeyword {
   public:
       SOLVCONC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOLVDIMS : public ParserKeyword {
   public:
       SOLVDIMS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOLVDIRS : public ParserKeyword {
   public:
       SOLVDIRS();
       static const std::string keywordName;

       class DIRECTION {
       public:
           static const std::string itemName;
       };
   };



   class SOLVENT : public ParserKeyword {
   public:
       SOLVENT();
       static const std::string keywordName;
   };



   class SOLVFRAC : public ParserKeyword {
   public:
       SOLVFRAC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOLVNUM : public ParserKeyword {
   public:
       SOLVNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SOMGAS : public ParserKeyword {
   public:
       SOMGAS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SOMWAT : public ParserKeyword {
   public:
       SOMWAT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
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



   class SPOLY : public ParserKeyword {
   public:
       SPOLY();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SPOLYMW : public ParserKeyword {
   public:
       SPOLYMW();
       static const std::string keywordName;

       class data {
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



   class SSGCR : public ParserKeyword {
   public:
       SSGCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SSGL : public ParserKeyword {
   public:
       SSGL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SSOGCR : public ParserKeyword {
   public:
       SSOGCR();
       static const std::string keywordName;

       class data {
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



   class SSOWCR : public ParserKeyword {
   public:
       SSOWCR();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SSWL : public ParserKeyword {
   public:
       SSWL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SSWU : public ParserKeyword {
   public:
       SSWU();
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



   class STOG : public ParserKeyword {
   public:
       STOG();
       static const std::string keywordName;

       class REF_OIL_PHASE_PRESSURE {
       public:
           static const std::string itemName;
       };

       class table {
       public:
           static const std::string itemName;
       };
   };



   class STONE : public ParserKeyword {
   public:
       STONE();
       static const std::string keywordName;
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



   class STONE2 : public ParserKeyword {
   public:
       STONE2();
       static const std::string keywordName;
   };



   class STOW : public ParserKeyword {
   public:
       STOW();
       static const std::string keywordName;

       class REF_OIL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class table {
       public:
           static const std::string itemName;
       };
   };



   class STWG : public ParserKeyword {
   public:
       STWG();
       static const std::string keywordName;

       class REF_OIL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class table {
       public:
           static const std::string itemName;
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



   class SURF : public ParserKeyword {
   public:
       SURF();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SURFACT : public ParserKeyword {
   public:
       SURFACT();
       static const std::string keywordName;
   };



   class SURFACTW : public ParserKeyword {
   public:
       SURFACTW();
       static const std::string keywordName;
   };



   class SURFADDW : public ParserKeyword {
   public:
       SURFADDW();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SURFADS : public ParserKeyword {
   public:
       SURFADS();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SURFESAL : public ParserKeyword {
   public:
       SURFESAL();
       static const std::string keywordName;

       class COEFF {
       public:
           static const std::string itemName;
       };
   };



   class SURFNUM : public ParserKeyword {
   public:
       SURFNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class SURFOPTS : public ParserKeyword {
   public:
       SURFOPTS();
       static const std::string keywordName;

       class MIN_SWAT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SMOOTHING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class SURFROCK : public ParserKeyword {
   public:
       SURFROCK();
       static const std::string keywordName;

       class INDEX {
       public:
           static const std::string itemName;
       };

       class MASS_DENSITY {
       public:
           static const std::string itemName;
       };
   };



   class SURFST : public ParserKeyword {
   public:
       SURFST();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SURFSTES : public ParserKeyword {
   public:
       SURFSTES();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SURFVISC : public ParserKeyword {
   public:
       SURFVISC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class SURFWNUM : public ParserKeyword {
   public:
       SURFWNUM();
       static const std::string keywordName;

       class data {
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



   class SWINGFAC : public ParserKeyword {
   public:
       SWINGFAC();
       static const std::string keywordName;

       class SWING_FACTOR1 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR2 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR3 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR4 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR5 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR6 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR7 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR8 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR9 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR10 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR11 {
       public:
           static const std::string itemName;
       };

       class SWING_FACTOR12 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR1 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR2 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR3 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR4 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR5 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR6 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR7 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR8 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR9 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR10 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR11 {
       public:
           static const std::string itemName;
       };

       class PROFILE_FACTOR12 {
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



   class SWLPC : public ParserKeyword {
   public:
       SWLPC();
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
