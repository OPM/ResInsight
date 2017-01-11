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
//  File:   VTOTriangleIntersect.h
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

#ifndef __VTOTRIANGLEINTERSECT_H__
#define __VTOTRIANGLEINTERSECT_H__

#include "VTOBase.h"
#include "VTOLinAlgebra.h"


VTbool	VTTriangleTriangleIntersect(const VTVector* pNodesA, const VTint* piConnA, const VTVector* pNodesB, const VTint* piConnB);
VTbool	VTTriangleTriangleIntersect(const VTVector& a1, const VTVector& a2, const VTVector& a3, const VTVector& b1, const VTVector& b2, const VTVector& b3);
VTbool	VTTriangleTriangleIntersectLine(const VTVector& a1, const VTVector& a2, const VTVector& a3, const VTVector& b1, const VTVector& b2, const VTVector& b3, VTVector* pStart, VTVector* pEnd);
VTbool	VTTriangleBoxIntersect(const VTVector& center, const VTVector& extent, const VTVector& t1, const VTVector& t2, const VTVector& t3);


#endif // __VTOTRIANGLEINTERSECT_H__
