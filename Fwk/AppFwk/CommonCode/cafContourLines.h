/*
Copyright (c) 1996-1997 Nicholas Yue

This software is copyrighted by Nicholas Yue. This code is base on the work of
Paul D. Bourke CONREC.F routine

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#pragma once

#include "cvfBase.h"
#include "cvfVector2.h"
#include <vector>

namespace caf
{
class ContourLines
{
public:
    static void create(const std::vector<double>& dataXY,
                       const std::vector<double>& xPositions,
                       const std::vector<double>& yPositions,
                       const std::vector<double>& contourLevels,
                       std::vector<std::vector<cvf::Vec2d>>* polygons);
private:
    static double saneValue(int index, const std::vector<double>& dataXY, const std::vector<double>& contourLevels);
    static double xsect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh);
    static double ysect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh);
    static int    gridIndex1d(int i, int j, int nx);
private:
    static const int s_castab[3][3][3];
};
}