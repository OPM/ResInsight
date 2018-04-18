/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RimTernaryLegendConfig.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"
#include "RimViewLinker.h"

#include "RivTernarySaturationOverlayItem.h"
#include "RivTernaryScalarMapper.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfqtUtils.h"

#include <cmath>
#include "RiaPreferences.h"


CAF_PDM_SOURCE_INIT(RimTernaryLegendConfig, "RimTernaryLegendConfig");

namespace caf {
    template<>
    void AppEnum<RimTernaryLegendConfig::RangeModeType>::setUp()
    {
        addItem(RimTernaryLegendConfig::AUTOMATIC_ALLTIMESTEPS,    "AUTOMATIC_ALLTIMESTEPS",       "Global range");
        addItem(RimTernaryLegendConfig::AUTOMATIC_CURRENT_TIMESTEP,"AUTOMATIC_CURRENT_TIMESTEP",   "Local range");
        addItem(RimTernaryLegendConfig::USER_DEFINED,              "USER_DEFINED_MAX_MIN",         "User defined range");
        setDefault(RimTernaryLegendConfig::AUTOMATIC_ALLTIMESTEPS);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTernaryLegendConfig::RimTernaryLegendConfig() 
{
    CAF_PDM_InitObject("Ternary Legend Definition", ":/Legend.png", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowTernaryLegend", true, "Show Ternary Legend", "", "", "");
    m_showLegend.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&precision, "Precision", 2, "Significant digits", "", "The number of significant digits displayed in the legend numbers","");
    CAF_PDM_InitField(&rangeMode, "RangeType", RangeModeEnum(USER_DEFINED), "Range type", "", "Switches between automatic and user defined range on the legend", "");

    CAF_PDM_InitFieldNoDefault(&applyLocalMinMax,   "m_applyLocalMinMax", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&applyLocalMinMax);
    applyLocalMinMax = false;

    CAF_PDM_InitFieldNoDefault(&applyGlobalMinMax,   "m_applyGlobalMinMax", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&applyGlobalMinMax);
    applyGlobalMinMax = false;

    CAF_PDM_InitFieldNoDefault(&applyFullRangeMinMax,   "m_applyFullRangeMinMax", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&applyFullRangeMinMax);
    applyFullRangeMinMax = false;

    CAF_PDM_InitFieldNoDefault(&ternaryRangeSummary,        "ternaryRangeSummary", "Range summary", "", "", "");
    ternaryRangeSummary.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    ternaryRangeSummary.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);


    CAF_PDM_InitField(&userDefinedMaxValueSoil, "UserDefinedMaxSoil", 1.0, "Max", "", "Min value of the legend", "");
    CAF_PDM_InitField(&userDefinedMinValueSoil, "UserDefinedMinSoil", 0.0, "Min", "", "Max value of the legend", "");

    CAF_PDM_InitField(&userDefinedMaxValueSgas, "UserDefinedMaxSgas", 1.0, "Max", "", "Min value of the legend", "");
    CAF_PDM_InitField(&userDefinedMinValueSgas, "UserDefinedMinSgas", 0.0, "Min", "", "Max value of the legend", "");

    CAF_PDM_InitField(&userDefinedMaxValueSwat, "UserDefinedMaxSwat", 1.0, "Max", "", "Min value of the legend", "");
    CAF_PDM_InitField(&userDefinedMinValueSwat, "UserDefinedMinSwat", 0.0, "Min", "", "Max value of the legend", "");

    m_globalAutoMin.resize(3, 0.0);
    m_globalAutoMax.resize(3, 1.0);
    m_localAutoMin.resize(3, 0.0);
    m_localAutoMax.resize(3, 1.0);

    m_scalarMapper = new RivTernaryScalarMapper(RiaColorTables::undefinedCellColor());

    recreateLegend();
    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTernaryLegendConfig::~RimTernaryLegendConfig()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &applyLocalMinMax)
    {
        userDefinedMaxValueSoil = m_localAutoMax[TERNARY_SOIL_IDX];
        userDefinedMinValueSoil = m_localAutoMin[TERNARY_SOIL_IDX];
        userDefinedMaxValueSgas = m_localAutoMax[TERNARY_SGAS_IDX];
        userDefinedMinValueSgas = m_localAutoMin[TERNARY_SGAS_IDX];
        userDefinedMaxValueSwat = m_localAutoMax[TERNARY_SWAT_IDX];
        userDefinedMinValueSwat = m_localAutoMin[TERNARY_SWAT_IDX];
        
        applyLocalMinMax = false;
    }
    else if (changedField == &applyGlobalMinMax)
    {
        userDefinedMaxValueSoil = m_globalAutoMax[TERNARY_SOIL_IDX];
        userDefinedMinValueSoil = m_globalAutoMin[TERNARY_SOIL_IDX];
        userDefinedMaxValueSgas = m_globalAutoMax[TERNARY_SGAS_IDX];
        userDefinedMinValueSgas = m_globalAutoMin[TERNARY_SGAS_IDX];
        userDefinedMaxValueSwat = m_globalAutoMax[TERNARY_SWAT_IDX];
        userDefinedMinValueSwat = m_globalAutoMin[TERNARY_SWAT_IDX];

        applyGlobalMinMax = false;
    }
    else if (changedField == &applyFullRangeMinMax)
    {
        userDefinedMaxValueSoil = 1.0;
        userDefinedMinValueSoil = 0.0;
        userDefinedMaxValueSgas = 1.0;
        userDefinedMinValueSgas = 0.0;
        userDefinedMaxValueSwat = 1.0;
        userDefinedMinValueSwat = 0.0;

        applyFullRangeMinMax = false;
    }

    updateLabelText();
    updateLegend();

    RimGridView* view = nullptr;
    this->firstAncestorOrThisOfType(view);

    if (view)
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->updateCellResult();
        }
        
        view->updateCurrentTimeStepAndRedraw();
        view->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::updateLegend()
{
    double soilLower = 0.0;
    double soilUpper = 1.0;
    double sgasLower = 0.0;
    double sgasUpper = 1.0;
    double swatLower = 0.0;
    double swatUpper = 1.0;

    ternaryRanges(soilLower, soilUpper, sgasLower, sgasUpper, swatLower, swatUpper);
    m_scalarMapper->setTernaryRanges(soilLower, soilUpper, sgasLower, sgasUpper);

    cvf::String soilRange;
    cvf::String sgasRange;
    cvf::String swatRange;

    int numberPrecision = 1;
    {
        QString tmpString = QString::number(soilLower, 'g', numberPrecision) + " - " + QString::number(soilUpper, 'g', numberPrecision);
        soilRange = cvfqt::Utils::toString(tmpString);
    }

    {
        QString tmpString = QString::number(sgasLower, 'g', numberPrecision) + " - " + QString::number(sgasUpper, 'g', numberPrecision);
        sgasRange = cvfqt::Utils::toString(tmpString);
    }

    {
        QString tmpString = QString::number(swatLower, 'g', numberPrecision) + " - " + QString::number(swatUpper, 'g', numberPrecision);
        swatRange = cvfqt::Utils::toString(tmpString);
    }

    if (!m_legend.isNull())
    {
        m_legend->setRangeText(soilRange, sgasRange, swatRange);

        RiaApplication* app = RiaApplication::instance();
        RiaPreferences* preferences = app->preferences();
        m_legend->enableBackground(preferences->showLegendBackground());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::setAutomaticRanges(TernaryArrayIndex ternaryIndex, double globalMin, double globalMax, double localMin, double localMax)
{
    double candidateGlobalAutoMin = roundToNumSignificantDigits(globalMin, precision);
    double candidateGlobalAutoMax = roundToNumSignificantDigits(globalMax, precision);

    double candidateLocalAutoMin = roundToNumSignificantDigits(localMin, precision);
    double candidateLocalAutoMax = roundToNumSignificantDigits(localMax, precision);

    m_globalAutoMin[ternaryIndex] = candidateGlobalAutoMin;
    m_globalAutoMax[ternaryIndex] = candidateGlobalAutoMax;
    m_localAutoMin[ternaryIndex] = candidateLocalAutoMin;
    m_localAutoMax[ternaryIndex] = candidateLocalAutoMax;

    updateLabelText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::recreateLegend()
{
    // Due to possible visualization bug, we need to recreate the legend if the last viewer 
    // has been removed, (and thus the opengl resources has been deleted) The text in 
    // the legend disappeared because of this, so workaround: recreate the legend when needed:

    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_legend = new RivTernarySaturationOverlayItem(standardFont);
    m_legend->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_LEFT);

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::setUiValuesFromLegendConfig(const RimTernaryLegendConfig* otherLegendConfig)
{
    QString serializedObjectString = otherLegendConfig->writeObjectToXmlString();
    this->readObjectFromXmlString(serializedObjectString, caf::PdmDefaultObjectFactory::instance());
    this->updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// Rounding the double value to given number of significant digits
//--------------------------------------------------------------------------------------------------
double RimTernaryLegendConfig::roundToNumSignificantDigits(double domainValue, double numSignificantDigits)
{
    double absDomainValue = cvf::Math::abs(domainValue);
    if (absDomainValue == 0.0)
    {
        return 0.0;
    }

    double logDecValue = log10(absDomainValue);
    logDecValue = cvf::Math::ceil(logDecValue);

    double factor = pow(10.0, numSignificantDigits - logDecValue);

    double tmp = domainValue * factor;
    double integerPart;
    double fraction = modf(tmp, &integerPart);

    if (cvf::Math::abs(fraction)>= 0.5) (integerPart >= 0) ? integerPart++: integerPart-- ;

    double newDomainValue = integerPart / factor;

    return newDomainValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiOrdering* formatGr = uiOrdering.addNewGroup("Format");
    formatGr->add(&precision);
    formatGr->add(&rangeMode);

    if (rangeMode == USER_DEFINED)
    {
        caf::PdmUiOrdering* ternaryGroupContainer = uiOrdering.addNewGroup("Ternary ");
        {
            caf::PdmUiOrdering* ternaryGroup = ternaryGroupContainer->addNewGroup("SGAS");
            ternaryGroup->add(&userDefinedMinValueSgas);
            ternaryGroup->add(&userDefinedMaxValueSgas);
        }

        {
            caf::PdmUiOrdering* ternaryGroup = ternaryGroupContainer->addNewGroup("SWAT");
            ternaryGroup->add(&userDefinedMinValueSwat);
            ternaryGroup->add(&userDefinedMaxValueSwat);
        }

        {
            caf::PdmUiOrdering* ternaryGroup = ternaryGroupContainer->addNewGroup("SOIL");
            ternaryGroup->add(&userDefinedMinValueSoil);
            ternaryGroup->add(&userDefinedMaxValueSoil);
        }

        ternaryGroupContainer->add(&applyLocalMinMax);
        ternaryGroupContainer->add(&applyGlobalMinMax);
        ternaryGroupContainer->add(&applyFullRangeMinMax);
    }
    else
    {
        caf::PdmUiOrdering* group = uiOrdering.addNewGroup("Summary");
        group->add(&ternaryRangeSummary);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTernaryLegendConfig::showLegend() const
{
    return m_showLegend;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::setTitle(const QString& title)
{
    m_legend->setTitle(cvfqt::Utils::toString(title));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&applyLocalMinMax == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply local min/max values";
        }
    }
    else if (&applyGlobalMinMax == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply global min/max values";
        }
    }
    else if (&applyFullRangeMinMax == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply full range";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::ternaryRanges(double& soilLower, double& soilUpper, double& sgasLower, double& sgasUpper, double& swatLower, double& swatUpper) const
{
    if (rangeMode() == AUTOMATIC_CURRENT_TIMESTEP)
    {
        soilLower = m_localAutoMin[TERNARY_SOIL_IDX];
        soilUpper = m_localAutoMax[TERNARY_SOIL_IDX];
        sgasLower = m_localAutoMin[TERNARY_SGAS_IDX];
        sgasUpper = m_localAutoMax[TERNARY_SGAS_IDX];
        swatLower = m_localAutoMin[TERNARY_SWAT_IDX];
        swatUpper = m_localAutoMax[TERNARY_SWAT_IDX];
    }
    else if (rangeMode() == AUTOMATIC_ALLTIMESTEPS)
    {
        soilLower = m_globalAutoMin[TERNARY_SOIL_IDX];
        soilUpper = m_globalAutoMax[TERNARY_SOIL_IDX];
        sgasLower = m_globalAutoMin[TERNARY_SGAS_IDX];
        sgasUpper = m_globalAutoMax[TERNARY_SGAS_IDX];
        swatLower = m_globalAutoMin[TERNARY_SWAT_IDX];
        swatUpper = m_globalAutoMax[TERNARY_SWAT_IDX];
    }
    else
    {
        soilLower = userDefinedMinValueSoil;
        soilUpper = userDefinedMaxValueSoil;
        sgasLower = userDefinedMinValueSgas;
        sgasUpper = userDefinedMaxValueSgas;
        swatLower = userDefinedMinValueSwat;
        swatUpper = userDefinedMaxValueSwat;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTernaryLegendConfig::updateLabelText()
{
    {
        userDefinedMinValueSoil.uiCapability()->setUiName("Min");
        userDefinedMaxValueSoil.uiCapability()->setUiName("Max");

        if (m_globalAutoMin[TERNARY_SOIL_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMinValueSoil.uiCapability()->setUiName(QString("Min ") + "(" + QString::number(m_globalAutoMin[TERNARY_SOIL_IDX], 'g', precision) + ")");
        }

        if (m_globalAutoMax[TERNARY_SOIL_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMaxValueSoil.uiCapability()->setUiName(QString("Max ") + "(" + QString::number(m_globalAutoMax[TERNARY_SOIL_IDX], 'g', precision) + ")");
        }
    }

    {
        userDefinedMinValueSgas.uiCapability()->setUiName("Min");
        userDefinedMaxValueSgas.uiCapability()->setUiName("Max");

        if (m_globalAutoMin[TERNARY_SGAS_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMinValueSgas.uiCapability()->setUiName(QString("Min ") + "(" + QString::number(m_globalAutoMin[TERNARY_SGAS_IDX], 'g', precision) + ")");
        }

        if (m_globalAutoMax[TERNARY_SGAS_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMaxValueSgas.uiCapability()->setUiName(QString("Max ") + "(" + QString::number(m_globalAutoMax[TERNARY_SGAS_IDX], 'g', precision) + ")");
        }
    }

    {
        userDefinedMinValueSwat.uiCapability()->setUiName("Min");
        userDefinedMaxValueSwat.uiCapability()->setUiName("Max");

        if (m_globalAutoMin[TERNARY_SWAT_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMinValueSwat.uiCapability()->setUiName(QString("Min ") + "(" + QString::number(m_globalAutoMin[TERNARY_SWAT_IDX], 'g', precision) + ")");
        }

        if (m_globalAutoMax[TERNARY_SWAT_IDX] != cvf::UNDEFINED_DOUBLE )
        {
            userDefinedMaxValueSwat.uiCapability()->setUiName(QString("Max ") + "(" + QString::number(m_globalAutoMax[TERNARY_SWAT_IDX], 'g', precision) + ")");
        }
    }

    if (rangeMode == AUTOMATIC_ALLTIMESTEPS)
    {
        QString tmpString;
        tmpString  = QString("SOIL : ") + QString::number(m_globalAutoMin[TERNARY_SOIL_IDX], 'g', precision) + " - " + QString::number(m_globalAutoMax[TERNARY_SOIL_IDX], 'g', precision) + "\n";
        tmpString += QString("SGAS : ") + QString::number(m_globalAutoMin[TERNARY_SGAS_IDX], 'g', precision) + " - " + QString::number(m_globalAutoMax[TERNARY_SGAS_IDX], 'g', precision) + "\n";
        tmpString += QString("SWAT : ") + QString::number(m_globalAutoMin[TERNARY_SWAT_IDX], 'g', precision) + " - " + QString::number(m_globalAutoMax[TERNARY_SWAT_IDX], 'g', precision) + "\n";

        ternaryRangeSummary = tmpString;
    }
    else if (rangeMode == AUTOMATIC_CURRENT_TIMESTEP)
    {
        QString tmpString;
        tmpString  = QString("SOIL : ") + QString::number(m_localAutoMin[TERNARY_SOIL_IDX], 'g', precision) + " - " + QString::number(m_localAutoMax[TERNARY_SOIL_IDX], 'g', precision) + "\n";
        tmpString += QString("SGAS : ") + QString::number(m_localAutoMin[TERNARY_SGAS_IDX], 'g', precision) + " - " + QString::number(m_localAutoMax[TERNARY_SGAS_IDX], 'g', precision) + "\n";
        tmpString += QString("SWAT : ") + QString::number(m_localAutoMin[TERNARY_SWAT_IDX], 'g', precision) + " - " + QString::number(m_localAutoMax[TERNARY_SWAT_IDX], 'g', precision) + "\n";

        ternaryRangeSummary = tmpString;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RivTernaryScalarMapper* RimTernaryLegendConfig::scalarMapper() const
{
    return m_scalarMapper.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const caf::TitledOverlayFrame* RimTernaryLegendConfig::titledOverlayFrame() const
{
    return m_legend.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::TitledOverlayFrame* RimTernaryLegendConfig::titledOverlayFrame()
{
    return m_legend.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTernaryLegendConfig::objectToggleField()
{
    return &m_showLegend;
}
