/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaPlotDefines.h"

#include "cafAppEnum.h"
#include "cafFontTools.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimColorLegend;

//==================================================================================================
///
/// Settings class for region annotation configuration in well log tracks
///
//==================================================================================================
class RimWellLogRegionAnnotationSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using RegionAnnotationTypeEnum    = caf::AppEnum<RiaDefines::RegionAnnotationType>;
    using RegionAnnotationDisplayEnum = caf::AppEnum<RiaDefines::RegionDisplay>;

    RimWellLogRegionAnnotationSettings();
    ~RimWellLogRegionAnnotationSettings() override;

    // Annotation type
    RiaDefines::RegionAnnotationType annotationType() const;
    void                             setAnnotationType( RiaDefines::RegionAnnotationType type );

    // Annotation display
    RiaDefines::RegionDisplay annotationDisplay() const;
    void                      setAnnotationDisplay( RiaDefines::RegionDisplay display );

    // Color shading
    RimColorLegend* colorShadingLegend() const;
    void            setColorShadingLegend( RimColorLegend* legend );

    int  colorShadingTransparency() const;
    void setColorShadingTransparency( int transparency );

    // Region labels
    bool showRegionLabels() const;
    void setShowRegionLabels( bool show );

    caf::FontTools::RelativeSize regionLabelFontSize() const;
    void                         setRegionLabelFontSize( caf::FontTools::RelativeSize fontSize );

    // Helpers
    bool showFormations() const;
    void uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<RegionAnnotationTypeEnum>                   m_regionAnnotationType;
    caf::PdmField<RegionAnnotationDisplayEnum>                m_regionAnnotationDisplay;
    caf::PdmPtrField<RimColorLegend*>                         m_colorShadingLegend;
    caf::PdmField<int>                                        m_colorShadingTransparency;
    caf::PdmField<bool>                                       m_showRegionLabels;
    caf::PdmField<caf::AppEnum<caf::FontTools::RelativeSize>> m_regionLabelFontSize;
};
