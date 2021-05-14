/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaResultNames.h"

#include "cafAppEnum.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaResultNames::isPerCellFaceResult( const QString& resultName )
{
    if ( resultName.compare( RiaResultNames::combinedTransmissibilityResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedMultResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::ternarySaturationResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedRiTranResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedRiMultResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedWaterFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedOilFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.compare( RiaResultNames::combinedGasFluxResultName(), Qt::CaseInsensitive ) == 0 )
    {
        return true;
    }
    else if ( resultName.endsWith( "IJK" ) )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::activeFormationNamesResultName()
{
    return "Active Formation Names";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::undefinedResultName()
{
    const static QString undefResultName = "None";

    return undefResultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::undefinedGridFaultName()
{
    return "Undefined Grid Faults";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::undefinedGridFaultWithInactiveName()
{
    return "Undefined Grid Faults With Inactive";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedTransmissibilityResultName()
{
    return "TRANXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedWaterFluxResultName()
{
    return "FLRWATIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedOilFluxResultName()
{
    return "FLROILIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedGasFluxResultName()
{
    return "FLRGASIJK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::ternarySaturationResultName()
{
    return "TERNARY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedMultResultName()
{
    return "MULTXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::eqlnumResultName()
{
    return "EQLNUM";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riTranXResultName()
{
    return "riTRANX";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riTranYResultName()
{
    return "riTRANY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riTranZResultName()
{
    return "riTRANZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedRiTranResultName()
{
    return "riTRANXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riMultXResultName()
{
    return "riMULTX";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riMultYResultName()
{
    return "riMULTY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riMultZResultName()
{
    return "riMULTZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedRiMultResultName()
{
    return "riMULTXYZ";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riAreaNormTranXResultName()
{
    return "riTRANXbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riAreaNormTranYResultName()
{
    return "riTRANYbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riAreaNormTranZResultName()
{
    return "riTRANZbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::combinedRiAreaNormTranResultName()
{
    return "riTRANXYZbyArea";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riCellVolumeResultName()
{
    return "riCELLVOLUME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::riOilVolumeResultName()
{
    return "riOILVOLUME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::mobilePoreVolumeName()
{
    return "MOBPORV";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::faultReactAssessmentPrefix()
{
    return "FAULT_";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::completionTypeResultName()
{
    return "Completion Type";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::formationBinaryAllanResultName()
{
    return "Binary Formation Allan";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::formationAllanResultName()
{
    return "Formation Allan";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsAzimuthResult()
{
    return "Azimuth";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsInclinationResult()
{
    return "Inclination";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsPPResult()
{
    return "PP";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsSHResult()
{
    return "SHMIN";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsSHMkResult()
{
    return "SH_MK";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsOBGResult()
{
    return "OBG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsFGResult()
{
    return "FG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaResultNames::wbsSFGResult()
{
    return "SFG";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RiaResultNames::nncResultNames()
{
    return { combinedTransmissibilityResultName(),
             formationAllanResultName(),
             formationBinaryAllanResultName(),
             combinedWaterFluxResultName(),
             combinedGasFluxResultName(),
             combinedOilFluxResultName(),
             combinedRiAreaNormTranResultName(),
             combinedRiMultResultName(),
             combinedRiTranResultName() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaResultNames::wbsAngleResultNames()
{
    return { wbsAzimuthResult(), wbsInclinationResult() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaResultNames::wbsDerivedResultNames()
{
    return {
        wbsFGResult(),
        wbsOBGResult(),
        wbsPPResult(),
        wbsSFGResult(),
        wbsSHResult(),
        wbsSHMkResult(),
    };
}
