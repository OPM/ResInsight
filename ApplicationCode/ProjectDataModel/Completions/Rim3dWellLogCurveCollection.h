/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
#include "cafPdmField.h"
#include "cafPdmObject.h"

class Rim3dWellLogCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dWellLogCurveCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    enum PlanePosition
    {
        ALONG_WELLPATH,
        ON_WELLPATH
    };

public:
    Rim3dWellLogCurveCollection();
    virtual ~Rim3dWellLogCurveCollection();

    bool has3dWellLogCurves() const;
    void add3dWellLogCurve(Rim3dWellLogCurve* curve);
    void remove3dWellLogCurve(Rim3dWellLogCurve* curve);

    bool isShowingPlot() const;
    bool isShowingGrid() const;
    bool isShowingBackground() const;

    PlanePosition planePositionVertical() const;
    PlanePosition planePositionHorizontal() const;
    float         planeWidthScaling() const;

    std::vector<Rim3dWellLogCurve*> vectorOf3dWellLogCurves() const;
    void                            redrawAffectedViewsAndEditors();

private:
    virtual void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual caf::PdmFieldHandle* objectToggleField() override;
    virtual void                 defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                 defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
private:
    caf::PdmField<bool>                         m_showPlot;
    caf::PdmField<caf::AppEnum<PlanePosition>>  m_planePositionVertical;
    caf::PdmField<caf::AppEnum<PlanePosition>>  m_planePositionHorizontal;
    caf::PdmField<float>                        m_planeWidthScaling;

    caf::PdmField<bool>                         m_showGrid;
    caf::PdmField<bool>                         m_showBackground;


    caf::PdmChildArrayField<Rim3dWellLogCurve*> m_3dWellLogCurves;
};
