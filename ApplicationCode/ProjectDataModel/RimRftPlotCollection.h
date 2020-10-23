/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfCollection.h"

#include <gsl/gsl>

class RimWellLogPlot;
class RigEclipseWellLogExtractor;
class RigGeoMechWellLogExtractor;
class RimGeoMechCase;
class RigEclipseCaseData;
class RigGeoMechCaseData;
class RigWellPath;
class RimWellPath;
class RimEclipseCase;
class RiuWellLogPlot;
class RimWellRftPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimRftPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftPlotCollection();
    ~RimRftPlotCollection() override;

    RigEclipseWellLogExtractor* findOrCreateSimWellExtractor( const QString&                    simWellName,
                                                              const QString&                    caseUserDescription,
                                                              gsl::not_null<const RigWellPath*> wellPathGeom,
                                                              gsl::not_null<const RigEclipseCaseData*> eclCaseData );

    RigEclipseWellLogExtractor* findOrCreateExtractor( gsl::not_null<RimWellPath*>    wellPath,
                                                       gsl::not_null<RimEclipseCase*> eclCase );
    RigGeoMechWellLogExtractor* findOrCreateExtractor( gsl::not_null<RimWellPath*>    wellPath,
                                                       gsl::not_null<RimGeoMechCase*> eclCase );

    void removeExtractors( const RigWellPath* wellPath );
    void removeExtractors( const RigEclipseCaseData* caseData );
    void removeExtractors( const RigGeoMechCaseData* caseData );
    void deleteAllExtractors();

    const std::vector<RimWellRftPlot*> rftPlots() const;
    void                               addPlot( gsl::not_null<RimWellRftPlot*> newPlot );
    void                               removePlot( gsl::not_null<RimWellRftPlot*> plot );
    void                               deleteAllPlots();

private:
    caf::PdmChildArrayField<RimWellRftPlot*>    m_rftPlots;
    cvf::Collection<RigEclipseWellLogExtractor> m_extractors;
    cvf::Collection<RigGeoMechWellLogExtractor> m_geomExtractors;
};
