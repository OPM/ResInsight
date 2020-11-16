#include <opm/output/eclipse/InteHEAD.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <ratio>
#include <utility>
#include <vector>

// Public INTEHEAD items are recorded in the common header file
//
//     opm/output/eclipse/VectorItems/intehead.hpp
//
// Promote items from 'index' to that list to make them public.
// The 'index' list always uses public items where available.
namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

enum index : std::vector<int>::size_type {
  ISNUM        =       VI::intehead::ISNUM,    //       0       0              An encoded integer corresponding to the time the file was created. For files not originating from ECLIPSE, this value may be set to zero.
  VERSION      =       VI::intehead::VERSION,  //       0       0
  UNIT         =       VI::intehead::UNIT,     //       (1,2,3)       1              units type: 1 - METRIC, 2 - FIELD, 3 - LAB
  ih_003       =       3       ,               //       0       0
  ih_004       =       4       ,               //       0       0
  ih_005       =       5       ,               //       0       0
  ih_006       =       6       ,               //       0       0
  ih_007       =       7       ,               //       0       0
  NX           =       VI::intehead::NX,       //       NX       137              Grid x-direction dimension, NX
  NY           =       VI::intehead::NY,       //       NY       236              Grid x-direction dimension, NY
  NZ           =       VI::intehead::NZ,       //       NZ       58              Grid x-direction dimension, NZ
  NACTIV       =       VI::intehead::NACTIV,   //       NACTIV?       89022              NACTIV = number of active cells
  ih_012       =       12       ,              //       0       0
  ih_013       =       13       ,              //       0       0
  PHASE        =       VI::intehead::PHASE,    //       IPHS       7              IPHS = phase indicator: 1 - oil, 2 - water, 3 - oil/water, 4 - gas, 5 – oil/gas, 6 - gas/water, 7 - oil/water/gas (ECLIPSE output only)
  ih_015       =       15       ,              //       0       0
  NWELLS       =       VI::intehead::NWELLS,   //       NWELLS       39              NWELL = number of wells
  NCWMAX       =       VI::intehead::NCWMAX,   //       NCWMAX       108       Weldims item2       NCWMAX = maximum number of completions per well
  NGRP         =       VI::intehead::NGRP,     //       NGRP?        0       Number of actual groups
  NWGMAX       =       VI::intehead::NWGMAX,   //       NWGMAX       0       maximum of weldims item3 or item4       NWGMAX = maximum number of wells in any well group
  NGMAXZ       =       VI::intehead::NGMAXZ,   //       NGMAXZ       0       weldims item3 + 1       NGMAXZ = maximum number of groups in field
  ih_021       =       21       ,              //       0       0
  ih_022       =       22       ,              //       0       0
  ih_023       =       23       ,              //       0       0
  NIWELZ       =       VI::intehead::NIWELZ,   //       NIWELZ       155       155       NIWELZ = no of data elements per well in IWEL array (default 97 for ECLIPSE, 94 for ECLIPSE 300)
  NSWELZ       =       VI::intehead::NSWELZ,   //       NSWELZ       122       122       NSWELZ = number of daelements per well in SWEL array
  NXWELZ       =       VI::intehead::NXWELZ,   //       NXWELZ       130       130       NXWELZ = number of delements per well in XWEL array
  NZWELZ       =       VI::intehead::NZWELZ,   //       NZWEL       3       3       NZWEL = no of 8-character words per well in ZWEL array (= 3)
  ih_028       =       28       ,              //       0       0
  ih_029       =       29       ,              //       0       0
  ih_030       =       30       ,              //       0       0
  ih_031       =       31       ,              //       0       0
  NICONZ       =       VI::intehead::NICONZ,   //       25       15       25       NICON = no of data elements per completion in ICON array (default 19)
  NSCONZ       =       VI::intehead::NSCONZ,   //       41       0              NSCONZ = number of data elements per completion in SCON array
  NXCONZ       =       VI::intehead::NXCONZ,   //       58       0       58       NXCONZ = number of data elements per completion in XCON array
  ih_035       =       35       ,              //       0       0
  NIGRPZ       =       VI::intehead::NIGRPZ,   //       97+intehead_array[19]       0       97 + intehead[19]       NIGRPZ = no of data elements per group in IGRP array
  NSGRPZ       =       VI::intehead::NSGRPZ,   //       112       0       112       NSGRPZ = number of data elements per group in SGRP array
  NXGRPZ       =       VI::intehead::NXGRPZ,   //       180       0       180       NXGRPZ = number of data elements per group in XGRP array
  NZGRPZ       =       VI::intehead::NZGRPZ,   //       5       0              NZGRPZ = number of data elements per group in ZGRP array
  ih_040       =       40       ,              //       0       0
  NCAMAX       =       VI::intehead::NCAMAX,   //       1       0              NCAMAX = maximum number of analytic aquifer connections
  NIAAQZ       =       VI::intehead::NIAAQZ,   //       18       0              NIAAQZ = number of data elements per aquifer in IAAQ array
  NSAAQZ       =       VI::intehead::NSAAQZ,   //       24       0              NSAAQZ = number of data elements per aquifer in SAAQ array
  NXAAQZ       =       VI::intehead::NXAAQZ,   //       10       0              NXAAQZ = number of data elements per aquifer in XAAQ array
  NICAQZ       =       VI::intehead::NICAQZ,   //       7       0              NSCAQZ= number of data elements per aquifer connection in SCAQ array
  NSCAQZ       =       VI::intehead::NSCAQZ,   //       2       0
  NACAQZ       =       VI::intehead::NACAQZ,   //       4       0
  ih_048       =       48       ,              //       0       0
  ih_049       =       49       ,              //       1       // has been determined by testing
  ih_050       =       50       ,              //       1       // has been determined by testing
  NGCONT       =       VI::intehead::NGCTRL,   //       0 - no group control, 1 if GCONPROD, 2 if GCONINJE
  ih_052       =       52       ,              //       0       0
  ih_053       =       53       ,              //       0       0
  ih_054       =       54       ,              //       0       0
  ih_055       =       55       ,              //       0       0
  ih_056       =       56       ,              //       0       0
  ih_057       =       57       ,              //       0       0
  NGRNPHASE    =       VI::intehead::NGRNPH,   //       Parameter to determine the nominated phase for the guiderate 
  ih_059       =       59       ,              //       0       0
  ih_060       =       60       ,              //       0       0
  ih_061       =       61       ,              //       0       0
  ih_062       =       62       ,              //       0       0
  ih_063       =       63       ,              //       0       0
  DAY          =       VI::intehead::DAY,      //       IDAY          2     IDAY = calendar day at this report time
  MONTH        =       VI::intehead::MONTH,    //       IMON          6     IMON = calendar month at this report time
  YEAR         =       VI::intehead::YEAR,     //       IYEAR      2016     IYEAR = calendar year at this report time
  NUM_SOLVER_STEPS  =  VI::intehead::NUM_SOLVER_STEPS,         //  The number of solver steps the simulator has performed so far.
  REPORT_STEP       =  VI::intehead::REPORT_STEP,              // The sequence/report number for for this restart file.
  ih_069       =       69       ,              //       0       0
  ih_070       =       70       ,              //       0       0
  NWHISTCTL    =       VI::intehead::WHISTC,   //       index for WHISTCTL keyword
  ih_072       =       72       ,              //       0       0
  ih_073       =       73       ,              //       0       0
  ih_074       =       74       ,              //       0       0
  ih_075       =       75       ,              //       0       0
  ih_076       =       76       ,              //       0       0       2
  ih_077       =       77       ,              //       0       0
  ih_078       =       78       ,              //       0       0
  ih_079       =       79       ,              //       0       0
  NEWTMX       =       VI::intehead::NEWTMX,   //       0       0       Tuning,Record3,Item1
  NEWTMN       =       VI::intehead::NEWTMN,   //       0       0       Tuning,Record3,Item2
  LITMAX       =       VI::intehead::LITMAX,   //       0       0       Tuning,Record3,Item3
  LITMIN       =       VI::intehead::LITMIN,   //       0       0       Tuning,Record3,Item4
  ih_084       =       84       ,              //       0       0       Tuning,Record3,Item5
  ih_085       =       85       ,              //       0       0       Tuning,Record3,Item6
  MXWSIT       =       VI::intehead::MXWSIT,   //       0       0
  MXWPIT       =       VI::intehead::MXWPIT,   //       0       0
  ih_088       =       88       ,              //       0       0
  NTFIP        =       VI::intehead::NTFIP,    //       0       0
  ih_090       =       90       ,              //       0       0
  ih_091       =       91       ,              //       0       0
  ih_092       =       92       ,              //       0       0
  ih_093       =       93       ,              //       0       0
  IPROG        =       VI::intehead::IPROG,    //       0       100
  INITSIZE     =       95       ,              //       0       0
  ih_096       =       96       ,              //       0       0
  ih_097       =       97       ,              //       0       0
  ih_098       =       98       ,              //       0       0
  NMFIPR       =       VI::intehead::NMFIPR,   //       0       0
  ih_100       =      100       ,              //       0       0
  ih_101       =      101       ,              //       0       0       1
  ih_102       =      102       ,              //       0       0
  ih_103       =      103       ,              //       0       0       1
  ih_104       =      104       ,              //       0       0
  ih_105       =      105       ,              //       0       0
  ih_106       =      106       ,              //       0       0
  ih_107       =      107       ,              //       0       0
  ih_108       =      108       ,              //       0       0
  ih_109       =      109       ,              //       0       0
  ih_110       =      110       ,              //       0       0
  ih_111       =      111       ,              //       0       0
  ih_112       =      112       ,              //       0       0
  ih_113       =      113       ,              //       0       0
  ih_114       =      114       ,              //       0       0
  ih_115       =      115       ,              //       0       0
  ih_116       =      116       ,              //       0       0
  ih_117       =      117       ,              //       0       0
  ih_118       =      118       ,              //       0       0
  ih_119       =      119       ,              //       0       0
  ih_120       =      120       ,              //       0       0
  ih_121       =      121       ,              //       0       0
  ih_122       =      122       ,              //       0       0
  ih_123       =      123       ,              //       0       0
  ih_124       =      124       ,              //       0       0
  ih_125       =      125       ,              //       0       0
  ih_126       =      126       ,              //       0       0
  ih_127       =      127       ,              //       0       0
  ih_128       =      128       ,              //       0       0
  ih_129       =      129       ,              //       0       0
  ih_130       =      130       ,              //       0       0
  NODMAX       =      131       ,              //       0       0              NODMAX = maximum number of nodes in extended network option
  NBRMAX       =      132       ,              //       0       0              NBRMAX = maximum number of branches in extended network option
  NIBRAN       =      133       ,              //       0       0              NIBRAN = number of entries per branch in the IBRAN array
  NRBRAN       =      134       ,              //       0       0              NRBRAN = number of tries per branch in the RBRAN array
  NINODE       =      135       ,              //       0       0              NINODE = number of entries per node in the INODE array
  NRNODE       =      136       ,              //       0       0              NRNODE = number of entries per node in the RNODE array
  NZNODE       =      137       ,              //       0       0              NZNODE = number of entries per node in the ZNODE array
  NINOBR       =      138       ,              //       0       0              NINOBR = size of the INOBR array
  ih_139       =      139       ,              //       0       0
  ih_140       =      140       ,              //       0       0
  ih_141       =      141       ,              //       0       0
  ih_142       =      142       ,              //       0       0
  ih_143       =      143       ,              //       0       0
  ih_144       =      144       ,              //       0       0
  ih_145       =      145       ,              //       0       0
  ih_146       =      146       ,              //       0       0
  ih_147       =      147       ,              //       0       0
  ih_148       =      148       ,              //       0       0
  ih_149       =      149       ,              //       0       0
  ih_150       =      150       ,              //       0       0
  ih_151       =      151       ,              //       0       0
  ih_152       =      152       ,              //       0       0
  ih_153       =      153       ,              //       0       0
  ih_154       =      154       ,              //       0       0
  ih_155       =      155       ,              //       0       0
  NO_ACT       =    VI::intehead::NOOFACTIONS, //       0       0
  MAX_LINES    =    VI::intehead::MAXNOLINES,  //       0       0
  MAXSPRLINE   =    VI::intehead::MAXNOSTRPRLINE,  //       0       0
  ih_159       =      159       ,              //       0       0
  ih_160       =      160       ,              //       0       0
  ih_161       =      161       ,              //       0       0
  NGCAUS       =      162       ,              //       0       0              NGCAUS = maximum number of aquifer connections actually used.
  NWMAXZ       =      VI::intehead::NWMAXZ,    //       0       0
  ih_164       =      164       ,              //       0       0
  ih_165       =      165       ,              //       0       0
  ih_166       =      166       ,              //       0       0
  ih_167       =      167       ,              //       0       0
  ih_168       =      168       ,              //       0       0
  ih_169       =      169       ,              //       0       0
  ih_170       =      170       ,              //       0       0
  ih_171       =      171       ,              //       0       0
  ih_172       =      172       ,              //       0       0
  ih_173       =      173       ,              //       0       0
  NSEGWL       =      VI::intehead::NSEGWL,    //       0       0       number of mswm wells defined with WELSEG
  NSWLMX       =      VI::intehead::NSWLMX,    //       NSWLMX       0       Item 1 in WSEGDIMS keyword (runspec section)       NSWLMX = maximum number of segmented wells
  NSEGMX       =      VI::intehead::NSEGMX,    //       NSEGMX       0       Item 2 in WSEGDIMS keyword (runspec section)       NSEGMX = maximum number of segments per well
  NLBRMX       =      VI::intehead::NLBRMX,    //       NLBRMX       0       Item 3 in WSEGDIMS keyword (runspec section)       NLBRMX = maximum number of lateral branches per well
  NISEGZ       =      VI::intehead::NISEGZ,    //       22       0       22       NISEGZ = number of entries per segment in ISEG array
  NRSEGZ       =      VI::intehead::NRSEGZ,    //       146       0       140       NRSEGZ = number of entries per segment in RSEG array
  NILBRZ       =      VI::intehead::NILBRZ,    //       10              10       NILBRZ = number of entries per segment in ILBR array
  RSTSIZE      =      181       ,              //       0
  ih_182       =      182       ,              //       0
  ih_183       =      183       ,              //       0
  ih_184       =      184       ,              //       0
  ih_185       =      185       ,              //       0
  ih_186       =      186       ,              //       0
  ih_187       =      187       ,              //       0
  ih_188       =      188       ,              //       0
  ih_189       =      189       ,              //       0
  ih_190       =      190       ,              //       0
  ih_191       =      191       ,              //       0
  ih_192       =      192       ,              //       0
  ih_193       =      193       ,              //       0
  ih_194       =      194       ,              //       0
  ih_195       =      195       ,              //       0
  ih_196       =      196       ,              //       0
  ih_197       =      197       ,              //       0
  ih_198       =      198       ,              //       0
  ih_199       =      199       ,              //       0
  ih_200       =      200       ,              //       0
  ih_201       =      201       ,              //       0
  ih_202       =      202       ,              //       0
  ih_203       =      203       ,              //       0
  ih_204       =      204       ,              //       0
  ih_205       =      205       ,              //       0
  IHOURZ       =      VI::intehead::IHOURZ,
  IMINTS       =      VI::intehead::IMINTS,
  ih_208       =      208       ,              //       0
  ih_209       =      209       ,              //       0
  ih_210       =      210       ,              //       0
  ih_211       =      211       ,              //       0
  ih_212       =      212       ,              //       0
  ih_213       =      213       ,              //       0
  ih_214       =      214       ,              //       0
  ih_215       =      215       ,              //       0
  ih_216       =      216       ,              //       0
  ih_217       =      217       ,              //       0
  ih_218       =      218       ,              //       0
  ih_219       =      219       ,              //       0
  ih_220       =      220       ,              //       0
  ih_221       =      221       ,              //       0
  ih_222       =      222       ,              //       0
  NIIAQN       =      223       ,              //       0                     NIIAQN = number of lines of integer AQUNUM data.
  NIRAQN       =      224       ,              //       0                     NIRAQN = number of lines of real AQUNUM data.
  ih_225       =      225       ,              //       0
  NUMAQN       =      226       ,              //       0                     NUMAQN = number of lines of AQUNUM data entered.
  ih_227       =      227       ,              //       0
  ih_228       =      228       ,              //       0
  ih_229       =      229       ,              //       0
  ih_230       =      230       ,              //       0
  ih_231       =      231       ,              //       0
  ih_232       =      232       ,              //       0
  ih_233       =      233       ,              //       0
  NICOTZ       =      234       ,              //       0                     NICOTZ = number of entries in the ICOT array
  NXCOTZ       =      235       ,              //       0                     NXCOTZ = number of entries in the XCOT array
  NIWETZ       =      236       ,              //       0                     NIWETZ = number of entries in the IWET array
  NXWETZ       =      237       ,              //       0                     NXWETZ = number of entries in the XWET array
  NIGRTZ       =      238       ,              //       0                     NIGRTZ = number of entries in the IGRT array
  NXGRTZ       =      239       ,              //       0                     NXGRTZ = number of entries in the XGRT array
  NSTRA2       =      240       ,              //       0                     NSTRA2 = number of tracers + 2
  ih_241       =      241       ,              //       0
  ih_242       =      242       ,              //       0
  ih_243       =      243       ,              //       0
  ih_244       =      244       ,              //       0
  MXACTC       =      VI::intehead::MAX_ACT_COND, //       Max no of conditions pr action 
  ih_246       =      246       ,              //       0
  ih_247       =      247       ,              //       0
  ih_248       =      248       ,              //       0
  ih_249       =      249       ,              //       0
  ih_250       =      250       ,              //       0
  ih_251       =      251       ,              //       0
  MAAQID       =      VI::intehead::MAX_AN_AQUIFERS,              //       0                     MAAQID = maximum number of analytic aquifers
  ih_253       =      253       ,              //       0
  ih_254       =      254       ,              //       0
  ih_255       =      255       ,              //       0
  ih_256       =      256       ,              //       0
  ih_257       =      257       ,              //       0
  ih_258       =      258       ,              //       0
  ih_259       =      259       ,              //       0
  ih_260       =      260       ,              //       0
  ih_261       =      261       ,              //       0
  NOFUDQS      =      VI::intehead::NO_FIELD_UDQS,    //       0
  NOGUDQS      =      VI::intehead::NO_GROUP_UDQS,    //       0
  ih_264       =      264       ,              //       0
  ih_265       =      265       ,              //       0
  NOWUDQS      =      VI::intehead::NO_WELL_UDQS,     //       0
  UDQPAR_1     =      VI::intehead::UDQPAR_1,  //       0
  ih_268       =      268       ,              //       0
  ih_269       =      269       ,              //       0
  ih_270       =      270       ,              //       0
  NCRDMX       =      271       ,              //       0                     NCRDMX = maximum number of chord segment links per well
  ih_272       =      272       ,              //       0
  ih_273       =      273       ,              //       0
  ih_274       =      274       ,              //       0
  ih_275       =      275       ,              //       0
  ih_276       =      276       ,              //       0
  ih_277       =      277       ,              //       0
  ih_278       =      278       ,              //       0
  ih_279       =      279       ,              //       0
  ih_280       =      280       ,              //       0
  ih_281       =      281       ,              //       0
  ih_282       =      282       ,              //       0
  ih_283       =      283       ,              //       0
  ih_284       =      284       ,              //       0
  ih_285       =      285       ,              //       0
  ih_286       =      286       ,              //       0
  ih_287       =      287       ,              //       0
  ih_288       =      288       ,              //       0
  ih_289       =      289       ,              //       0
  NOIUADS       =      VI::intehead::NO_IUADS,  //       0
  NOIUAPS       =      VI::intehead::NO_IUAPS,  //       0
  ih_292       =      292       ,              //       0
  ih_293       =      293       ,              //       0
  ih_294       =      294       ,              //       0
  ih_295       =      295       ,              //       0
  R_SEED       =      VI::intehead::RSEED,     //       0
  ih_297       =      297       ,              //       0
  ih_298       =      298       ,              //       0
  ih_299       =      299       ,              //       0
  ih_300       =      300       ,              //       0
  ih_301       =      301       ,              //       0
  ih_302       =      302       ,              //       0
  ih_303       =      303       ,              //       0
  ih_304       =      304       ,              //       0
  ih_305       =      305       ,              //       0
  ih_306       =      306       ,              //       0
  ih_307       =      307       ,              //       0
  ih_308       =      308       ,              //       0
  ih_309       =      309       ,              //       0
  ih_310       =      310       ,              //       0
  ih_311       =      311       ,              //       0
  ih_312       =      312       ,              //       0
  ih_313       =      313       ,              //       0
  ih_314       =      314       ,              //       0
  ih_315       =      315       ,              //       0
  ih_316       =      316       ,              //       0
  ih_317       =      317       ,              //       0
  ih_318       =      318       ,              //       0
  ih_319       =      319       ,              //       0
  ih_320       =      320       ,              //       0
  ih_321       =      321       ,              //       0
  ih_322       =      322       ,              //       0
  ih_323       =      323       ,              //       0
  ih_324       =      324       ,              //       0
  ih_325       =      325       ,              //       0
  ih_326       =      326       ,              //       0
  ih_327       =      327       ,              //       0
  ih_328       =      328       ,              //       0
  ih_329       =      329       ,              //       0
  ih_330       =      330       ,              //       0
  ih_331       =      331       ,              //       0
  ih_332       =      332       ,              //       0
  ih_333       =      333       ,              //       0
  ih_334       =      334       ,              //       0
  ih_335       =      335       ,              //       0
  ih_336       =      336       ,              //       0
  ih_337       =      337       ,              //       0
  ih_338       =      338       ,              //       0
  ih_339       =      339       ,              //       0
  ih_340       =      340       ,              //       0
  ih_341       =      341       ,              //       0
  ih_342       =      342       ,              //       0
  ih_343       =      343       ,              //       0
  ih_344       =      344       ,              //       0
  ih_345       =      345       ,              //       0
  ih_346       =      346       ,              //       0
  ih_347       =      347       ,              //       0
  ih_348       =      348       ,              //       0
  ih_349       =      349       ,              //       0
  ih_350       =      350       ,              //       0
  ih_351       =      351       ,              //       0
  ih_352       =      352       ,              //       0
  ih_353       =      353       ,              //       0
  ih_354       =      354       ,              //       0
  ih_355       =      355       ,              //       0
  ih_356       =      356       ,              //       0
  ih_357       =      357       ,              //       0
  ih_358       =      358       ,              //       0
  ih_359       =      359       ,              //       0
  ih_360       =      360       ,              //       0
  ih_361       =      361       ,              //       0
  ih_362       =      362       ,              //       0
  ih_363       =      363       ,              //       0
  ih_364       =      364       ,              //       0
  ih_365       =      365       ,              //       0
  ih_366       =      366       ,              //       0
  ih_367       =      367       ,              //       0
  ih_368       =      368       ,              //       0
  ih_369       =      369       ,              //       0
  ih_370       =      370       ,              //       0
  ih_371       =      371       ,              //       0
  ih_372       =      372       ,              //       0
  ih_373       =      373       ,              //       0
  ih_374       =      374       ,              //       0
  ih_375       =      375       ,              //       0
  ih_376       =      376       ,              //       0
  ih_377       =      377       ,              //       0
  ih_378       =      378       ,              //       0
  ih_379       =      379       ,              //       0
  ih_380       =      380       ,              //       0
  ih_381       =      381       ,              //       0
  ih_382       =      382       ,              //       0
  ih_383       =      383       ,              //       0
  ih_384       =      384       ,              //       0
  ih_385       =      385       ,              //       0
  ih_386       =      386       ,              //       0
  ih_387       =      387       ,              //       0
  ih_388       =      388       ,              //       0
  ih_389       =      389       ,              //       0
  ih_390       =      390       ,              //       0
  ih_391       =      391       ,              //       0
  ih_392       =      392       ,              //       0
  ih_393       =      393       ,              //       0
  ih_394       =      394       ,              //       0
  ih_395       =      395       ,              //       0
  ih_396       =      396       ,              //       0
  ih_397       =      397       ,              //       0
  ih_398       =      398       ,              //       0
  ih_399       =      399       ,              //       0
  ih_400       =      400       ,              //       0
  ih_401       =      401       ,              //       0
  ih_402       =      402       ,              //       0
  ih_403       =      403       ,              //       0
  ih_404       =      404       ,              //       0
  ih_405       =      405       ,              //       0
  ih_406       =      406       ,              //       0
  ih_407       =      407       ,              //       0
  ih_408       =      408       ,              //       0
  ih_409       =      409       ,              //       0
  ISECND       =      VI::intehead::ISECND,
 // ---------------------------------------------------------------------
 // ---------------------------------------------------------------------

  INTEHEAD_NUMBER_OF_ITEMS        // MUST be last element of enum.
};

// =====================================================================
// Public Interface Below Separator
// =====================================================================

Opm::RestartIO::InteHEAD::InteHEAD()
    : data_(INTEHEAD_NUMBER_OF_ITEMS, 0)
{}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
dimensions(const int nx, const int ny, const int nz)
{
    this -> data_[NX] = nx;
    this -> data_[NY] = ny;
    this -> data_[NZ] = nz;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
dimensions(const std::array<int,3>& cartDims)
{
    return this->dimensions(cartDims[0], cartDims[1], cartDims[2]);
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::numActive(const int nactive)
{
    this->data_[NACTIV] = nactive;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::unitConventions(const UnitSystem& usys)
{
    this->data_[UNIT] = usys.ecl_id();
    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::wellTableDimensions(const WellTableDim& wtdim)
{
    this->data_[NWELLS] = wtdim.numWells;
    this->data_[NCWMAX] = wtdim.maxPerf;

    this->data_[NWGMAX] = std::max(wtdim.maxWellInGroup,
                                   wtdim.maxGroupInField);

    this->data_[NGMAXZ] = wtdim.maxGroupInField + 1;
    
    this->data_[NWMAXZ] = wtdim.maxWellsInField;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::calendarDate(const TimePoint& timePoint)
{
    this->data_[DAY]   = timePoint.day;
    this->data_[MONTH] = timePoint.month;
    this->data_[YEAR]  = timePoint.year;

    this->data_[IHOURZ] = timePoint.hour;
    this->data_[IMINTS] = timePoint.minute;

    // Microseonds...
    this->data_[ISECND] =
        ((timePoint.second * 1000) * 1000) + timePoint.microseconds;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::activePhases(const Phases& phases)
{
    const auto iphs =
        (static_cast<unsigned int>  (phases.oil)   << 0u)
        | (static_cast<unsigned int>(phases.water) << 1u)
        | (static_cast<unsigned int>(phases.gas)   << 2u);

    this->data_[PHASE] = static_cast<int>(iphs);

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
params_NWELZ(const int niwelz, const int nswelz, const int nxwelz, const int nzwelz)
{
    this -> data_[NIWELZ] = niwelz;
    this -> data_[NSWELZ] = nswelz;
    this -> data_[NXWELZ] = nxwelz;
    this -> data_[NZWELZ] = nzwelz;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
params_NCON(const int niconz, const int nsconz, const int nxconz)
{
    this -> data_[NICONZ] = niconz;
    this -> data_[NSCONZ] = nsconz;
    this -> data_[NXCONZ] = nxconz;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
params_GRPZ(const std::array<int, 4>& grpz)
{
    this -> data_[NIGRPZ] = grpz[0];
    this -> data_[NSGRPZ] = grpz[1];
    this -> data_[NXGRPZ] = grpz[2];
    this -> data_[NZGRPZ] = grpz[3];

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
params_NGCTRL(const int gct)
{
    this -> data_[NGCONT] = gct;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
params_NAAQZ(const int ncamax,
             const int niaaqz,
             const int nsaaqz,
             const int nxaaqz,
             const int nicaqz,
             const int nscaqz,
             const int nacaqz)
{
    this -> data_[NCAMAX] = ncamax;
    this -> data_[NIAAQZ] = niaaqz;
    this -> data_[NSAAQZ] = nsaaqz;
    this -> data_[NXAAQZ] = nxaaqz;
    this -> data_[NICAQZ] = nicaqz;
    this -> data_[NSCAQZ] = nscaqz;
    this -> data_[NACAQZ] = nacaqz;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
stepParam(const int num_solver_steps, const int report_step)
{
    this -> data_[NUM_SOLVER_STEPS] = num_solver_steps;
    this -> data_[REPORT_STEP]      = report_step;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::tuningParam(const TuningPar& tunpar)
{
    this->data_[NEWTMX] = tunpar.newtmx;
    this->data_[NEWTMN] = tunpar.newtmn;
    this->data_[LITMAX] = tunpar.litmax;
    this->data_[LITMIN] = tunpar.litmin;
    this->data_[MXWSIT] = tunpar.mxwsit;
    this->data_[MXWPIT] = tunpar.mxwpit;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::variousParam(const int version,
                                       const int iprog)
{
    this->data_[VERSION] = version;
    this->data_[IPROG]   = iprog;

    // ih_049: Usage unknown, value fixed across reference cases
    this->data_[ih_049]  = 1;

    // ih_050: Usage unknown, value fixed across reference cases
    this->data_[ih_050]  = 1;

    // ih_076: Usage unknown, experiments fails (zero determinant
    //         in well message) with too low numbers.
    //         5 is highest observed across reference cases.
    this->data_[ih_076] = 5;

    // ih_101: Usage unknown, value fixed across reference cases.
    this->data_[ih_101] = 1;

    // ih_103: Usage unknown, value not fixed across reference cases,
    //         experiments generate warning with 0 but not with 1.
    this->data_[ih_103] = 1;

    // ih_200: Usage unknown, value fixed across reference cases.
    this->data_[ih_200] = 1;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::wellSegDimensions(const WellSegDims& wsdim)
{
    this->data_[NSEGWL] = wsdim.nsegwl;
    this->data_[NSWLMX] = wsdim.nswlmx;
    this->data_[NSEGMX] = wsdim.nsegmx;
    this->data_[NLBRMX] = wsdim.nlbrmx;
    this->data_[NISEGZ] = wsdim.nisegz;
    this->data_[NRSEGZ] = wsdim.nrsegz;
    this->data_[NILBRZ] = wsdim.nilbrz;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::regionDimensions(const RegDims& rdim)
{
    this->data_[NTFIP]  = rdim.ntfip;
    this->data_[NMFIPR] = rdim.nmfipr;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
ngroups(const Group& gr)
{
    this -> data_[NGRP] = gr.ngroups;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
udqParam_1(const UdqParam& udq_par)
{
    this -> data_[UDQPAR_1]     = - udq_par.udqParam_1;
    this -> data_[R_SEED]       = - udq_par.udqParam_1;
    this -> data_[NOWUDQS]      = udq_par.no_wudqs;
    this -> data_[NOGUDQS]      = udq_par.no_gudqs;
    this -> data_[NOFUDQS]      = udq_par.no_fudqs;
    this -> data_[NOIUADS]      = udq_par.no_iuads;
    this -> data_[NOIUAPS]      = udq_par.no_iuaps;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
actionParam(const ActionParam& act_par)
{
    this -> data_[NO_ACT]     = act_par.no_actions;
    this -> data_[MAX_LINES]  = act_par.max_no_sched_lines_per_action;
    this -> data_[MXACTC]     = act_par.max_no_conditions_per_action;
    this -> data_[MAXSPRLINE] = ((act_par.max_no_characters_per_line % 8) == 0) ? act_par.max_no_characters_per_line / 8 : 
                                (act_par.max_no_characters_per_line / 8) + 1;

    return *this;
}


//InteHEAD parameters which meaning are currently not known, but which are needed for Eclipse restart runs with UDQ and ACTIONX data
Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
variousUDQ_ACTIONXParam()
{
    this -> data_[159]  =  4;
    this -> data_[160]  =  5;
    this -> data_[161]  =  9;
    this -> data_[246]  = 26;
    this -> data_[247]  = 16;
    this -> data_[248]  = 13;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
nominatedPhaseGuideRate(GuideRateNominatedPhase nphase)
{
    this -> data_[NGRNPHASE]  =  nphase.nominated_phase;

    return *this;
}

Opm::RestartIO::InteHEAD&
Opm::RestartIO::InteHEAD::
whistControlMode(int mode)
{
    this -> data_[NWHISTCTL]  =  mode;

    return *this;
}

// =====================================================================
// Free functions (calendar/time utilities)
// =====================================================================

namespace {
    std::time_t advance(const std::time_t tp, const double sec)
    {
        using namespace std::chrono;

        using TP      = time_point<system_clock>;
        using DoubSec = duration<double, seconds::period>;

        const auto t = system_clock::from_time_t(tp) +
            duration_cast<TP::duration>(DoubSec(sec));

        return system_clock::to_time_t(t);
    }
}

std::time_t
Opm::RestartIO::makeUTCTime(const std::tm& timePoint)
{
    auto       tp    =  timePoint; // Mutable copy.
    const auto ltime =  std::mktime(&tp);
    auto       tmval = *std::gmtime(&ltime); // Mutable.

    // offset =  ltime - tmval
    //        == #seconds by which 'ltime' is AHEAD of tmval.
    const auto offset =
        std::difftime(ltime, std::mktime(&tmval));

    // Advance 'ltime' by 'offset' so that std::gmtime(return value) will
    // have the same broken-down elements as 'tp'.
    return advance(ltime, offset);
}

Opm::RestartIO::InteHEAD::TimePoint
Opm::RestartIO::getSimulationTimePoint(const std::time_t start,
                                       const double      elapsed)
{
    const auto now = advance(start, elapsed);
    const auto tp  = *std::gmtime(&now);

    auto sec  = 0.0;            // Not really used here.
    auto usec = std::floor(1.0e6 * std::modf(elapsed, &sec));

    return {
        // Y-m-d
        tp.tm_year + 1900,
        tp.tm_mon  +    1,
        tp.tm_mday ,

        // H:M:S
        tp.tm_hour ,
        tp.tm_min  ,
        std::min(tp.tm_sec, 59), // Ignore leap seconds

        // Fractional seconds in microsecond resolution.
        static_cast<int>(usec),
    };
}
