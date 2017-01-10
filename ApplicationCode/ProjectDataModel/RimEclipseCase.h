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

#include "cvfBase.h"
#include "cvfObject.h"

#include "RimCase.h"
#include "RimEclipseView.h"


#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RifReaderInterface.h"

class QString;

class RigEclipseCaseData;
class RigGridBase;
class RimCaseCollection;
class RimIdenticalGridCaseGroup;
class RimReservoirCellResultsStorage;


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
    
    caf::PdmField<std::vector<QString> >        filesContainingFaults;


    bool                                        openReserviorCase();
    virtual bool                                openEclipseGridFile() = 0;
                                                      
    RigEclipseCaseData*                                reservoirData();
    const RigEclipseCaseData*                          reservoirData() const;

    RimReservoirCellResultsStorage*             results(RifReaderInterface::PorosityModelResultType porosityModel);
                                                      
    RimEclipseView*                             createAndAddReservoirView();
    RimEclipseView*                             createCopyAndAddView(const RimEclipseView* sourceView);

    void                                        removeResult(const QString& resultName);

    virtual QString                             locationOnDisc() const      { return QString(); }
    virtual QString                             gridFileName() const      { return QString(); }


    RimCaseCollection*                          parentCaseCollection();
                                                     
    virtual std::vector<RimView*>               views();
    virtual QStringList                         timeStepStrings();
    virtual QString                             timeStepName(int frameIdx);

    virtual cvf::BoundingBox                    activeCellsBoundingBox() const;
    virtual cvf::BoundingBox                    allCellsBoundingBox() const;
    virtual cvf::Vec3d                          displayModelOffset() const;

    // Overridden methods from PdmObject
public:

protected:
    virtual void                                initAfterRead();
    virtual void                                fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

    virtual void                                updateFormationNamesData() override;

    // Internal methods
protected:
    void                                        computeCachedData();
    void                                        setReservoirData(RigEclipseCaseData* eclipseCase);

private:
    cvf::ref<RigEclipseCaseData>                       m_rigEclipseCase;

private:
    caf::PdmChildField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmChildField<RimReservoirCellResultsStorage*> m_fractureModelResults;
    QString                                     m_timeStepFormatString;

    // Obsolete fields
protected:
    caf::PdmField<QString>                      caseName;
};
