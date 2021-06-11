/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimPolylineTarget.h"
#include "RimPolylinePickerInterface.h"

#include "cafPdmUiCheckBoxEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimPolylineTarget, "PolylineTarget" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineTarget::RimPolylineTarget()
    : m_isFullUpdateEnabled( true )
{
    CAF_PDM_InitField( &m_isEnabled, "IsEnabled", true, "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_targetPointXyd, "TargetPointXyd", "Point", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineTarget::~RimPolylineTarget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylineTarget::isEnabled() const
{
    return m_isEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::setAsPointTargetXYD( const cvf::Vec3d& point )
{
    m_targetPointXyd = point;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::setAsPointXYZ( const cvf::Vec3d& point )
{
    m_targetPointXyd = cvf::Vec3d( point.x(), point.y(), -point.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimPolylineTarget::targetPointXYZ() const
{
    cvf::Vec3d xyzPoint( m_targetPointXyd() );
    xyzPoint.z() = -xyzPoint.z();
    return xyzPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldHandle* RimPolylineTarget::targetPointUiCapability()
{
    return m_targetPointXyd.uiCapability();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::enableFullUpdate( bool enable )
{
    m_isFullUpdateEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolylineTarget::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    triggerVisualizationUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::triggerVisualizationUpdate() const
{
    RimPolylinePickerInterface* ppInterface;
    firstAncestorOrThisOfTypeAsserted( ppInterface );
    if ( ppInterface ) ppInterface->updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineTarget::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_targetPointXyd.uiCapability()->setUiReadOnly( m_isEnabled() );
}
