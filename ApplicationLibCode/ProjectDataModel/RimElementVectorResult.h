/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h" // Include to make Pdm work for cvf::Color
#include "cafPdmObject.h"

#include "RigEclipseResultAddress.h"

#include "RimRegularLegendConfig.h"

#include <QList>

#include <vector>

class RimRegularLegendConfig;

class RiuViewer;

//==================================================================================================
///
///
//==================================================================================================
class RimElementVectorResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class TensorColors
    {
        UNIFORM_COLOR,
        RESULT_COLORS
    };

    enum class VectorView
    {
        CELL_CENTER_TOTAL,
        PER_FACE
    };

    enum class VectorSurfaceCrossingLocation
    {
        VECTOR_ANCHOR,
        VECTOR_CENTER
    };

public:
    RimElementVectorResult();
    ~RimElementVectorResult() override;

    void                          setShowResult( bool enableResult );
    bool                          showResult() const;
    VectorView                    vectorView() const;
    bool                          showOil() const;
    bool                          showGas() const;
    bool                          showWater() const;
    bool                          showVectorI() const;
    bool                          showVectorJ() const;
    bool                          showVectorK() const;
    bool                          showNncData() const;
    VectorSurfaceCrossingLocation vectorSuraceCrossingLocation() const;
    float                         threshold() const;
    float                         sizeScale() const;
    TensorColors                  vectorColors() const;

    const cvf::Color3f&           getUniformVectorColor() const;
    const RimRegularLegendConfig* legendConfig() const;

    void mappingRange( double& min, double& max ) const;

    bool resultAddressesCombined( std::vector<RigEclipseResultAddress>& addresses ) const;
    bool resultAddressesIJK( std::vector<RigEclipseResultAddress>& addresses ) const;

    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

private:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<bool>                                        m_showResult;
    caf::PdmField<bool>                                        m_showOil;
    caf::PdmField<bool>                                        m_showGas;
    caf::PdmField<bool>                                        m_showWater;
    caf::PdmField<caf::AppEnum<VectorView>>                    m_vectorView;
    caf::PdmField<bool>                                        m_showVectorI;
    caf::PdmField<bool>                                        m_showVectorJ;
    caf::PdmField<bool>                                        m_showVectorK;
    caf::PdmField<bool>                                        m_showNncData;
    caf::PdmField<caf::AppEnum<VectorSurfaceCrossingLocation>> m_vectorSurfaceCrossingLocation;
    caf::PdmField<float>                                       m_threshold;
    caf::PdmField<caf::AppEnum<TensorColors>>                  m_vectorColor;
    caf::PdmField<cvf::Color3f>                                m_uniformVectorColor;
    caf::PdmField<float>                                       m_sizeScale;
    caf::PdmField<RimRegularLegendConfig::RangeModeEnum>       m_rangeMode;
    caf::PdmChildField<RimRegularLegendConfig*>                m_legendConfig;
};
