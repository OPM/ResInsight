/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimFractureTemplate.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RimMeshFractureTemplate : public RimFractureTemplate
{
    CAF_PDM_HEADER_INIT;

public:
    RimMeshFractureTemplate();
    ~RimMeshFractureTemplate() override;

    int activeTimeStepIndex();

    virtual void setDefaultsBasedOnFile() = 0;

    void    setFileName( const QString& fileName );
    QString fileName();

    QString wellPathDepthAtFractureUiName() const override;

    // Result Access
    virtual std::vector<double>  timeSteps()                     = 0;
    virtual std::vector<QString> timeStepsStrings()              = 0;
    virtual QStringList          conductivityResultNames() const = 0;
    virtual std::vector<std::vector<double>>
        resultValues( const QString& uiResultName, const QString& unitName, size_t timeStepIndex ) const = 0;
    virtual std::vector<double>
                   fractureGridResults( const QString& resultName, const QString& unitName, size_t timeStepIndex ) const = 0;
    virtual bool   hasConductivity() const     = 0;
    virtual double resultValueAtIJ( const RigFractureGrid* fractureGrid,
                                    const QString&         uiResultName,
                                    const QString&         unitName,
                                    size_t                 timeStepIndex,
                                    size_t                 i,
                                    size_t                 j ) = 0;

    virtual bool isValidResult( double value ) const = 0;

    virtual QString getFileSelectionFilter() const = 0;

    virtual std::vector<double> widthResultValues() const;

    QString mapUiResultNameToFileResultName( const QString& uiResultName ) const;

protected:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          onLoadDataAndUpdateGeometryHasChanged() override;

    virtual std::vector<double>
        fractureGridResultsForUnitSystem( const QString&                resultName,
                                          const QString&                unitName,
                                          size_t                        timeStepIndex,
                                          RiaDefines::EclipseUnitSystem requiredUnitSystem ) const = 0;

    WellFractureIntersectionData wellFractureIntersectionData( const RimFracture* fractureInstance ) const override;

    virtual std::pair<QString, QString> widthParameterNameAndUnit() const        = 0;
    virtual std::pair<QString, QString> conductivityParameterNameAndUnit() const = 0;
    virtual std::pair<QString, QString> betaFactorParameterNameAndUnit() const   = 0;

    double conversionFactorForBetaValues() const;

protected:
    caf::PdmField<caf::FilePath> m_stimPlanFileName;
    bool                         m_readError;

    caf::PdmField<int>     m_activeTimeStepIndex;
    caf::PdmField<QString> m_conductivityResultNameOnFile;

    caf::PdmField<bool>    m_userDefinedWellPathDepthAtFracture;
    caf::PdmField<QString> m_borderPolygonResultName;
};
