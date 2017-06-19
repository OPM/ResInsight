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

#include "RimGeoMechCase.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifOdbReader.h"

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigFormationNames.h"

#include "RimGeoMechView.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellLogPlotCollection.h"
#include "RimFormationNames.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechPropertyFilter.h"

#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT(RimGeoMechCase, "ResInsightGeoMechCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::RimGeoMechCase(void)
{
    CAF_PDM_InitObject("Geomechanical Case", ":/GeoMechCase48x48.png", "", "");

    CAF_PDM_InitField(&m_caseFileName, "CaseFileName", QString(), "Case File Name", "", "", "");
    m_caseFileName.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitFieldNoDefault(&geoMechViews, "GeoMechViews", "",  "", "", "");
    geoMechViews.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_cohesion, "CaseCohesion", 10.0, "Cohesion", "", "Used to calculate the SE:SFI result", "");
    CAF_PDM_InitField(&m_frictionAngleDeg, "FrctionAngleDeg", 30.0, "Friction Angle [Deg]", "", "Used to calculate the SE:SFI result", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::~RimGeoMechCase(void)
{
    geoMechViews.deleteAllChildObjects();

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (project->mainPlotCollection())
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if (plotCollection)
            {
                plotCollection->removeExtractors(this->geoMechData());
            }
        }
    }

    if (this->geoMechData())
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT(this->geoMechData()->refCount() == 1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::setFileName(const QString& fileName)
{
    m_caseFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::caseFileName() const
{
    return m_caseFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechCase::geoMechData()
{
    return m_geoMechCaseData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigGeoMechCaseData* RimGeoMechCase::geoMechData() const
{
    return m_geoMechCaseData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createAndAddReservoirView()
{
    RimGeoMechView* gmv = new RimGeoMechView();
    size_t i = geoMechViews().size();
    gmv->name = QString("View %1").arg(i + 1);
    
    gmv->setGeoMechCase(this);

    geoMechViews.push_back(gmv);
    return gmv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechCase::openGeoMechCase(std::string* errorMessage)
{
    // If read already, return
    if (this->m_geoMechCaseData.notNull()) return true;

    if (!caf::Utils::fileExists(m_caseFileName()))
    {
        return false;
    }

    m_geoMechCaseData = new RigGeoMechCaseData(m_caseFileName().toStdString());

    bool fileOpenSuccess = m_geoMechCaseData->openAndReadFemParts(errorMessage);
    if (!fileOpenSuccess)
    {
        // If opening failed, release all data
        // Also, several places is checked for this data to validate availability of data
        m_geoMechCaseData = NULL;
    }
    else
    {
        if ( activeFormationNames() )
        {
            m_geoMechCaseData->femPartResults()->setActiveFormationNames(activeFormationNames()->formationNamesData());
        }
        else
        {
            m_geoMechCaseData->femPartResults()->setActiveFormationNames(nullptr);
        }
    }
    return fileOpenSuccess;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    bool foundFile = false;
    std::vector<QString> searchedPaths;

    // Update filename and folder paths when opening project from a different file location
    m_caseFileName = RimTools::relocateFile(m_caseFileName(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
    
#if 0 // Output the search path for debugging
    for (size_t i = 0; i < searchedPaths.size(); ++i)
       qDebug() << searchedPaths[i];
#endif 

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimView*> RimGeoMechCase::views()
{
    std::vector<RimView*> views;
    for (size_t vIdx = 0; vIdx < geoMechViews.size(); ++vIdx)
    {
        views.push_back(geoMechViews[vIdx]);
    }
    return views;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::timeStepDates() const
{
    QStringList timeStrings = timeStepStrings();

    return RimGeoMechCase::dateTimeVectorFromTimeStepStrings(timeStrings);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::initAfterRead()
{
    size_t j;
    for (j = 0; j < geoMechViews().size(); j++)
    {
        RimGeoMechView* riv = geoMechViews()[j];
        CVF_ASSERT(riv);

        riv->setGeoMechCase(this);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimGeoMechCase::timeStepStrings() const
{
    QStringList stringList;

    const RigGeoMechCaseData* rigCaseData = geoMechData();
    if (rigCaseData && rigCaseData->femPartResults())
    {
        std::vector<std::string> stepNames = rigCaseData->femPartResults()->stepNames();
        for (size_t i = 0; i < stepNames.size(); i++)
        {
            stringList += QString::fromStdString(stepNames[i]);
        }
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::timeStepName(int frameIdx) const
{
    const RigGeoMechCaseData* rigCaseData = geoMechData();
    if (rigCaseData && rigCaseData->femPartResults())
    {
       std::vector<std::string> stepNames = rigCaseData->femPartResults()->stepNames();

       return QString::fromStdString(stepNames[frameIdx]);
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechCase::activeCellsBoundingBox() const
{
    return allCellsBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechCase::allCellsBoundingBox() const
{
    if (m_geoMechCaseData.notNull() && m_geoMechCaseData->femParts())
    {
        return m_geoMechCaseData->femParts()->boundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::characteristicCellSize() const
{
    if (geoMechData() && geoMechData()->femParts())
    {
        double cellSize = geoMechData()->femParts()->characteristicElementSize();

        return cellSize;
    }
    
    return 10.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::dateTimeVectorFromTimeStepStrings(const QStringList& timeStepStrings)
{
    std::vector<QDateTime> dates;

    QString dateFormat = "ddMMyyyy";

    for (int i = 0; i < timeStepStrings.size(); i++)
    {
        QString timeStepString = timeStepStrings[i];

        QString dateStr = subStringOfDigits(timeStepString, dateFormat.size());

        QDateTime dateTime = QDateTime::fromString(dateStr, dateFormat);
        if (dateTime.isValid())
        {
            dates.push_back(dateTime);
        }
    }

    return dates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if(changedField == &activeFormationNames)
    {
        updateFormationNamesData();
    }

    if (changedField == &m_cohesion || changedField == &m_frictionAngleDeg)
    {

        RigGeoMechCaseData* rigCaseData = geoMechData();
        if ( rigCaseData && rigCaseData->femPartResults() )
        {
            rigCaseData->femPartResults()->setCalculationParameters(m_cohesion(), cvf::Math::toRadians(m_frictionAngleDeg()));
        }

        std::vector<RimView*> views = this->views();
        for ( RimView* view : views )
        {
            if ( view  ) // Todo: only those using the variable actively
            {
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::updateFormationNamesData()
{
    RigGeoMechCaseData* rigCaseData = geoMechData();
    if(rigCaseData && rigCaseData->femPartResults())
    {
        if(activeFormationNames())
        {
            rigCaseData->femPartResults()->setActiveFormationNames(activeFormationNames()->formationNamesData());
        }
        else
        {
            rigCaseData->femPartResults()->setActiveFormationNames(nullptr);
        }

        std::vector<RimView*> views = this->views();
        for(RimView* view : views)
        {
            RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(view);

            if ( geomView && geomView->isUsingFormationNames() )
            {
                if ( !activeFormationNames() )
                {
                    if ( geomView->cellResult()->resultPositionType() == RIG_FORMATION_NAMES )
                    {
                        geomView->cellResult()->setResultAddress(RigFemResultAddress(RIG_FORMATION_NAMES, "", ""));
                        geomView->cellResult()->updateConnectedEditors();
                    }

                    RimGeoMechPropertyFilterCollection* eclFilColl = geomView->geoMechPropertyFilterCollection();
                    for ( RimGeoMechPropertyFilter* propFilter : eclFilColl->propertyFilters )
                    {
                        if ( propFilter->resultDefinition()->resultPositionType() == RIG_FORMATION_NAMES )
                        {
                            propFilter->resultDefinition()->setResultAddress(RigFemResultAddress(RIG_FORMATION_NAMES, "", ""));
                        }
                    }
                }

                RimGeoMechPropertyFilterCollection* eclFilColl = geomView->geoMechPropertyFilterCollection();
                for ( RimGeoMechPropertyFilter* propFilter : eclFilColl->propertyFilters )
                {
                    if ( propFilter->resultDefinition->resultPositionType() == RIG_FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                geomView->cellResult()->updateConnectedEditors();

                view->scheduleGeometryRegen(PROPERTY_FILTERED);
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::subStringOfDigits(const QString& inputString, int numberOfDigitsToFind)
{
    for (int j = 0; j < inputString.size(); j++)
    {
        if (inputString.at(j).isDigit())
        {
            QString digitString;

            for (int k = 0; k < numberOfDigitsToFind; k++)
            {
                if (j + k < inputString.size() && inputString.at(j + k).isDigit())
                {
                    digitString += inputString.at(j + k);
                }
            }

            if (digitString.size() == numberOfDigitsToFind)
            {
                return digitString;
            }
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
   uiOrdering.add(&caseUserDescription);
   uiOrdering.add(&caseId);
   uiOrdering.add(&m_caseFileName);

   auto group = uiOrdering.addNewGroup("Case Options");
   group->add(&activeFormationNames);
   group->add(&m_cohesion);
   group->add(&m_frictionAngleDeg);

}

