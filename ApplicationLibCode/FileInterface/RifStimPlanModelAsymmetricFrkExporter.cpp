/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-    Equinor ASA
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

#include "RifStimPlanModelAsymmetricFrkExporter.h"

#include "RiaEclipseUnitTools.h"

#include "RimStimPlanModel.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelAsymmetricFrkExporter::writeToFile( RimStimPlanModel* stimPlanModel, const QString& filepath )
{
    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    double bedDipDeg            = stimPlanModel->formationDip();
    bool   hasBarrier           = stimPlanModel->hasBarrier();
    double distanceToBarrier    = stimPlanModel->distanceToBarrier();
    double barrierDip           = stimPlanModel->barrierDip();
    int    wellPenetrationLayer = stimPlanModel->wellPenetrationLayer();

    appendBarrierDataToStream( stream,
                               bedDipDeg,
                               hasBarrier,
                               RiaEclipseUnitTools::meterToFeet( distanceToBarrier ),
                               barrierDip,
                               wellPenetrationLayer );

    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelAsymmetricFrkExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << "<asymmetric>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelAsymmetricFrkExporter::appendBarrierDataToStream( QTextStream& stream,
                                                                       double       bedDipDeg,
                                                                       bool         hasBarrier,
                                                                       double       distanceToBarrier,
                                                                       double       barrierDipDeg,
                                                                       int          wellPenetrationLayer )
{
    stream << "<BedDipDeg>" << endl
           << bedDipDeg << endl
           << "</BedDipDeg>" << endl
           << "<Barrier>" << endl
           << static_cast<int>( hasBarrier ) << endl
           << "</Barrier>" << endl
           << "<BarrierDipDeg>" << endl
           << barrierDipDeg << endl
           << "</BarrierDipDeg>" << endl
           << "<DistanceToBarrier>" << endl
           << distanceToBarrier << endl
           << "</DistanceToBarrier>" << endl
           << "<WellPenetrationLayer>" << endl
           << wellPenetrationLayer << endl
           << "</WellPenetrationLayer>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelAsymmetricFrkExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</asymmetric>" << endl;
}
