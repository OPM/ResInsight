/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigFractureTransmissibilityEquations
{
public:
    static double centerToCenterFractureCellTrans(double conductivityCell1,
                                                  double sideLengthParallellTransCell1,
                                                  double sideLengthNormalTransCell1,
                                                  double conductivityCell2,
                                                  double sideLengthParallellTransCell2,
                                                  double sideLengthNormalTransCell2,
                                                  double cDarcyForRelevantUnit);

    static double fractureCellToWellRadialTrans(double fractureCellConductivity,
                                                double fractureCellSizeX,
                                                double fractureCellSizeZ,
                                                double wellRadius,
                                                double skinFactor,
                                                double cDarcyForRelevantUnit);

    static double fractureCellToWellLinearTrans(double fractureConductivity,
                                                double fractureCellSizeX,
                                                double fractureCellSizeZ,
                                                double perforationLengthVertical,
                                                double perforationLengthHorizontal,
                                                double perforationEfficiency,
                                                double skinfactor,
                                                double cDarcyForRelevantUnit,
                                                double wellRadius);

    static double matrixToFractureTrans(double permX,
                                        double NTG,
                                        double Ay,
                                        double dx,
                                        double skinfactor,
                                        double fractureAreaWeightedlength,
                                        double cDarcy);

    static double effectiveInternalFractureToWellTransPDDHC(double sumScaledMatrixToFractureTrans,
                                                            double scaledMatrixToWellTrans);

    static double effectiveMatrixToWellTransPDDHC(double sumOriginalMatrixToFractureTrans,
                                                  double effectiveInternalFractureToWellTrans);

    static double matrixPermeability(double permx, double permy, double NTG);

private:
    static double centerToEdgeFractureCellTrans(double conductivity,
                                                double sideLengthParallellTrans,
                                                double sideLengthNormalTrans,
                                                double cDarcyForRelevantUnit);

private:
    static const double EPSILON;
};
