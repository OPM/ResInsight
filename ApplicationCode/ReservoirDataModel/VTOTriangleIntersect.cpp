//################################################################################################
// 
//   ###               ##
//  ## ##  ###   ###  #### ## ##  ###  ####
//  ##    ## ## ## ##  ##  ##### ## ## ## ##
//  ##    ##### #####  ##  ###   ## ## ## ##
//  ## ## ##    ##     ##  ##    ## ## ## ##
//   ###   ###   ###    ## ##     ###  ## ## -- understanding by visualization
//
// 
//  Lib/App: GLview API
// 	Module:	Base Module
//  -------------------
//
//  File:   VTOTriangleIntersect.cpp
//  By:     Paal Chr. Hagen
//	Date:   17-aug-2005
//	Status:	Public
//
//  Description: 
//  Triangle intersection test functions
//
//  --------------------------------------------------------------------------------------------
//  Copyright (C) 2005, Ceetron ASA
//
//  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Ceetron ASA. The contents of this file may 
//  not be disclosed to third parties, copied or duplicated in any form, in whole or in part,
//  without the prior written permission of Ceetron ASA.
//
//################################################################################################

#include "VTOTriangleIntersect.h"

#include <math.h>

//=================================================================================================================================
//
// VTTriangleTriangleIntersect - START
//
// Based on work by P. Guigue - O. Devillers and Tomas Akenie-Moller
// URL: http://www-sop.inria.fr/prisme/personnel/guigue/triangle_triangle_intersection.c
//
// Documentation:
// -----------------
// 
//  Triangle-Triangle Overlap Test Routines				
//  July, 2002                                                          
//  Updated December 2003                                                
//                                                                       
//  This file contains C implementation of algorithms for                
//  performing two and three-dimensional triangle-triangle intersection test 
//  The algorithms and underlying theory are described in                    
//                                                                           
// "Fast and Robust Triangle-Triangle Overlap Test 
//  Using Orientation Predicates"  P. Guigue - O. Devillers
//                                                 
//  Journal of Graphics Tools, 8(1), 2003                                    
//                                                                           
//  Several geometric predicates are defined.  Their parameters are all      
//  points.  Each point is an array of two or three float precision         
//  floating point numbers. The geometric predicates implemented in          
//  this file are:                                                            
//                                                                           
//    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)                         
//    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)                         
//                                                                           
//    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,
//                                     coplanar,source,target)               
//                                                                           
//       is a version that computes the segment of intersection when            
//       the triangles overlap (and are not coplanar)                        
//                                                                           
//    each function returns 1 if the triangles (including their              
//    boundary) intersect, otherwise 0                                       
//                                                                           
//                                                                           
//  Other information are available from the Web page                        
//  http://www.acm.org/jgt/papers/GuigueDevillers03/                         

//=================================================================================================================================

int VT_tri_tri_overlap_test_3d(const VTfloat* p1, const VTfloat* q1, const VTfloat* r1, const VTfloat* p2, const VTfloat* q2, const VTfloat* r2);
int VT_tri_tri_intersection_test_3d(const VTfloat*  p1, const VTfloat* q1, const VTfloat* r1, const VTfloat* p2, const VTfloat* q2, const VTfloat* r2, int * coplanar, VTfloat* source, VTfloat* target);


/*************************************************************************************************
 *//** 
 * \brief	Triangle vs. triangle intersection testing
 *
 * \param	a1  First node of first triangle. Global coordinates.
 * \param	a2  Second node of first triangle. Global coordinates.
 * \param	a3  Third node of first triangle. Global coordinates.
 * \param	b1  First node of second triangle. Global coordinates.
 * \param	b2  Second node of second triangle. Global coordinates.
 * \param	b3  Third node of second triangle. Global coordinates.
 *
 * \return	VT_TRUE if the triangles intersect. VT_FALSE if not.
 *
 * \assumpt	Coordinates in same coordinate system.
 * \comment	Based on work by P. Guigue - O. Devillers
 * \author		FV
 *************************************************************************************************/
VTbool VTTriangleTriangleIntersect(const VTVector& a1, const VTVector& a2, const VTVector& a3, const VTVector& b1, const VTVector& b2, const VTVector& b3)
{
	return (VT_tri_tri_overlap_test_3d(~a1, ~a2, ~a3, ~b1, ~b2, ~b3) == 1);
}



/*************************************************************************************************
 *//** 
 * \brief	Triangle vs. triangle intersection testing using indexes and node arrays.
 *
 * \param	pNodesA  Array with nodes in which the first triangle have its nodes.
 * \param	piConnA  Indices to the tree nodes of the first triangle. Size: >= 3 integers. 
 * \param	pNodesB  Array with nodes in which the second triangle have its nodes.
 * \param	piConnB  Indices to the tree nodes of the second triangle. Size: >= 3 integers. 
 *
 * \return	VT_TRUE if the triangles intersect. VT_FALSE if not.
 *
 * \assumpt	Coordinates in same coordinate system.
 * \comment	Based on work by P. Guigue - O. Devillers
 * \author		FV
 *************************************************************************************************/
VTbool VTTriangleTriangleIntersect(const VTVector* pNodesA, const VTint* piConnA, const VTVector* pNodesB, const VTint* piConnB)
{
	return (VT_tri_tri_overlap_test_3d(~pNodesA[piConnA[0]], ~pNodesA[piConnA[1]], ~pNodesA[piConnA[2]], ~pNodesB[piConnB[0]], ~pNodesB[piConnB[1]], ~pNodesB[piConnB[2]]) == 1);
}


/*************************************************************************************************
 *//** 
 * \brief	Triangle vs. triangle intersection testing, returning the intersection line if possible.
 *
 * \param	a1  First node of first triangle. Global coordinates.
 * \param	a2  Second node of first triangle. Global coordinates.
 * \param	a3  Third node of first triangle. Global coordinates.
 * \param	b1  First node of second triangle. Global coordinates.
 * \param	b2  Second node of second triangle. Global coordinates.
 * \param	b3  Third node of second triangle. Global coordinates.
 * \param	pStart  Out: First node of the line forming the intersection between the two triangles.
 * \param	pEnd  Out: Second node of the line forming the intersection between the two triangles.
 *
 * \return	VT_TRUE if the triangles intersect. VT_FALSE if not.
 *
 * \assumpt	Coordinates in same coordinate system.
 * \comment	Use this function to check for intersection between two polygons and to compute the 
 *			intersection line if there is an intersection.
 *			Based on work by P. Guigue - O. Devillers
 * \author		FV
 *************************************************************************************************/
VTbool VTTriangleTriangleIntersectLine(const VTVector& a1, const VTVector& a2, const VTVector& a3, const VTVector& b1, const VTVector& b2, const VTVector& b3, VTVector* pStart, VTVector* pEnd)
{
	VT_ASSERT(pStart && pEnd);

	VTint iCoplanar = 0;

	VTint iRes = VT_tri_tri_intersection_test_3d(~a1, ~a2, ~a3, ~b1, ~b2, ~b3, &iCoplanar, pStart->GetPtr(), pEnd->GetPtr());

	return (iRes && !iCoplanar);
}

// function prototypes
int VT_coplanar_tri_tri3d(const VTfloat* p1, const VTfloat* q1, const VTfloat* r1, const VTfloat* p2, const VTfloat* q2, const VTfloat* r2,
		       VTfloat  N1[3]);

int VT_tri_tri_overlap_test_2d(VTfloat p1[2], VTfloat q1[2], VTfloat r1[2], 
			    VTfloat p2[2], VTfloat q2[2], VTfloat r2[2]);


// some 3D macros
#define CROSS(dest,v1,v2)                       \
               dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
               dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
               dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
 


#define SUB(dest,v1,v2) dest[0]=v1[0]-v2[0]; \
                        dest[1]=v1[1]-v2[1]; \
                        dest[2]=v1[2]-v2[2]; 


#define SCALAR(dest,alpha,v) dest[0] = alpha * v[0]; \
                             dest[1] = alpha * v[1]; \
                             dest[2] = alpha * v[2];



#define CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) {\
  SUB(v1,p2,q1)\
  SUB(v2,p1,q1)\
  CROSS(N1,v1,v2)\
  SUB(v1,q2,q1)\
  if (DOT(v1,N1) > 0.0f) return 0;\
  SUB(v1,p2,p1)\
  SUB(v2,r1,p1)\
  CROSS(N1,v1,v2)\
  SUB(v1,r2,p1) \
  if (DOT(v1,N1) > 0.0f) return 0;\
  else return 1; }



/* Permutation in a canonical form of T2's vertices */
#define TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
  if (dp2 > 0.0f) { \
     if (dq2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2) \
     else if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
     else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
    if (dq2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
    else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
    else CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
  } else { \
    if (dq2 < 0.0f) { \
      if (dr2 >= 0.0f)  CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
      else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2)\
    } \
    else if (dq2 > 0.0f) { \
      if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
      else  CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
    } \
    else  { \
      if (dr2 > 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
      else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2)\
      else return VT_coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1);\
     }}}
  


/*
*
*  Three-dimensional Triangle-Triangle Overlap Test
*
*/
int VT_tri_tri_overlap_test_3d(const VTfloat* p1, const VTfloat* q1, const VTfloat* r1, const VTfloat* p2, const VTfloat* q2, const VTfloat* r2)
{
  VTfloat dp1, dq1, dr1, dp2, dq2, dr2;
  VTfloat v1[3], v2[3];
  VTfloat N1[3], N2[3]; 
  
  /* Compute distance signs  of p1, q1 and r1 to the plane of
     triangle(p2,q2,r2) */


  SUB(v1,p2,r2)
  SUB(v2,q2,r2)
  CROSS(N2,v1,v2)

  SUB(v1,p1,r2)
  dp1 = DOT(v1,N2);
  SUB(v1,q1,r2)
  dq1 = DOT(v1,N2);
  SUB(v1,r1,r2)
  dr1 = DOT(v1,N2);
  
  if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 

  /* Compute distance signs  of p2, q2 and r2 to the plane of
     triangle(p1,q1,r1) */

  
  SUB(v1,q1,p1)
  SUB(v2,r1,p1)
  CROSS(N1,v1,v2)

  SUB(v1,p2,r1)
  dp2 = DOT(v1,N1);
  SUB(v1,q2,r1)
  dq2 = DOT(v1,N1);
  SUB(v1,r2,r1)
  dr2 = DOT(v1,N1);
  
  if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

  /* Permutation in a canonical form of T1's vertices */


  if (dp1 > 0.0f) {
    if (dq1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
    else if (dr1 > 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)	
    else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
  } else if (dp1 < 0.0f) {
    if (dq1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
    else if (dr1 < 0.0f) TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    else TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
  } else {
    if (dq1 < 0.0f) {
      if (dr1 >= 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
    }
    else if (dq1 > 0.0f) {
      if (dr1 > 0.0f) TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    }
    else  {
      if (dr1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
      else if (dr1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
      else return VT_coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1);
    }
  }
};


int VT_coplanar_tri_tri3d(const VTfloat* p1, const VTfloat* q1, const VTfloat* r1, const VTfloat* p2, const VTfloat* q2, const VTfloat* r2,
		       VTfloat normal_1[3])
{
  VTfloat P1[2],Q1[2],R1[2];
  VTfloat P2[2],Q2[2],R2[2];

  VTfloat n_x, n_y, n_z;

  n_x = ((normal_1[0]<0)?-normal_1[0]:normal_1[0]);
  n_y = ((normal_1[1]<0)?-normal_1[1]:normal_1[1]);
  n_z = ((normal_1[2]<0)?-normal_1[2]:normal_1[2]);


  /* Projection of the triangles in 3D onto 2D such that the area of
     the projection is maximized. */


  if (( n_x > n_z ) && ( n_x >= n_y )) {
    // Project onto plane YZ

      P1[0] = q1[2]; P1[1] = q1[1];
      Q1[0] = p1[2]; Q1[1] = p1[1];
      R1[0] = r1[2]; R1[1] = r1[1]; 
    
      P2[0] = q2[2]; P2[1] = q2[1];
      Q2[0] = p2[2]; Q2[1] = p2[1];
      R2[0] = r2[2]; R2[1] = r2[1]; 

  } else if (( n_y > n_z ) && ( n_y >= n_x )) {
    // Project onto plane XZ

    P1[0] = q1[0]; P1[1] = q1[2];
    Q1[0] = p1[0]; Q1[1] = p1[2];
    R1[0] = r1[0]; R1[1] = r1[2]; 
 
    P2[0] = q2[0]; P2[1] = q2[2];
    Q2[0] = p2[0]; Q2[1] = p2[2];
    R2[0] = r2[0]; R2[1] = r2[2]; 
    
  } else {
    // Project onto plane XY

    P1[0] = p1[0]; P1[1] = p1[1]; 
    Q1[0] = q1[0]; Q1[1] = q1[1]; 
    R1[0] = r1[0]; R1[1] = r1[1]; 
    
    P2[0] = p2[0]; P2[1] = p2[1]; 
    Q2[0] = q2[0]; Q2[1] = q2[1]; 
    R2[0] = r2[0]; R2[1] = r2[1]; 
  }

  return VT_tri_tri_overlap_test_2d(P1,Q1,R1,P2,Q2,R2);
    
};



/*
*                                                                
*  Three-dimensional Triangle-Triangle Intersection              
*
*/

/*
   This macro is called when the triangles surely intersect
   It constructs the segment of intersection of the two triangles
   if they are not coplanar.
*/

#define CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) { \
  SUB(v1,q1,p1) \
  SUB(v2,r2,p1) \
  CROSS(N,v1,v2) \
  SUB(v,p2,p1) \
  if (DOT(v,N) > 0.0f) {\
    SUB(v1,r1,p1) \
    CROSS(N,v1,v2) \
    if (DOT(v,N) <= 0.0f) { \
      SUB(v2,q2,p1) \
      CROSS(N,v1,v2) \
      if (DOT(v,N) > 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
      } else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
      } \
    } else { \
      return 0; \
    } \
  } else { \
    SUB(v2,q2,p1) \
    CROSS(N,v1,v2) \
    if (DOT(v,N) < 0.0f) { \
      return 0; \
    } else { \
      SUB(v1,r1,p1) \
      CROSS(N,v1,v2) \
      if (DOT(v,N) >= 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
      } else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
      }}}} 

								

#define TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
  if (dp2 > 0.0f) { \
     if (dq2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2) \
     else if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
     else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
    if (dq2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
    else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
    else CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
  } else { \
    if (dq2 < 0.0f) { \
      if (dr2 >= 0.0f)  CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
      else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2)\
    } \
    else if (dq2 > 0.0f) { \
      if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
      else  CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
    } \
    else  { \
      if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
      else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2)\
      else { \
       	*coplanar = 1; \
	return VT_coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1);\
     } \
  }} }
  

/*
   The following version computes the segment of intersection of the
   two triangles if it exists. 
   coplanar returns whether the triangles are coplanar
   source and target are the endpoints of the line segment of intersection 
*/

int VT_tri_tri_intersection_test_3d(const VTfloat*  p1, const VTfloat* q1, const VTfloat* r1, 
				 const VTfloat* p2, const VTfloat* q2, const VTfloat* r2,
				 int * coplanar, 
				 VTfloat* source, VTfloat* target)
				 
{
  VTfloat dp1, dq1, dr1, dp2, dq2, dr2;
  VTfloat v1[3], v2[3], v[3];
  VTfloat N1[3], N2[3], N[3];
  VTfloat alpha;

  // Compute distance signs  of p1, q1 and r1 
  // to the plane of triangle(p2,q2,r2)


  SUB(v1,p2,r2)
  SUB(v2,q2,r2)
  CROSS(N2,v1,v2)

  SUB(v1,p1,r2)
  dp1 = DOT(v1,N2);
  SUB(v1,q1,r2)
  dq1 = DOT(v1,N2);
  SUB(v1,r1,r2)
  dr1 = DOT(v1,N2);
  
  if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 

  // Compute distance signs  of p2, q2 and r2 
  // to the plane of triangle(p1,q1,r1)

  
  SUB(v1,q1,p1)
  SUB(v2,r1,p1)
  CROSS(N1,v1,v2)

  SUB(v1,p2,r1)
  dp2 = DOT(v1,N1);
  SUB(v1,q2,r1)
  dq2 = DOT(v1,N1);
  SUB(v1,r2,r1)
  dr2 = DOT(v1,N1);
  
  if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

  // Permutation in a canonical form of T1's vertices


  if (dp1 > 0.0f) {
    if (dq1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
    else if (dr1 > 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
	
    else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
  } else if (dp1 < 0.0f) {
    if (dq1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
    else if (dr1 < 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    else TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
  } else {
    if (dq1 < 0.0f) {
      if (dr1 >= 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
    }
    else if (dq1 > 0.0f) {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    }
    else  {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
      else if (dr1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
      else {
	// triangles are co-planar

	*coplanar = 1;
	return VT_coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1);
      }
    }
  }
};


/*
*
*  Two dimensional Triangle-Triangle Overlap Test    
*
*/

/* some 2D macros */

#define ORIENT_2D(a, b, c)  ((a[0]-c[0])*(b[1]-c[1])-(a[1]-c[1])*(b[0]-c[0]))


#define INTERSECTION_TEST_VERTEX(P1, Q1, R1, P2, Q2, R2) {\
  if (ORIENT_2D(R2,P2,Q1) >= 0.0f)\
    if (ORIENT_2D(R2,Q2,Q1) <= 0.0f)\
      if (ORIENT_2D(P1,P2,Q1) > 0.0f) {\
	if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) return 1; \
	else return 0;} else {\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f)\
	  if (ORIENT_2D(Q1,R1,P2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;}\
    else \
      if (ORIENT_2D(P1,Q2,Q1) <= 0.0f)\
	if (ORIENT_2D(R2,Q2,R1) <= 0.0f)\
	  if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;\
      else return 0;\
  else\
    if (ORIENT_2D(R2,P2,R1) >= 0.0f) \
      if (ORIENT_2D(Q1,R1,R2) >= 0.0f)\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f) return 1;\
	else return 0;\
      else \
	if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) {\
	  if (ORIENT_2D(R2,R1,Q2) >= 0.0f) return 1; \
	  else return 0; }\
	else return 0; \
    else  return 0; \
 };



#define INTERSECTION_TEST_EDGE(P1, Q1, R1, P2, Q2, R2) { \
  if (ORIENT_2D(R2,P2,Q1) >= 0.0f) {\
    if (ORIENT_2D(P1,P2,Q1) >= 0.0f) { \
        if (ORIENT_2D(P1,Q1,R2) >= 0.0f) return 1; \
        else return 0;} else { \
      if (ORIENT_2D(Q1,R1,P2) >= 0.0f){ \
	if (ORIENT_2D(R1,P1,P2) >= 0.0f) return 1; else return 0;} \
      else return 0; } \
  } else {\
    if (ORIENT_2D(R2,P2,R1) >= 0.0f) {\
      if (ORIENT_2D(P1,P2,R1) >= 0.0f) {\
	if (ORIENT_2D(P1,R1,R2) >= 0.0f) return 1;  \
	else {\
	  if (ORIENT_2D(Q1,R1,R2) >= 0.0f) return 1; else return 0;}}\
      else  return 0; }\
    else return 0; }}



int VT_ccw_tri_tri_intersection_2d(VTfloat p1[2], VTfloat q1[2], VTfloat r1[2], 
				VTfloat p2[2], VTfloat q2[2], VTfloat r2[2]) {
  if ( ORIENT_2D(p2,q2,p1) >= 0.0f ) {
    if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) return 1;
      else INTERSECTION_TEST_EDGE(p1,q1,r1,p2,q2,r2)
    } else {  
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) 
	INTERSECTION_TEST_EDGE(p1,q1,r1,r2,p2,q2)
      else INTERSECTION_TEST_VERTEX(p1,q1,r1,p2,q2,r2)}}
  else {
    if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) 
	INTERSECTION_TEST_EDGE(p1,q1,r1,q2,r2,p2)
      else  INTERSECTION_TEST_VERTEX(p1,q1,r1,q2,r2,p2)}
    else INTERSECTION_TEST_VERTEX(p1,q1,r1,r2,p2,q2)}
};


int VT_tri_tri_overlap_test_2d(VTfloat p1[2], VTfloat q1[2], VTfloat r1[2], 
			    VTfloat p2[2], VTfloat q2[2], VTfloat r2[2]) {
  if ( ORIENT_2D(p1,q1,r1) < 0.0f )
    if ( ORIENT_2D(p2,q2,r2) < 0.0f )
      return VT_ccw_tri_tri_intersection_2d(p1,r1,q1,p2,r2,q2);
    else
      return VT_ccw_tri_tri_intersection_2d(p1,r1,q1,p2,q2,r2);
  else
    if ( ORIENT_2D(p2,q2,r2) < 0.0f )
      return VT_ccw_tri_tri_intersection_2d(p1,q1,r1,p2,r2,q2);
    else
      return VT_ccw_tri_tri_intersection_2d(p1,q1,r1,p2,q2,r2);

};

// Undef macros used in the VTTriangleTriangleIntersect code
#undef CROSS
#undef DOT
#undef SUB
#undef SCALAR
#undef CHECK_MIN_MAX
#undef TRI_TRI_3D
#undef CONSTRUCT_INTERSECTION
#undef TRI_TRI_INTER_3D
#undef ORIENT_2D
#undef INTERSECTION_TEST_VERTEX
#undef INTERSECTION_TEST_EDGE

//=================================================================================================================================
//
// VTTriangleTriangleIntersect - END
//
//=================================================================================================================================



//=================================================================================================================================
//
// VTTriangleBoxIntersect - START
//
// Based on work by Tomas Akenie-Moller
// URL: http://www.ce.chalmers.se/staff/tomasm/code/tribox2.txt
//
// Modifications:
// 1) Added VT_ prefix to functions
// 2) Changed from VTfloat[3] to [const] VTfloat* params
// 3) Type casts for fabs() to avoid warning
//
// Doc from him:
// AABB-triangle overlap test code                      
// by Tomas Akenine-Möller                              
// Function: int triBoxOverlap(VTfloat boxcenter[3],      
//          VTfloat boxhalfsize[3],VTfloat triverts[3][3]); 
// History:                                             
//   2001-03-05: released the code in its first version 
//   2001-06-18: changed the order of the tests, faster 
//                                                      
// Acknowledgement: Many thanks to Pierre Terdiman for  
// suggestions and discussions on how to optimize code. 
// Thanks to David Hunt for finding a ">="-bug!         
//
//=================================================================================================================================

int VT_triBoxOverlap(const VTfloat* boxcenter, const VTfloat* boxhalfsize, const VTfloat* triverts1, const VTfloat* triverts2, const VTfloat* triverts3);

/*************************************************************************************************
 *//** 
 * \brief	Intersection test between a Axis Aligned Bounding Box (AABB) and a triangle
 *
 * \param	center  The center of the AABB. Global coords
 * \param	extent  The (half) extent of the AABB (from center-extent to center+extent). Global coords
 * \param	t1  First coordinate of triangle. Global coordinate
 * \param	t2  Second coordinate of triangle. Global coordinate
 * \param	t3  Third coordinate of triangle. Global coordinate
 *
 * \return	VT_TRUE if intesection
 *
 * \comment	Based on an algorithm by Tomas Akenie-Moller.
 * \author		FV
 *************************************************************************************************/
VTbool VTTriangleBoxIntersect(const VTVector& center, const VTVector& extent, const VTVector& t1, const VTVector& t2, const VTVector& t3)
{
	return (VT_triBoxOverlap(~center, ~extent, ~t1, ~t2, ~t3) == 1);
}

#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

int VT_planeBoxOverlap(const VTfloat* normal, VTfloat d, const VTfloat* maxbox)
{
  int q;
  VTfloat vmin[3],vmax[3];
  for(q=X;q<=Z;q++)
  {
    if(normal[q]>0.0f)
    {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else
    {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;

  return 0;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p2 = a*v2[Y] - b*v2[Z];                    \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p1 = a*v1[Y] - b*v1[Z];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p2 = -a*v2[X] + b*v2[Z];                       \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p1 = -a*v1[X] + b*v1[Z];                       \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[X] - b*v1[Y];                    \
    p2 = a*v2[X] - b*v2[Y];                    \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0[X] - b*v0[Y];                \
    p1 = a*v1[X] - b*v1[Y];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

int VT_triBoxOverlap(const VTfloat* boxcenter, const VTfloat* boxhalfsize, const VTfloat* triverts1, const VTfloat* triverts2, const VTfloat* triverts3)
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   VTfloat v0[3],v1[3],v2[3];
   VTfloat min,max,d,p0,p1,p2,rad,fex,fey,fez;
   VTfloat normal[3],e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts1,boxcenter);
   SUB(v1,triverts2,boxcenter);
   SUB(v2,triverts3,boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = (VTfloat)fabs(e0[X]);
   fey = (VTfloat)fabs(e0[Y]);
   fez = (VTfloat)fabs(e0[Z]);
   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = (VTfloat)fabs(e1[X]);
   fey = (VTfloat)fabs(e1[Y]);
   fez = (VTfloat)fabs(e1[Z]);
   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = (VTfloat)fabs(e2[X]);
   fey = (VTfloat)fabs(e2[Y]);
   fez = (VTfloat)fabs(e2[Z]);
   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   CROSS(normal,e0,e1);
   d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
   if(!VT_planeBoxOverlap(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}

// Undef macros used in the VTTriangleBoxIntersect code
#undef X
#undef Y
#undef Z
#undef CROSS
#undef DOT
#undef SUB
#undef FINDMINMAX
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

//=================================================================================================================================
//
// VTTriangleBoxIntersect - END
//
//=================================================================================================================================
