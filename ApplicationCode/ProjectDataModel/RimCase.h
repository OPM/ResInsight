/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "RimReservoirCellResultsCacher.h"
#include "RifReaderInterface.h"

class QString;

class RigCaseData;
class RigGridBase;
class RimReservoirView;
class RimCaseCollection;
class RimIdenticalGridCaseGroup;
//class RimReservoirCellResultsCacher;

//==================================================================================================
// 
// Interface for reservoirs. 
// 
//==================================================================================================
class RimCase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCase();
    virtual ~RimCase();


    // Fields:                                        
    caf::PdmField<QString>                      caseUserDescription;
    caf::PdmField<bool>                         releaseResultMemory;
    caf::PdmPointersField<RimReservoirView*>    reservoirViews;

    virtual bool                                openEclipseGridFile() { return false;}; // Should be pure virtual but PDM does not allow that.
                                                      
    RigCaseData*                                reservoirData();
    const RigCaseData*                          reservoirData() const;
    void                                        removeReservoirData();

    RimReservoirCellResultsStorage*		        results(RifReaderInterface::PorosityModelResultType porosityModel);
                                                      
    RimReservoirView*                           createAndAddReservoirView();
    void                                        removeReservoirView(RimReservoirView* reservoirView);

    void                                        removeResult(const QString& resultName);

    virtual QString                             locationOnDisc() const      { return QString(); }
    virtual QString                             gridFileName() const      { return QString(); }

    virtual void                                updateFilePathsFromProjectPath(const QString& projectPath) { };

    RimCaseCollection*                          parentCaseCollection();
    RimIdenticalGridCaseGroup*                  parentGridCaseGroup();
                                                     
                                                     
    // Overridden methods from PdmObject
public:
    virtual caf::PdmFieldHandle*                userDescriptionField()  { return &caseUserDescription; }
protected:
    virtual void                                initAfterRead();
    virtual void                                fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

    // Internal methods
protected:
    void                                        computeCachedData();
    void                                        setReservoirData(RigCaseData* eclipseCase);


private:
    cvf::ref<RigCaseData>                       m_rigEclipseCase;

private:
    caf::PdmField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmField<RimReservoirCellResultsStorage*> m_fractureModelResults;

    // Obsolete fields
protected:
    caf::PdmField<QString>                      caseName;
};
