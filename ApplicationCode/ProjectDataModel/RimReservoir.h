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

class RigEclipseCase;
class RigGridBase;
class RimReservoirView;
class RimCaseCollection;
//class RimReservoirCellResultsCacher;

//==================================================================================================
// 
// Interface for reservoirs. 
// 
//==================================================================================================
class RimReservoir : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoir();
    virtual ~RimReservoir();

    // Fields:                                        
    caf::PdmField<QString>                      caseName;
    caf::PdmField<bool>                         releaseResultMemory;
    caf::PdmPointersField<RimReservoirView*>    reservoirViews;

    virtual bool                                openEclipseGridFile() { return false;}; // Should be pure virtual but PDM does not allow that.
                                                      
    RigEclipseCase*                             reservoirData();
    const RigEclipseCase*                       reservoirData() const;

    RimReservoirCellResultsStorage*		        results(RifReaderInterface::PorosityModelResultType porosityModel);
                                                      
    RimReservoirView*                           createAndAddReservoirView();
    void                                        removeReservoirView(RimReservoirView* reservoirView);

    void                                        removeResult(const QString& resultName);

    virtual QString                             locationOnDisc() const      { return QString(); }

    RimCaseCollection*                          parentCaseCollection();
                                                     
    // Overridden methods from PdmObject
public:
    virtual caf::PdmFieldHandle*                userDescriptionField()  { return &caseName; }
protected:
    virtual void                                initAfterRead();
    virtual void                                fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

    // Internal methods
protected:
    void                                        computeCachedData();
    void                                        setReservoirData(RigEclipseCase* eclipseCase);


private:
    cvf::ref<RigEclipseCase>                    m_rigEclipseCase;

private:
    caf::PdmField<RimReservoirCellResultsStorage*> m_matrixModelResults;
    caf::PdmField<RimReservoirCellResultsStorage*> m_fractureModelResults;

};
