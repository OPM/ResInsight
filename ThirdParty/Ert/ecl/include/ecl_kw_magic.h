#ifndef __ECL_KW_MAGIC_H__
#define __ECL_KW_MAGIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 
   This header file contains names and indices of ECLIPSE keywords
   which have special significance in various files; everything
   related to the INTEHEAD keyword is in the ecl_intehead.h header
   file.
*/


/*****************************************************************/
/*                INIT and RESTART files:                        */
/*****************************************************************/

#define SEQNUM_KW    "SEQNUM"       /* Contains the report step as the only data;
                                       not present in non-unified files, where the
                                       report step can be inferred from the filename. */


#define IWEL_KW      "IWEL"
#define ZWEL_KW      "ZWEL"
#define ICON_KW      "ICON"
#define ISEG_KW      "ISEG"

#define PORV_KW      "PORV"
#define AQUIFER_KW   "AQUIFERN"

/*****************************************************************/
/*                     Summary files                             */
/*****************************************************************/

/* Summary header file */
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


/* Magic indices uset to locate the grid dimensions from the DIMENS
   keyword in the SMSPEC files. Observe that these magic indices
   differ from the magic indices used to look up grid dimensions from
   the DIMENS keyword in GRID files.  */
#define DIMENS_SMSPEC_NX_INDEX 1
#define DIMENS_SMSPEC_NY_INDEX 2
#define DIMENS_SMSPEC_NZ_INDEX 3


/* Summary data files: */
#define SEQHDR_KW    "SEQHDR"      // Contains a single 'magic' integer - not used in libecl.
#define PARAMS_KW    "PARAMS"      // Contains the actual summary data for one timestep.
#define MINISTEP_KW  "MINISTEP"    // Scalar integer - contains the timestep number.

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
#define MAPAXES_KW     "MAPAXES"    /* Keyword used to transform from grid coordinates to
                                       world coordinates. */
#define LGR_KW         "LGR"        /* Name of LGR; for GRID files it can contain two elements, 
                                       the second element will be the name of the parent. */

/* EGRID keywords */
#define LGR_PARENT_KW  "LGRPARNT"   /* The name of the parent for an LGR. */
#define COORDS_KW      "COORDS"     /* The (x,y) coordinates of the top and bottom of the pillars constituting the grid. */
#define ZCORN_KW       "ZCORN"      /* Z coordinate where pillars cross planes. */
#define ACTNUM_KW      "ACTNUM"     /* Integer flag of with active=0,1. */
#define HOSTNUM_KW     "HOSTNUM"    /* For cells in LGR - pointing back to cell nr in
                                       parent grid. */

/* GRID keywords */
#define GRIDHEAD_KW    "GRIDHEAD"   /* Header information for GRID files. */
#define COORD_KW       "COORD"      /* Header information for one cell in GRID file. */
#define CORNERS_KW     "CORNERS"    /* Vector containing (x,y,z) x 8 elements - all corners in a cell. */
#define DIMENS_KW      "DIMENS"     /* The dimensions of the grid. */

#define GLOBAL_STRING  "GLOBAL"

#define GRIDHEAD_TYPE_INDEX   0
#define GRIDHEAD_NX_INDEX     1
#define GRIDHEAD_NY_INDEX     2
#define GRIDHEAD_NZ_INDEX     3

/* Observe that these indices are one value lower the values used
   in the ecl_smspec file. */
#define DIMENS_NX_INDEX     0
#define DIMENS_NY_INDEX     1
#define DIMENS_NZ_INDEX     2

#ifdef __cplusplus
}
#endif
#endif
