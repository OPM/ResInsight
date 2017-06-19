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

#include "cafPdmPtrField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

class RimView;
class RimFormationNames;

namespace cvf {
    class BoundingBox;
}

class RimCase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCase();
    virtual ~RimCase();
    
    caf::PdmField<int>                          caseId;
    caf::PdmField<QString>                      caseUserDescription;

    caf::PdmPtrField<RimFormationNames*>        activeFormationNames;

    virtual std::vector<RimView*>               views() = 0;

    virtual void                                updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath) = 0;

    virtual std::vector<QDateTime>              timeStepDates() const = 0;
    virtual QStringList                         timeStepStrings() const = 0;
    virtual QString                             timeStepName(int frameIdx) const = 0;

    virtual cvf::BoundingBox                    activeCellsBoundingBox() const = 0;
    virtual cvf::BoundingBox                    allCellsBoundingBox() const = 0;

    virtual cvf::Vec3d                          displayModelOffset() const;

    virtual void                                updateFormationNamesData() = 0;

    virtual double                              characteristicCellSize() const = 0;

protected:
    virtual QList<caf::PdmOptionItemInfo>       calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
private:
    virtual caf::PdmFieldHandle*                userDescriptionField() override { return &caseUserDescription; }
};



