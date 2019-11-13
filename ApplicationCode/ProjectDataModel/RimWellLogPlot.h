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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

#include "RiaDefines.h"
#include "RimMultiPlot.h"
#include "RimWellLogPlotNameConfig.h"

#include <QPointer>

#include <set>

class RimWellLogCurveCommonDataSource;
class RiuMultiPlotWindow;
class RimPlotInterface;
class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogPlot : public RimMultiPlot, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum AxisGridVisibility
    {
        AXIS_GRID_NONE            = 0x00,
        AXIS_GRID_MAJOR           = 0x01,
        AXIS_GRID_MINOR           = 0x02,
        AXIS_GRID_MAJOR_AND_MINOR = 0x03
    };

    typedef caf::AppEnum<AxisGridVisibility> AxisGridEnum;
    using DepthTypeEnum = RiaDefines::DepthTypeEnum;

public:
    RimWellLogPlot();
    ~RimWellLogPlot() override;

    QWidget* createPlotWidget( QWidget* mainWindowParent = nullptr );

    RimWellLogPlot& operator=( RimWellLogPlot&& rhs );

    DepthTypeEnum depthType() const;
    void          setDepthType( DepthTypeEnum depthType );

    RiaDefines::DepthUnitType depthUnit() const;
    void                      setDepthUnit( RiaDefines::DepthUnitType depthUnit );

    QString            depthAxisTitle() const;
    void               enableDepthAxisGridLines( AxisGridVisibility gridVisibility );
    AxisGridVisibility depthAxisGridLinesEnabled() const;

    void updateZoom() override;
    void setDepthAxisRangeByFactorAndCenter( double zoomFactor, double zoomCenter );
    void setDepthAxisRangeByPanDepth( double panFactor );
    void setDepthAxisRange( double minimumDepth, double maximumDepth );

    void calculateAvailableDepthRange();
    void availableDepthRange( double* minimumDepth, double* maximumDepth ) const;

    void setAutoScaleYEnabled( bool enabled ) override;
    void enableAllAutoNameTags( bool enable );

    void uiOrderingForDepthAxis( caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForPlotLayout( caf::PdmUiOrdering& uiOrdering ) override;

    QString createAutoName() const override;

    RimWellLogCurveCommonDataSource* commonDataSource() const;
    void                             updateCommonDataSource();
    void                             setCommonDataSourceEnabled( bool enable );

    void setAvailableDepthUnits( const std::set<RiaDefines::DepthUnitType>& depthUnits );
    void setAvailableDepthTypes( const std::set<DepthTypeEnum>& depthTypes );

    void onPlotAdditionOrRemoval() override;

    void updatePlotNames() override;

protected:
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     performAutoNameUpdate() override;
    void     handleKeyPressEvent( QKeyEvent* keyEvent ) override;

    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void initAfterRead() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    QImage snapshotWindowContent() override;

protected:
    caf::PdmChildField<RimWellLogCurveCommonDataSource*> m_commonDataSource;
    bool                                                 m_commonDataSourceEnabled;

    caf::PdmField<caf::AppEnum<DepthTypeEnum>>             m_depthType;
    caf::PdmField<caf::AppEnum<RiaDefines::DepthUnitType>> m_depthUnit;
    caf::PdmField<double>                                  m_minVisibleDepth;
    caf::PdmField<double>                                  m_maxVisibleDepth;
    caf::PdmField<AxisGridEnum>                            m_depthAxisGridVisibility;
    caf::PdmField<bool>                                    m_isAutoScaleDepthEnabled;

    caf::PdmChildField<RimWellLogPlotNameConfig*> m_nameConfig;

    std::set<RiaDefines::DepthUnitType> m_availableDepthUnits;
    std::set<DepthTypeEnum>             m_availableDepthTypes;

    double m_minAvailableDepth;
    double m_maxAvailableDepth;
};
