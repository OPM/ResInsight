/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaPorosityModel.h"

#include "RimCase.h"
#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfColor3.h"

class QString;

class RigEclipseCaseData;
class RigGridBase;
class RimCaseCollection;
class RimIdenticalGridCaseGroup;
class RimReservoirCellResultsStorage;
class RimEclipseView;


//==================================================================================================
// 
// Interface for reservoirs. 
// 
//==================================================================================================
class RimEclipseCase : public RimCase
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseCase();
    virtual ~RimEclipseCase();


    // Fields:                                        
    caf::PdmField<bool>                         releaseResultMemory;
    caf::PdmChildArrayField<RimEclipseView*>    reservoirViews;
    caf::PdmField<bool>                         flipXAxis;
    caf::PdmField<bool>                         flipYAxis;
    
    std::vector<QString>                        filesContainingFaults() const;
    void                                        setFilesContainingFaults(const std::vector<QString>& val);

    bool                                        openReserviorCase();
    virtual bool                                openEclipseGridFile() = 0;
                                                      
    RigEclipseCaseData*                         eclipseCaseData();
    const RigEclipseCaseData*                   eclipseCaseData() const;
    cvf::Color3f                                defaultWellColor(const QString& wellName);

    RimReservoirCellResultsStorage*             results(RiaDefines::PorosityModelType porosityModel);
    const RimReservoirCellResultsStorage*       results(RiaDefines::PorosityModelType porosityModel) const;
                                                      
    RimEclipseView*                             createAndAddReservoirView();
    RimEclipseView*                             createCopyAndAddView(const RimEclipseView* sourceView);

    void                                        removeEclipseResultAndScheduleRedrawAllViews(RiaDefines::ResultCatType type, const QString& resultName);

    virtual QString                             locationOnDisc() const      { return QString(); }
    virtual QString                             gridFileName() const      { return QString(); }


    RimCaseCollection*                          parentCaseCollection();
                                                     
    virtual std::vector<RimView*>               views();
    virtual QStringList                         timeStepStrings() const override;
    virtual QString                             timeStepName(int frameIdx) const override;
    virtual std::vector<QDateTime>              timeStepDates() const override;


    virtual cvf::BoundingBox                    activeCellsBoundingBox() const;
    virtual cvf::BoundingBox                    allCellsBoundingBox() const;
    virtual cvf::Vec3d                          displayModelOffset() const;

    void                                        reloadDataAndUpdate();
    virtual void                                reloadEclipseGridFile() = 0;


    virtual double                              characteristicCellSize() const override;

protected:
    virtual void                                initAfterRead();
    virtual void                                fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

    virtual void                                updateFormationNamesData() override;

    // Internal methods
protected:
    void                                        computeCachedData();
    void                                        setReservoirData(RigEclipseCaseData* eclipseCase);

private:
    void                                        createTimeStepFormatString();

private:
    cvf::ref<RigEclipseCaseData>                m_rigEclipseCase;
    QString                                     m_timeStepFormatString;
    std::map<QString , cvf::Color3f>            m_wellToColorMap;
    caf::PdmField<QString >                     m_filesContainingFaultsSemColSeparated;


    caf::PdmChildField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmChildField<RimReservoirCellResultsStorage*> m_fractureModelResults;

    // Obsolete fields
protected:
    caf::PdmField<QString>                      caseName;
private:
    caf::PdmField<std::vector<QString> >        m_filesContainingFaults_OBSOLETE;

};
