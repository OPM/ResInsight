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

#include "RimCase.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

#include <QDateTime>

class RimGeoMechView;
class RigGeoMechCaseData;
class RifGeoMechReaderInterface;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechCase : public RimCase
{
     CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase(void);
    virtual ~RimGeoMechCase(void);
    
    void                                    setFileName(const QString& fileName);
    QString                                 caseFileName() const;
    bool                                    openGeoMechCase(std::string* errorMessage);

    RigGeoMechCaseData*                     geoMechData();
    const RigGeoMechCaseData*               geoMechData() const;

    RimGeoMechView*                         createAndAddReservoirView();

    virtual void                            updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath);
    virtual std::vector<RimView*>           views();

    virtual std::vector<QDateTime>          timeStepDates() const override;
    virtual QStringList                     timeStepStrings() const override;
    virtual QString                         timeStepName(int frameIdx) const override;

    virtual cvf::BoundingBox                activeCellsBoundingBox() const;
    virtual cvf::BoundingBox                allCellsBoundingBox() const;

    virtual double                          characteristicCellSize() const override;

    virtual void                            setFormationNames(RimFormationNames* formationNames) override;

    // Fields:                                        
    caf::PdmChildArrayField<RimGeoMechView*>  geoMechViews;


private:
    static std::vector<QDateTime>           dateTimeVectorFromTimeStepStrings(const QStringList& timeStepStrings);

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual void                            updateFormationNamesData() override;

    virtual void                            initAfterRead();
    static QString                          subStringOfDigits(const QString& timeStepString, int numberOfDigitsToFind);

private:
    cvf::ref<RigGeoMechCaseData>            m_geoMechCaseData;
    caf::PdmField<QString>                  m_caseFileName;
    caf::PdmField<double>                   m_cohesion;
    caf::PdmField<double>                   m_frictionAngleDeg;
};
