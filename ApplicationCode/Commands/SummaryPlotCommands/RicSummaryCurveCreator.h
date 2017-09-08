/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

//#include "cafAppEnum.h"
//#include "cafPdmPtrArrayField.h"
//
#include "RifEclipseSummaryAddress.h"
//
//#include "RiaDefines.h"
//#include "RimSummaryCurveAppearanceCalculator.h"
//
//class QwtPlot;
//class QwtPlotCurve;
//class RifReaderEclipseSummary;

class RimSummaryCase;

//class RimSummaryCurve;
//class RimSummaryFilter;
//class RiuLineSegmentQwtPlotCurve;
//class RimSummaryCurveAutoName;
//
//
//Q_DECLARE_METATYPE(RifEclipseSummaryAddress);


//==================================================================================================
///  
///  
//==================================================================================================
class RicSummaryCurveCreator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum ItemType
    {
        SUM_FILTER_FIELD,
        SUM_FILTER_AQUIFER,
        SUM_FILTER_NETWORK,
        SUM_FILTER_MISC,
        SUM_FILTER_REGION,
        SUM_FILTER_REGION_2_REGION,
        SUM_FILTER_WELL_GROUP,
        SUM_FILTER_WELL,
        SUM_FILTER_WELL_COMPLETION,
        SUM_FILTER_WELL_COMPLETION_LGR,
        SUM_FILTER_WELL_LGR,
        SUM_FILTER_WELL_SEGMENT,
        SUM_FILTER_BLOCK,
        SUM_FILTER_BLOCK_LGR,
    };

    enum ItemTypeInput
    {
        INPUT_REGION_NUMBER,
        INPUT_REGION2_NUMBER,
        INPUT_WELL_NAME,
        INPUT_WELL_GROUP_NAME,
        INPUT_CELL_IJK,
        INPUT_LGR_NAME,
        INPUT_SEGMENT_NUMBER
    };

private:
    class PdmFieldInfo
    {
    public:
        PdmFieldInfo() :
            m_itemTypeInput((ItemTypeInput)0),
            m_index(-1),
            m_pdmField(nullptr) {}
        PdmFieldInfo(ItemTypeInput itemTypeInput, int index) :
            m_itemTypeInput(itemTypeInput),
            m_index(index),
            m_pdmField(new caf::PdmField<std::vector<QString>>()){}
        virtual ~PdmFieldInfo() { delete m_pdmField; }
    private:
        ItemTypeInput m_itemTypeInput;
        int m_index;
        caf::PdmField<std::vector<QString>> *m_pdmField;
    public:
        ItemTypeInput itemTypeInput() { return m_itemTypeInput; }
        int index() const { return m_index; }
        caf::PdmField<std::vector<QString>>* pdmField() { return m_pdmField; }
    };

    typedef std::pair<ItemTypeInput, PdmFieldInfo*>  PdmFieldInfoElement;
    typedef std::pair<ItemTypeInput, QString>    Selection;

public:
    RicSummaryCurveCreator();
    virtual ~RicSummaryCurveCreator();

private:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddresses();
    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddresses(const std::vector<Selection> &selections, bool ignoreSelections = false);
    RifEclipseSummaryAddress::SummaryVarCategory mapItemType(ItemType itemType);
    QString getItemTypeValueFromAddress(ItemTypeInput itemTypeInput, const RifEclipseSummaryAddress &address);
    PdmFieldInfo* findPdmFieldInfo(const caf::PdmFieldHandle* fieldNeedingOptions);
    PdmFieldInfo* findParentPdmFieldInfo(const PdmFieldInfo *pdmFieldInfo);
    bool isAddressSelected(const RifEclipseSummaryAddress &address, const std::vector<Selection> &selections);
    std::vector<Selection> buildSelectionVector(const caf::PdmFieldHandle *pdmField);
    ItemType findItemTypeFromPdmField(const caf::PdmFieldHandle *pdmField);

private:
    caf::PdmPtrArrayField<RimSummaryCase*>			 m_selectedCases;
    caf::PdmField<caf::AppEnum<ItemType>>			 m_selectedItemType;
    std::map<ItemType, std::vector<PdmFieldInfo*>>   m_itemTypePdmFields;
    caf::PdmField<std::vector<QString>>				 m_selectedVectors;
    caf::PdmField<std::vector<QString>>              m_selectedCurves;
};

