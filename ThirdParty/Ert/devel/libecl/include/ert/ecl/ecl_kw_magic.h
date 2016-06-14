#ifndef ERT_ECL_KW_MAGIC_H
#define ERT_ECL_KW_MAGIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
   This header file contains names and indices of ECLIPSE keywords
   which have special significance in various files. Observe that many
   of the keywords like e.g. INTEHEAD occur in many different file
   types, with partly overlapping layout and values.
 */

/*****************************************************************/
/*                INIT files:                                    */
/*****************************************************************/

#define PORO_KW      "PORO"
#define PORV_KW      "PORV"
#define AQUIFER_KW   "AQUIFERN"
#define INTEHEAD_KW  "INTEHEAD"
#define LOGIHEAD_KW  "LOGIHEAD"
#define DOUBHEAD_KW  "DOUBHEAD"
#define RPORV_KW     "RPORV"
#define PORV_KW      "PORV"
#define PORMOD_KW    "PORV_MOD"

#define PVTNUM_KW    "PVTNUM"
#define LGRHEADI_KW  "LGRHEADI"
#define LGRHEADI_LGR_NR_INDEX 0
#define LGRJOIN_KW   "LGRJOIN"

/*
   Observe that many of the elements in the INTEHEAD keyword is shared
   between the restart and init files. The ones listed below here are
   in both the INIT and the restart files. In addition the restart
   files have many well related items which are only in the restart
   files.
*/


#define INTEHEAD_UNIT_INDEX     2
#define INTEHEAD_NX_INDEX       8
#define INTEHEAD_NY_INDEX       9
#define INTEHEAD_NZ_INDEX      10
#define INTEHEAD_NACTIVE_INDEX 11
#define INTEHEAD_PHASE_INDEX   14
#define INTEHEAD_DAY_INDEX     64
#define INTEHEAD_MONTH_INDEX   65
#define INTEHEAD_YEAR_INDEX    66
#define INTEHEAD_IPROG_INDEX   94


#define INTEHEAD_METRIC_VALUE              1
#define INTEHEAD_ECLIPSE100_VALUE        100
#define INTEHEAD_ECLIPSE300_VALUE        300
#define INTEHEAD_ECLIPSE300THERMAL_VALUE 500

#define INTEHEAD_INIT_SIZE                95
#define INTEHEAD_RESTART_SIZE             180

#define LOGIHEAD_ECLIPSE300_RADIAL_INDEX   3
#define LOGIHEAD_ECLIPSE100_RADIAL_INDEX   4
#define LOGIHEAD_DUALP_INDEX              14
#define LOGIHEAD_INIT_SIZE                80
#define LOGIHEAD_RESTART_SIZE             15


#define LOGIHEAD_RS_INDEX                           0
#define LOGIHEAD_RV_INDEX                           1
#define LOGIHEAD_DIR_RELPERM_INDEX                  2
/*-----------------------------------------------------------------*/
#define LOGIHEAD_REV_RELPERM100_INDEX               3   /* The indices for reversible relperm and */
#define LOGIHEAD_RADIAL100_INDEX                    4   /* use of radial grids is interchanged between */
#define LOGIHEAD_REV_RELPERM300_INDEX               4   /* ECLIPSE100 and ECLIPSE300. */
#define LOGIHEAD_RADIAL300_INDEX                    3
/*-----------------------------------------------------------------*/
#define LOGIHEAD_HYSTERISIS_INDEX                   6
#define LOGIHEAD_DUALP_INDEX                       14
#define LOGIHEAD_ENDPOINT_SCALING_INDEX            16
#define LOGIHEAD_DIR_ENDPOINT_SCALING_INDEX        17
#define LOGIHEAD_REV_ENDPOINT_SCALING_INDEX        18
#define LOGIHEAD_ALT_ENDPOINT_SCALING_INDEX        19
#define LOGIHEAD_MISC_DISPLACEMENT_INDEX           35
#define LOGIHEAD_SCALE_WATER_PC_AT_MAX_SAT_INDEX   55
#define LOGIHEAD_SCALE_GAS_PC_AT_MAX_SAT_INDEX     56




#define DOUBHEAD_INIT_SIZE                 1
#define DOUBHEAD_RESTART_SIZE              1


/*****************************************************************/
/*                RESTART files:                                 */
/*****************************************************************/

#define SEQNUM_KW    "SEQNUM"       /* Contains the report step as the only data; not
                                       present in non-unified files, where the report
                                       step must be inferred from the filename. */
#define STARTSOL_KW  "STARTSOL"
#define ENDSOL_KW    "ENDSOL"

#define IWEL_KW      "IWEL"
#define ZWEL_KW      "ZWEL"
#define ICON_KW      "ICON"
#define SCON_KW      "SCON"
#define ISEG_KW      "ISEG"
#define RSEG_KW      "RSEG"

#define ECLIPSE100_OIL_DEN_KW   "OIL_DEN"
#define ECLIPSE100_GAS_DEN_KW   "GAS_DEN"
#define ECLIPSE100_WATER_DEN_KW "WAT_DEN"

#define ECLIPSE300_OIL_DEN_KW   "DENO"
#define ECLIPSE300_GAS_DEN_KW   "DENG"
#define ECLIPSE300_WATER_DEN_KW "DENW"

#define FIPGAS_KW               "FIPGAS"
#define FIPWAT_KW               "FIPWAT"
#define FIPOIL_KW               "FIPOIL"
#define RFIPGAS_KW              "RFIPGAS"
#define RFIPWAT_KW              "RFIPWAT"
#define RFIPOIL_KW              "RFIPOIL"


#define INTEHEAD_NWELLS_INDEX  16     // Number of wells
#define INTEHEAD_NIWELZ_INDEX  24     // Number of elements pr. well in the IWEL array.
#define INTEHEAD_NZWELZ_INDEX  27     // Number of 8 character words pr. well

#define INTEHEAD_NCWMAX_INDEX  17     // Maximum number of completions per well
#define INTEHEAD_NWGMAX_INDEX  19     // Maximum number of wells in any group
#define INTEHEAD_NGMAXZ_INDEX  20     // Maximum number of groups in field
#define INTEHEAD_NICONZ_INDEX  32     // Number of elements pr completion in the ICON array.
#define INTEHEAD_NSCONZ_INDEX  33     // Number of elements pr completion in the SCON array.
#define INTEHEAD_NIGRPZ_INDEX  36     // Number of elements pr group in the IGRP array.

#define INTEHEAD_NSWLMX_INDEX  175
#define INTEHEAD_NSEGMX_INDEX  176
#define INTEHEAD_NLBRMX_INDEX  177
#define INTEHEAD_NISEGZ_INDEX  178
#define INTEHEAD_NRSEGZ_INDEX  179
#define INTEHEAD_NILBRZ_INDEX  180

#define DOUBHEAD_DAYS_INDEX 0

/*****************************************************************/
/*                     Summary files                             */
/*****************************************************************/

/* Summary header file */
#define MINISTEP_KW  "MINISTEP"
#define STARTDAT_KW  "STARTDAT"   /* Intgere keyword containing day,month,year. */
#define WGNAMES_KW   "WGNAMES"    /* The names of wells/groups for the summary vectors. */
#define KEYWORDS_KW  "KEYWORDS"   /* The variable type for the various summary vectors. */
#define UNITS_KW     "UNITS"      /* The units, i.e SM^3/DAY the summary vectors. */
#define DIMENS_KW    "DIMENS"     /* The dimensions of the grid - also used in the GRID files. */
#define NUMS_KW      "NUMS"       /* Extra numeric qualifiers for the summary variables, like cell number. */
#define LGRS_KW      "LGRS"       /* The lgr name for a vector originating from an lgr. */
#define NUMLX_KW     "NUMLX"      /* For block variables defined in a an lgr this is i coordinate in the lgr. */
#define NUMLY_KW     "NUMLY"      /* ... j coordinate in the lgr. */
#define NUMLZ_KW     "NUMLZ"      /* ... k coordinate in the lgr. */


/* Magic indices used to locate day,month,year from the STARTDAT keyword. */
#define STARTDAT_DAY_INDEX   0
#define STARTDAT_MONTH_INDEX 1
#define STARTDAT_YEAR_INDEX  2
#define STARTDAT_SIZE        3


/* Magic indices uset to locate the grid dimensions from the DIMENS
   keyword in the SMSPEC files. Observe that these magic indices
   differ from the magic indices used to look up grid dimensions from
   the DIMENS keyword in GRID files.  */
#define DIMENS_SMSPEC_SIZE_INDEX      0
#define DIMENS_SMSPEC_NX_INDEX 1
#define DIMENS_SMSPEC_NY_INDEX 2
#define DIMENS_SMSPEC_NZ_INDEX 3

#define DIMENS_SIZE            6   // Do not know what the two last items are?


/* Summary data files: */
#define SEQHDR_KW    "SEQHDR"      // Contains a single 'magic' integer - not used in libecl.
#define PARAMS_KW    "PARAMS"      // Contains the actual summary data for one timestep.
#define MINISTEP_KW  "MINISTEP"    // Scalar integer - contains the timestep number.

#define SEQHDR_SIZE  1

#define RESTART_KW    "RESTART"
#define SUMMARY_RESTART_SIZE 8

/*
   There are no magic indices in the summary data files, for all
   interesting data the table created from the ecl_smspec file must be
   consulted.
*/


/*****************************************************************/
/*                        RFT Files                              */
/*****************************************************************/
/* The files with extension .RFT can contain three quite different
   types of information: RFT / PLT / SEGMENT, this is indicated by an
   element of the WELLETC keyword. The keywords below are organized in
   common keywords, keywords for RFTs and keywords for PLTs. The
   special information for SEGMENT data is not internalized at all,
   and there are also several additional keywords for the PLTs which
   are not internalized. */


/* Common keywords */
#define TIME_KW      "TIME"     /* The days since simulation start when
                                   an RFT is performed, also used as
                                   block header when splitting an RFT
                                   file into different wells and timesteps. */
#define DATE_KW      "DATE"     /* The date of an RFT as integers: (day,month,year). */
#define WELLETC_KW   "WELLETC"  /* The type of date RFT|PLT|SEGMENT and well name are
                                   extracted from this keyword. */
#define CONIPOS_KW   "CONIPOS"  /* The i-index of the connections in the well. */
#define CONJPOS_KW   "CONJPOS"  /* The j-index ... */
#define CONKPOS_KW   "CONKPOS"  /* The k-index ... */
#define HOSTGRID_KW  "HOSTGRID"

/* RFT keywords */
#define SWAT_KW      "SWAT"     /* The kewyord containing SWAT. */
#define SGAS_KW      "SGAS"     /* The kewyord containing SGAS. */
#define PRESSURE_KW  "PRESSURE" /* The kewyord containing PRESSURE. */
#define DEPTH_KW     "DEPTH"    /* The depth of the connection. */

/* PLT keywords */
#define CONDEPTH_KW  "CONDEPTH" /* The depth of the connection. */
#define CONWRAT_KW   "CONWRAT"  /* Water rate in a connection. */
#define CONGRAT_KW   "CONGRAT"  /* Gas ... */
#define CONORAT_KW   "CONORAT"  /* Oil ... */
#define CONPRES_KW   "CONPRES"  /* Pressure ... */
#define CONLENST_KW  "CONLENST" /* Length along MSW well */
#define CONLENEN_KW  "CONLENEN" /* Length to connection end for MSW well */
#define CONVTUB_KW   "CONVTUB"  /* Volumetric flow at tubing head conditions. */
#define CONOTUB_KW   "CONOTUB"  /* Volumetric oil flow at tubing head conditions. */
#define CONGTUB_KW   "CONGTUB"  /* Volumetric gas flow at tubing head conditions. */
#define CONWTUB_KW   "CONWTUB"  /* Volumetric water flow at tubing head conditions. */


#define WELLETC_TYPE_INDEX  5 /* At this keyword the WELLETC keyword contains a string
                                 containing 'R', 'P' , or 'S' for RFT, PLT or SEGMENT data
                                 respectively.*/
#define WELLETC_NAME_INDEX  1 /* The name of well being investigated is on this index of
                                 the WELLETC keyword. */

/* Magic indices used to get day,month,year from the DATE
   keyword. */
#define DATE_DAY_INDEX   0
#define DATE_MONTH_INDEX 1
#define DATE_YEAR_INDEX  2


/*****************************************************************/
/*                     GRID and EGRID files.                     */
/*****************************************************************/
/* GRID and EGRID files have very different structure, and only a few
   keywords are shared. */


/* Common keywords */
#define SPECGRID_KW    "SPECGRID"
#define SPECGRID_NX_INDEX  0
#define SPECGRID_NY_INDEX  1
#define SPECGRID_NZ_INDEX  2
#define MAPAXES_KW     "MAPAXES"    /* Keyword used to transform from grid coordinates to
                                       world coordinates. */
#define LGR_KW         "LGR"        /* Name of LGR; for GRID files it can contain two elements,
                                       the second element will be the name of the parent. */
#define MAPUNITS_KW    "MAPUNITS"
#define GRIDUNIT_KW    "GRIDUNIT"

#define NNC1_KW        "NNC1"      /*Upstream cell numbers for non-neighbour connections*/
#define NNC2_KW        "NNC2"      /*Downstream cell numbers for non-neighbour connections*/
#define NNCL_KW        "NNCL"      /*Cell numbers for LGR cells that are connected to global grid cells*/
#define NNCG_KW        "NNCG"      /*Cell numbers for global cells connected to LGR cells*/

#define NNCHEAD_KW     "NNCHEAD"   /*Non-neighbour connection header*/
#define NNCHEAD_SIZE        10
#define NNCHEAD_NUMNNC_INDEX  0    /*Item 1 in non-neighbour connection header: number of NNCs. Only present for main grid*/
#define NNCHEAD_LGR_INDEX     1    /*Item 2 in non-neighbour connection header: LGR number (0 for global grid)*/

#define NNCHEADA_KW    "NNCHEADA"  /*Header for NNC's between two amalgamated LGRs*/
#define NNA1_KW        "NNA1"      /*Cell numbers in connecting local grid ILOC1*/
#define NNA2_KW        "NNA2"      /*Cell numbers in connecting local grid ILOC2*/
#define NNCHEADA_ILOC1_INDEX 0     /*ILOC1: Index of first LGR*/
#define NNCHEADA_ILOC2_INDEX 1     /*ILOC2: Index of second LGR*/
#define NNA_NUMNNC_INDEX     0     /*Item 1 in NNA1 or NNA2 is number of NNCs*/

#define TRANNNC_KW     "TRANNNC"
#define TRANGL_KW      "TRANGL"
#define TRANLL_KW      "TRANLL"

/* EGRID keywords */
#define LGR_PARENT_KW  "LGRPARNT"   /* The name of the parent for an LGR. */
#define COORDS_KW      "COORDS"     /* The (x,y) coordinates of the top and bottom of the pillars constituting the grid. */
#define ZCORN_KW       "ZCORN"      /* Z coordinate where pillars cross planes. */
#define ACTNUM_KW      "ACTNUM"     /* Integer flag of with active=0,1. */
#define HOSTNUM_KW     "HOSTNUM"    /* For cells in LGR - pointing back to cell nr in
                                       parent grid. */
#define FILEHEAD_KW    "FILEHEAD"
#define ENDGRID_KW     "ENDGRID"
#define ENDLGR_KW      "ENDLGR"
#define CORSNUM_KW     "CORSNUM"

/* GRID keywords */
#define GRIDHEAD_KW    "GRIDHEAD"   /* Header information for GRID files. */
#define COORD_KW       "COORD"      /* Header information for one cell in GRID file. */
#define CORNERS_KW     "CORNERS"    /* Vector containing (x,y,z) x 8 elements - all corners in a cell. */
#define DIMENS_KW      "DIMENS"     /* The dimensions of the grid. */
#define RADIAL_KW      "RADIAL"

#define GLOBAL_STRING  "GLOBAL"

#define GRIDHEAD_TYPE_INDEX   0
#define GRIDHEAD_NX_INDEX     1
#define GRIDHEAD_NY_INDEX     2
#define GRIDHEAD_NZ_INDEX     3
#define GRIDHEAD_LGR_INDEX    4
#define GRIDHEAD_SIZE       100

/* Observe that these indices are one value lower than the values used
   in the ecl_smspec file. */
#define DIMENS_NX_INDEX     0
#define DIMENS_NY_INDEX     1
#define DIMENS_NZ_INDEX     2

#define FILEHEAD_VERSION_INDEX   0
#define FILEHEAD_YEAR_INDEX      1
#define FILEHEAD_COMPAT_INDEX    3
#define FILEHEAD_TYPE_INDEX      4
#define FILEHEAD_DUALP_INDEX     5
#define FILEHEAD_ORGFORMAT_INDEX 6

#define GRIDHEAD_GRIDTYPE_CORNERPOINT 1 /*  <----\                        */
                                        /*       |  Fucking hysterical!   */
#define FILEHEAD_GRIDTYPE_CORNERPOINT 0 /*  <----/                        */

#define FILEHEAD_ORGTYPE_CORNERPOINT  1
#define FILEHEAD_SINGLE_POROSITY      0
#define FILEHEAD_DUAL_POROSITY        1
#define FILEHEAD_DUAL_PERMEABILITY    2


#define CELL_NOT_ACTIVE          0
#define CELL_ACTIVE_MATRIX       1
#define CELL_ACTIVE              CELL_ACTIVE_MATRIX
#define CELL_ACTIVE_FRACTURE     2



#ifdef __cplusplus
}
#endif
#endif
