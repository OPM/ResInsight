/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "CommandFileInterface/Core/RicfCommandObject.h"

#include "cafCmdFeature.h"
#include "cafPdmField.h"

#include <utility>

class RimContourMapProjection;
class RimEclipseContourMapView;
class RimGeoMechContourMapView;

class QTextStream;

//==================================================================================================
///
//==================================================================================================
class RicExportContourMapToTextFeature : public caf::CmdFeature, public RicfCommandObject
{
    RICF_HEADER_INIT;

public:
    RicExportContourMapToTextFeature();
    caf::PdmScriptResponse execute() override;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    static void writeMetaDataToStream( QTextStream&                   stream,
                                       const RimContourMapProjection* contourMapProjection,
                                       const QString&                 caseName,
                                       bool                           exportLocalCoordinates );

    static void writeContourMapToStream( QTextStream&                   stream,
                                         const RimContourMapProjection* contourMapProjection,
                                         bool                           exportLocalCoordinates,
                                         const QString&                 undefinedValueLabel,
                                         bool                           excludeUndefinedValues );

private:
    static std::pair<RimEclipseContourMapView*, RimGeoMechContourMapView*> findContourMapView();

private:
    caf::PdmField<QString> m_exportFileName;
    caf::PdmField<bool>    m_exportLocalCoordinates;
    caf::PdmField<QString> m_undefinedValueLabel;
    caf::PdmField<bool>    m_excludeUndefinedValues;
    caf::PdmField<int>     m_viewId;
};
