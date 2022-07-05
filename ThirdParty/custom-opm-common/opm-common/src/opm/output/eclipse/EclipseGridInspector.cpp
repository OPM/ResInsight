//===========================================================================
//
// File: EclipseGridInspector.C
//
// Created: Mon Jun  2 12:17:51 2008
//
// Author: Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
// Revision: $Id: EclipseGridInspector.C,v 1.2 2008/08/18 14:16:13 atgeirr Exp $
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <opm/output/eclipse/EclipseGridInspector.hpp>
#include <opm/common/ErrorMacros.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <array>
#include <iostream>

namespace Opm
{
EclipseGridInspector::EclipseGridInspector(Opm::Deck deck)
    : deck_( std::move( deck ) )
{
    init_();
}

void EclipseGridInspector::init_()
{
    if (!deck_.hasKeyword("COORD")) {
        OPM_THROW(std::runtime_error, "Needed field \"COORD\" is missing in file");
    }
    if (!deck_.hasKeyword("ZCORN")) {
        OPM_THROW(std::runtime_error, "Needed field \"ZCORN\" is missing in file");
    }

    if (deck_.hasKeyword("SPECGRID")) {
        const auto& specgridRecord =
            deck_["SPECGRID"].back().getRecord(0);
        logical_gridsize_[0] = specgridRecord.getItem("NX").get< int >(0);
        logical_gridsize_[1] = specgridRecord.getItem("NY").get< int >(0);
        logical_gridsize_[2] = specgridRecord.getItem("NZ").get< int >(0);
    } else if (deck_.hasKeyword("DIMENS")) {
        const auto& dimensRecord =
            deck_["DIMENS"].back().getRecord(0);
        logical_gridsize_[0] = dimensRecord.getItem("NX").get< int >(0);
        logical_gridsize_[1] = dimensRecord.getItem("NY").get< int >(0);
        logical_gridsize_[2] = dimensRecord.getItem("NZ").get< int >(0);
    } else {
        OPM_THROW(std::runtime_error, "Found neither SPECGRID nor DIMENS in file. At least one is needed.");
    }

}

/**
   Return the dip slopes for the cell relative to xy-plane in x- and y- direction.
   Dip slope is average rise in positive x-direction over cell length in x-direction.
   Similarly for y.

   Current implementation is for vertical pillars, but is not difficult to fix.

   @returns a std::pair<double,double> with x-dip in first component and y-dip in second.
*/
std::pair<double,double> EclipseGridInspector::cellDips(int i, int j, int k) const
{
    checkLogicalCoords(i, j, k);
    const std::vector<double>& pillc =
        deck_["COORD"].back().getSIDoubleData();
    int num_pillars = (logical_gridsize_[0] + 1)*(logical_gridsize_[1] + 1);
        if (6*num_pillars != int(pillc.size())) {
        throw std::runtime_error("Wrong size of COORD field.");
    }
    const std::vector<double>& z =
        deck_["ZCORN"].back().getSIDoubleData();
    int num_cells = logical_gridsize_[0]*logical_gridsize_[1]*logical_gridsize_[2];
    if (8*num_cells != int(z.size())) {
        throw std::runtime_error("Wrong size of ZCORN field");
    }

    // Pick ZCORN-value for all 8 corners of the given cell
    std::array<double, 8> cellz = cellZvals(i, j, k);

    // Compute rise in positive x-direction for all four edges (and then find mean)
    // Current implementation is for regularly placed and vertical pillars!
    int numxpill = logical_gridsize_[0] + 1;
    int pix = i + j*numxpill;
    double cell_xlength = pillc[6*(pix + 1)] - pillc[6*pix];
    flush(std::cout);
    double xrise[4] = { (cellz[1] - cellz[0])/cell_xlength,  // LLL -> HLL
                        (cellz[3] - cellz[2])/cell_xlength,  // LHL -> HHL
                        (cellz[5] - cellz[4])/cell_xlength,  // LLH -> HLH
                        (cellz[7] - cellz[6])/cell_xlength}; // LHH -> HHH

    double cell_ylength = pillc[6*(pix + numxpill) + 1] - pillc[6*pix + 1];
    double yrise[4] = { (cellz[2] - cellz[0])/cell_ylength,  // LLL -> LHL
                        (cellz[3] - cellz[1])/cell_ylength,  // HLL -> HHL
                        (cellz[6] - cellz[4])/cell_ylength,  // LLH -> LHH
                        (cellz[7] - cellz[5])/cell_ylength}; // HLH -> HHH


    // Now ignore those edges that touch the global top or bottom surface
    // of the entire grdecl model. This is to avoid bias, as these edges probably
    // don't follow an overall dip for the model if it exists.
    int x_edges = 4;
    int y_edges = 4;
    std::array<double, 6> gridlimits = getGridLimits();
    double zmin = gridlimits[4];
    double zmax = gridlimits[5];
    // LLL -> HLL
    if ((cellz[1] == zmin) || (cellz[0] == zmin)) {
	xrise[0] = 0; x_edges--;
    }
    // LHL -> HHL
    if ((cellz[2] == zmin) || (cellz[3] == zmin)) {
	xrise[1] = 0; x_edges--;
    }
    // LLH -> HLH
    if ((cellz[4] == zmax) || (cellz[5] == zmax)) {
	xrise[2] = 0; x_edges--;
    }
    // LHH -> HHH
    if ((cellz[6] == zmax) || (cellz[7] == zmax)) {
	xrise[3] = 0; x_edges--;
    }
    // LLL -> LHL
    if ((cellz[0] == zmin) || (cellz[2] == zmin)) {
	yrise[0] = 0; y_edges--;
    }
    // HLL -> HHL
    if ((cellz[1] == zmin) || (cellz[3] == zmin)) {
	yrise[1] = 0; y_edges--;
    }
    // LLH -> LHH
    if ((cellz[6] == zmax) || (cellz[4] == zmax)) {
	yrise[2] = 0; y_edges--;
    }
    // HLH -> HHH
    if ((cellz[7] == zmax) || (cellz[5] == zmax)) {
	yrise[3] = 0; y_edges--;
    }

    return std::make_pair( (xrise[0] + xrise[1] + xrise[2] + xrise[3])/x_edges,
                          (yrise[0] + yrise[1] + yrise[2] + yrise[3])/y_edges);
}
/**
  Wrapper for cellDips(i, j, k).
*/
std::pair<double,double> EclipseGridInspector::cellDips(int cell_idx) const
{
    std::array<int, 3> idxs = cellIdxToLogicalCoords(cell_idx);
    return cellDips(idxs[0], idxs[1], idxs[2]);
}

std::array<int, 3> EclipseGridInspector::cellIdxToLogicalCoords(int cell_idx) const
{

    int i,j,k; // Position of cell in cell hierarchy
    int horIdx = (cell_idx+1) - int(std::floor(((double)(cell_idx+1))/((double)(logical_gridsize_[0]*logical_gridsize_[1]))))*logical_gridsize_[0]*logical_gridsize_[1]; // index in the corresponding horizon
    if (horIdx == 0) {
        horIdx = logical_gridsize_[0]*logical_gridsize_[1];
    }
    i = horIdx - int(std::floor(((double)horIdx)/((double)logical_gridsize_[0])))*logical_gridsize_[0];
    if (i == 0) {
        i = logical_gridsize_[0];
    }
    j = (horIdx-i)/logical_gridsize_[0]+1;
    k = ((cell_idx+1)-logical_gridsize_[0]*(j-1)-1)/(logical_gridsize_[0]*logical_gridsize_[1])+1;

    std::array<int, 3> a = {{i-1, j-1, k-1}};
    return a; //std::array<int, 3> {{i-1, j-1, k-1}};
}

double EclipseGridInspector::cellVolumeVerticalPillars(int i, int j, int k) const
{
    // Checking parameters and obtaining values from parser.
    checkLogicalCoords(i, j, k);
    const std::vector<double>& pillc =
        deck_["COORD"].back().getSIDoubleData();
    int num_pillars = (logical_gridsize_[0] + 1)*(logical_gridsize_[1] + 1);
    if (6*num_pillars != int(pillc.size())) {
	throw std::runtime_error("Wrong size of COORD field.");
    }
    const std::vector<double>& z =
        deck_["ZCORN"].back().getSIDoubleData();
    int num_cells = logical_gridsize_[0]*logical_gridsize_[1]*logical_gridsize_[2];
    if (8*num_cells != int(z.size())) {
	throw std::runtime_error("Wrong size of ZCORN field");
    }

    // Computing the base area as half the 2d cross product of the diagonals.
    int numxpill = logical_gridsize_[0] + 1;
    int pix = i + j*numxpill;
    double px[4] = { pillc[6*pix],
		     pillc[6*(pix + 1)],
		     pillc[6*(pix + numxpill)],
		     pillc[6*(pix + numxpill + 1)] };
    double py[4] = { pillc[6*pix + 1],
		     pillc[6*(pix + 1) + 1],
		     pillc[6*(pix + numxpill) + 1],
		     pillc[6*(pix + numxpill + 1) + 1] };
    double diag1[2] = { px[3] - px[0], py[3] - py[0] };
    double diag2[2] = { px[2] - px[1], py[2] - py[1] };
    double area = 0.5*(diag1[0]*diag2[1] - diag1[1]*diag2[0]);

    // Computing the average of the z-differences along each pillar.
    int delta[3] = { 1,
		     2*logical_gridsize_[0],
		     4*logical_gridsize_[0]*logical_gridsize_[1] };
    int ix = 2*(i*delta[0] + j*delta[1] + k*delta[2]);
    double cellz[8] = { z[ix], z[ix + delta[0]],
			z[ix + delta[1]], z[ix + delta[1] + delta[0]],
			z[ix + delta[2]], z[ix + delta[2] + delta[0]],
			z[ix + delta[2] + delta[1]], z[ix + delta[2] + delta[1] + delta[0]] };
    double diffz[4] = { cellz[4] - cellz[0],
			cellz[5] - cellz[1],
			cellz[6] - cellz[2],
			cellz[7] - cellz[3] };
    double averzdiff = 0.25*std::accumulate(diffz, diffz + 4, 0.0);
    return averzdiff*area;
}


double EclipseGridInspector::cellVolumeVerticalPillars(int cell_idx) const
{
    std::array<int, 3> idxs = cellIdxToLogicalCoords(cell_idx);
    return cellVolumeVerticalPillars(idxs[0], idxs[1], idxs[2]);
}

void EclipseGridInspector::checkLogicalCoords(int i, int j, int k) const
{
    if (i < 0 || i >= logical_gridsize_[0])
	throw std::runtime_error("First coordinate out of bounds");
    if (j < 0 || j >= logical_gridsize_[1])
	throw std::runtime_error("Second coordinate out of bounds");
    if (k < 0 || k >= logical_gridsize_[2])
	throw std::runtime_error("Third coordinate out of bounds");
}


std::array<double, 6> EclipseGridInspector::getGridLimits() const
{
    if (! (deck_.hasKeyword("COORD") && deck_.hasKeyword("ZCORN") && deck_.hasKeyword("SPECGRID")) ) {
        throw std::runtime_error("EclipseGridInspector: Grid does not have SPECGRID, COORD, and ZCORN, can't find dimensions.");
    }

    std::vector<double> coord = deck_["COORD"].back().getSIDoubleData();
    std::vector<double> zcorn = deck_["ZCORN"].back().getSIDoubleData();

    double xmin = +DBL_MAX;
    double xmax = -DBL_MAX;
    double ymin = +DBL_MAX;
    double ymax = -DBL_MAX;


    int pillars = (logical_gridsize_[0]+1) * (logical_gridsize_[1]+1);

    for (int pillarindex = 0; pillarindex < pillars; ++pillarindex) {
        if        (coord[pillarindex * 6 + 0] > xmax)
            xmax = coord[pillarindex * 6 + 0];
        if        (coord[pillarindex * 6 + 0] < xmin)
            xmin = coord[pillarindex * 6 + 0];
        if        (coord[pillarindex * 6 + 1] > ymax)
            ymax = coord[pillarindex * 6 + 1];
        if        (coord[pillarindex * 6 + 1] < ymin)
            ymin = coord[pillarindex * 6 + 1];
        if        (coord[pillarindex * 6 + 3] > xmax)
            xmax = coord[pillarindex * 6 + 3];
        if        (coord[pillarindex * 6 + 3] < xmin)
            xmin = coord[pillarindex * 6 + 3];
        if        (coord[pillarindex * 6 + 4] > ymax)
            ymax = coord[pillarindex * 6 + 4];
        if        (coord[pillarindex * 6 + 4] < ymin)
            ymin = coord[pillarindex * 6 + 4];
    }

    std::array<double, 6> gridlimits = {{ xmin, xmax, ymin, ymax,
                                            *min_element(zcorn.begin(), zcorn.end()),
                                            *max_element(zcorn.begin(), zcorn.end()) }};
    return gridlimits;
}



std::array<int, 3> EclipseGridInspector::gridSize() const
{
    std::array<int, 3> retval = {{ logical_gridsize_[0],
				     logical_gridsize_[1],
				     logical_gridsize_[2] }};
    return retval;
}


std::array<double, 8> EclipseGridInspector::cellZvals(int i, int j, int k) const
{
    // Get the zcorn field.
    const std::vector<double>& z = deck_["ZCORN"].back().getSIDoubleData();
    int num_cells = logical_gridsize_[0]*logical_gridsize_[1]*logical_gridsize_[2];
    if (8*num_cells != int(z.size())) {
	throw std::runtime_error("Wrong size of ZCORN field");
    }

    // Make the coordinate array.
    int delta[3] = { 1,
		     2*logical_gridsize_[0],
		     4*logical_gridsize_[0]*logical_gridsize_[1] };
    int ix = 2*(i*delta[0] + j*delta[1] + k*delta[2]);
    std::array<double, 8> cellz = {{ z[ix], z[ix + delta[0]],
				       z[ix + delta[1]], z[ix + delta[1] + delta[0]],
				       z[ix + delta[2]], z[ix + delta[2] + delta[0]],
				       z[ix + delta[2] + delta[1]], z[ix + delta[2] + delta[1] + delta[0]] }};
    return cellz;
}


} // namespace Opm
