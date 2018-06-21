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

#include "cafFilePath.h"
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
    enum CaseOpenStatus
    {
        CASE_OPEN_OK = 0,
        CASE_OPEN_CANCELLED,
        CASE_OPEN_ERROR
    };
    RimGeoMechCase(void);
    virtual ~RimGeoMechCase(void);
    
    void                                    setFileName(const QString& fileName);
    QString                                 caseFileName() const;
    CaseOpenStatus                          openGeoMechCase(std::string* errorMessage);

    RigGeoMechCaseData*                     geoMechData();
    const RigGeoMechCaseData*               geoMechData() const;

    void                                    reloadDataAndUpdate();

    RimGeoMechView*                         createAndAddReservoirView();

    virtual void                            updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath);

    virtual std::vector<QDateTime>          timeStepDates() const override;
    virtual QStringList                     timeStepStrings() const override;
    virtual QString                         timeStepName(int frameIdx) const override;

    virtual cvf::BoundingBox                activeCellsBoundingBox() const override;
    virtual cvf::BoundingBox                allCellsBoundingBox() const override;

    virtual double                          characteristicCellSize() const override;

    virtual void                            setFormationNames(RimFormationNames* formationNames) override;

    void                                    addElementPropertyFiles(const std::vector<caf::FilePath>& filenames);

    double                                  cohesion() const;
    double                                  frictionAngleDeg() const;

    void                                    setApplyTimeFilter(bool applyTimeFilter);
    // Fields:                                        
    caf::PdmChildArrayField<RimGeoMechView*>  geoMechViews;

private:
    virtual cvf::Vec3d                      displayModelOffset() const override;
    static std::vector<QDateTime>           vectorOfValidDateTimesFromTimeStepStrings(const QStringList& timeStepStrings);
    static QDateTime                        dateTimeFromTimeStepString(const QString& timeStepString);

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual void                            updateFormationNamesData() override;

    virtual void                            initAfterRead() override;
    static QString                          subStringOfDigits(const QString& timeStepString, int numberOfDigitsToFind);

    void                                    closeSelectedElementPropertyFiles();
    void                                    reloadSelectedElementPropertyFiles();
    virtual std::vector<Rim3dView*>         allSpecialViews() const override;

private:
    cvf::ref<RigGeoMechCaseData>              m_geoMechCaseData;
    caf::PdmField<caf::FilePath>              m_caseFileName;
    caf::PdmField<double>                     m_cohesion;
    caf::PdmField<double>                     m_frictionAngleDeg;
    caf::PdmField<std::vector<caf::FilePath>> m_elementPropertyFileNames;
    caf::PdmField<std::vector<int> >          m_elementPropertyFileNameIndexUiSelection;
    caf::PdmField<bool>                       m_closeElementPropertyFileCommand;
    caf::PdmField<bool>                       m_reloadElementPropertyFileCommand;
    bool                                      m_applyTimeFilter;
};
