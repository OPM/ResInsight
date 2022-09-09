/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RimMeshFractureTemplate.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"

#include <vector>

class RigStimPlanFractureDefinition;
class RimStimPlanLegendConfig;
class RigFractureCell;
class RigFractureGrid;

//==================================================================================================
///
///
//==================================================================================================
class RimStimPlanFractureTemplate : public RimMeshFractureTemplate
{
    CAF_PDM_HEADER_INIT;

public:
    RimStimPlanFractureTemplate();
    ~RimStimPlanFractureTemplate() override;

    void loadDataAndUpdate() override;
    void setDefaultsBasedOnFile() override;

    std::pair<double, double> wellPathDepthAtFractureRange() const override;
    QString                   wellPathDepthAtFractureUiName() const override;

    // Fracture geometry
    cvf::cref<RigFractureGrid> createFractureGrid( double wellPathDepthAtFracture ) const override;

    void fractureTriangleGeometry( std::vector<cvf::Vec3f>* nodeCoords,
                                   std::vector<cvf::uint>*  triangleIndices,
                                   double                   wellPathDepthAtFracture ) const override;

    // Result Access
    std::vector<QString>                     timeStepsStrings() override;
    std::vector<double>                      timeSteps() override;
    std::vector<std::pair<QString, QString>> uiResultNamesWithUnit() const override;
    std::vector<std::vector<double>>
        resultValues( const QString& uiResultName, const QString& unitName, size_t timeStepIndex ) const override;
    std::vector<double>
           fractureGridResults( const QString& resultName, const QString& unitName, size_t timeStepIndex ) const override;
    bool   hasConductivity() const override;
    double resultValueAtIJ( const RigFractureGrid* fractureGrid,
                            const QString&         uiResultName,
                            const QString&         unitName,
                            size_t                 timeStepIndex,
                            size_t                 i,
                            size_t                 j ) override;

    void appendDataToResultStatistics( const QString&     uiResultName,
                                       const QString&     unit,
                                       MinMaxAccumulator& minMaxAccumulator,
                                       PosNegAccumulator& posNegAccumulator ) const override;

    void setDefaultConductivityResultIfEmpty();
    bool setBorderPolygonResultNameToDefault();

    void computeDepthOfWellPathAtFracture();
    void computePerforationLength();

    std::vector<double> fractureGridResultsForUnitSystem( const QString&                resultName,
                                                          const QString&                unitName,
                                                          size_t                        timeStepIndex,
                                                          RiaDefines::EclipseUnitSystem requiredUnitSystem ) const override;

    std::pair<QString, QString> widthParameterNameAndUnit() const override;
    std::pair<QString, QString> conductivityParameterNameAndUnit() const override;
    std::pair<QString, QString> betaFactorParameterNameAndUnit() const override;
    bool                        isBetaFactorAvailableOnFile() const override;

    double conversionFactorForBetaValues() const;

    QString     generatePropertiesTable() const;
    static void appendTextIfValidValue( QString& body, const QString& title, double value );
    static void appendTextIfValidValue( QString&                                              body,
                                        double                                                value,
                                        RigEnsembleFractureStatisticsCalculator::PropertyType propertyType );

    QString mapUiResultNameToFileResultName( const QString& uiResultName ) const;
    void    convertToUnitSystem( RiaDefines::EclipseUnitSystem neededUnit ) override;

    double formationDip() const;

    static std::pair<QString, QString> widthParameterNameAndUnit( cvf::ref<RigStimPlanFractureDefinition> def );

    bool isValidResult( double value ) const override;

protected:
    void initAfterRead() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    QString     getFileSelectionFilter() const override;
    QStringList conductivityResultNames() const override;

private:
    cvf::ref<RigStimPlanFractureDefinition> m_stimPlanFractureDefinitionData;

    caf::PdmField<QString> m_propertiesTable;

    caf::PdmField<bool> m_showStimPlanMesh_OBSOLETE;
};
