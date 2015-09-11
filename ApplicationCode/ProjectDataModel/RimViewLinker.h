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

#include "RimDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimViewLink;
class RiuViewer;
class RimView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewLinker : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewLinker(void);
    virtual ~RimViewLinker(void);

    void     setMainView(RimView* view);
    RimView* mainView();

    caf::PdmChildArrayField<RimViewLink*> linkedViews;

    void applyAllOperations();

    void updateTimeStep(RimView* sourceView, int timeStep);
    void updateCellResult();

    void updateRangeFilters();
    void updatePropertyFilters();

    void configureOverrides();

    void updateScaleZ(RimView* source, double scaleZ);
    void allViewsForCameraSync(RimView* source, std::vector<RimView*>& views);
    void allViews(std::vector<RimView*>& views);

    void updateUiIcon();

public:
    static QString  displayNameForView(RimView* view);
    RimViewLink*  linkedViewFromView(RimView* view);

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual caf::PdmFieldHandle*            objectToggleField()     { return &m_isActive; }
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                            initAfterRead();

    void setNameAndIcon();

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

private:
    bool isActive();

private:
    caf::PdmField<bool>         m_isActive;
    caf::PdmPtrField<RimView*>  m_mainView;
    caf::PdmField<QString>      m_name;
    QIcon                       m_originalIcon;
};
