/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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
#include "RimLineBasedAnnotation.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include "cafPdmUiOrdering.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class QString;
class RimGridView;
class RigPolyLinesData;

//==================================================================================================
///
///
//==================================================================================================
class RimPolylinesAnnotation : public RimLineBasedAnnotation
{
    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;

public:
    RimPolylinesAnnotation();
    ~RimPolylinesAnnotation();

    bool isActive();

    virtual cvf::ref<RigPolyLinesData> polyLinesData() = 0;
    virtual bool isEmpty() = 0;

protected:
    virtual caf::PdmFieldHandle* objectToggleField() override;

private:
    caf::PdmField<bool> m_isActive;
};

//==================================================================================================
///
///
//==================================================================================================

class RimUserDefinedPolylinesAnnotation : public RimPolylinesAnnotation
{
    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;
public:
    RimUserDefinedPolylinesAnnotation();
    ~RimUserDefinedPolylinesAnnotation();

    cvf::ref<RigPolyLinesData>  polyLinesData() override;
    virtual bool isEmpty() override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<std::vector<Vec3d>>   m_points;
};

//==================================================================================================
///
///
//==================================================================================================


class RimPolylinesFromFileAnnotation : public RimPolylinesAnnotation
{
    CAF_PDM_HEADER_INIT;
public:
    RimPolylinesFromFileAnnotation();
    ~RimPolylinesFromFileAnnotation();

    void                        setFileName(const QString& fileName);
    QString                     fileName() const;
    void                        readPolyLinesFile(QString * errorMessage);

    cvf::ref<RigPolyLinesData>  polyLinesData() override { return m_polyLinesData;}
    virtual bool                isEmpty() override;

    void                        setDescriptionFromFileName();

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    virtual caf::PdmFieldHandle* userDescriptionField() override;

    caf::PdmField<QString> m_userDescription;
    caf::PdmField<caf::FilePath> m_polyLinesFileName;
    cvf::ref<RigPolyLinesData> m_polyLinesData;
};


//==================================================================================================
///
///
//==================================================================================================
class RigPolyLinesData : public cvf::Object
{
public:
    RigPolyLinesData() {}

    const std::vector<std::vector<cvf::Vec3d> >& polyLines() const            { return m_polylines;}
    void setPolyLines(const std::vector<std::vector<cvf::Vec3d> >& polyLines) { m_polylines = polyLines;}

private:
    std::vector<std::vector<cvf::Vec3d> > m_polylines;
};