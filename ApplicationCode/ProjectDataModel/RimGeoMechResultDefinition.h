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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include "RigFemResultPosEnum.h"
#include "RigFemResultAddress.h"
#include "RimFemResultObserver.h"

class RimGeoMechView;
class RimGeoMechPropertyFilter;
class RifGeoMechReaderInterface;
class RigGeoMechCaseData;
class RimGeoMechCase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechResultDefinition : public RimFemResultObserver
{
     CAF_PDM_HEADER_INIT;
public:
    RimGeoMechResultDefinition(void);
    ~RimGeoMechResultDefinition(void) override;

    void                                              setGeoMechCase(RimGeoMechCase* geomCase);

    RigGeoMechCaseData*                               ownerCaseData();
    bool                                              hasResult(); 
    void                                              loadResult();
    void                                              setAddWellPathDerivedResults(bool addWellPathDerivedResults);

    RigFemResultAddress                               resultAddress() const;
    std::vector<RigFemResultAddress>          observedResults() const override;

    RigFemResultPosEnum                               resultPositionType() const;
    QString                                           resultFieldName() const;
    QString                                           resultComponentName() const;
    void                                              setResultAddress(const RigFemResultAddress& resultAddress);

    QString                                           resultFieldUiName();
    QString                                           resultComponentUiName();

    bool                                              hasCategoryResult()  { return m_resultPositionType() == RIG_FORMATION_NAMES; }

protected:
    virtual void                                      updateLegendCategorySettings() {};
    void                                      defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:

    // Overridden PDM methods

    QList<caf::PdmOptionItemInfo>             calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                            bool * useOptionsOnly) override;
    void                                      fieldChangedByUi(const caf::PdmFieldHandle* changedField, 
                                                                       const QVariant& oldValue, 
                                                                       const QVariant& newValue) override;
    void                                      initAfterRead() override;

    // Metadata and option build tools

    std::map<std::string, std::vector<std::string> >  getResultMetaDataForUIFieldSetting();
    static void                                       getUiAndResultVariableStringList(QStringList* uiNames, 
                                                                                       QStringList* variableNames, 
                                                                                       const std::map<std::string, std::vector<std::string> >& fieldCompNames, 
                                                                                       bool isTimeLapseResultList, 
                                                                                       int baseFrameIdx);
    static QString                                    composeFieldCompString(const QString& resultFieldName, 
                                                                             const QString& resultComponentName);

    static QString                                    convertToUiResultFieldName(QString resultFieldName,
                                                                                 bool isTimeLapseResultList,
                                                                                 int baseFrameIdx);
    static QString                                    convertToUIComponentName(QString resultComponentName,
                                                                               bool isTimeLapseResultList,
                                                                               int baseFrameIdx);

    // Data Fields

    caf::PdmField<caf::AppEnum<RigFemResultPosEnum> > m_resultPositionType;
    caf::PdmField<QString>                            m_resultFieldName;
    caf::PdmField<QString>                            m_resultComponentName;
    caf::PdmField<bool>                               m_isTimeLapseResult;
    caf::PdmField<int>                                m_timeLapseBaseTimestep;
    caf::PdmField<int>                                m_compactionRefLayer;

    // UI Fields only

    friend class RimGeoMechPropertyFilter;  // Property filter needs the ui fields
    friend class RimWellLogExtractionCurve; // Curve needs the ui fields
    friend class RimGeoMechCellColors;      // Needs the ui fields
 
    caf::PdmField<caf::AppEnum<RigFemResultPosEnum> > m_resultPositionTypeUiField;
    caf::PdmField<QString>                            m_resultVariableUiField;
    caf::PdmField<bool>                               m_isTimeLapseResultUiField;
    caf::PdmField<int>                                m_timeLapseBaseTimestepUiField;
    caf::PdmField<int>                                m_compactionRefLayerUiField;
    caf::PdmPointer<RimGeoMechCase>                   m_geomCase;

    bool                                              m_isChangedByField;
    bool                                              m_addWellPathDerivedResults;
};
