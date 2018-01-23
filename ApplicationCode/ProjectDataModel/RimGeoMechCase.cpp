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

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include <QFile>
#include <QIcon>

CAF_PDM_SOURCE_INIT(RimGeoMechCase, "ResInsightGeoMechCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::RimGeoMechCase(void)
{
    CAF_PDM_InitObject("Geomechanical Case", ":/GeoMechCase48x48.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_caseFileName, "CaseFileName", "Case File Name", "", "", "");
    m_caseFileName.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitFieldNoDefault(&geoMechViews, "GeoMechViews", "",  "", "", "");
    geoMechViews.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_cohesion, "CaseCohesion", 10.0, "Cohesion", "", "Used to calculate the SE:SFI result", "");
    CAF_PDM_InitField(&m_frictionAngleDeg, "FrctionAngleDeg", 30.0, "Friction Angle [Deg]", "", "Used to calculate the SE:SFI result", "");

    CAF_PDM_InitFieldNoDefault(&m_elementPropertyFileNames, "ElementPropertyFileNames", "Element Property Files", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_elementPropertyFileNameIndexUiSelection, "ElementPropertyFileNameIndexUiSelection", "", "", "", "");
    m_elementPropertyFileNameIndexUiSelection.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_closeElementPropertyFileCommand, "closeElementPropertyFileCommad", false, "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_closeElementPropertyFileCommand);

    CAF_PDM_InitField(&m_reloadElementPropertyFileCommand, "reloadElementPropertyFileCommand", false, "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_reloadElementPropertyFileCommand);
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
    m_caseFileName.v().setPath(fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::caseFileName() const
{
    return m_caseFileName().path();
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

    if (!caf::Utils::fileExists(m_caseFileName().path()))
    {
        return false;
    }

    m_geoMechCaseData = new RigGeoMechCaseData(m_caseFileName().path().toStdString());

    bool fileOpenSuccess = m_geoMechCaseData->openAndReadFemParts(errorMessage);
    if (!fileOpenSuccess)
    {
        // If opening failed, release all data
        // Also, several places is checked for this data to validate availability of data
        m_geoMechCaseData = nullptr;
        return false;
    }
    
    if (activeFormationNames())
    {
        m_geoMechCaseData->femPartResults()->setActiveFormationNames(activeFormationNames()->formationNamesData());
    }
    else
    {
        m_geoMechCaseData->femPartResults()->setActiveFormationNames(nullptr);
    }

    if (m_geoMechCaseData.notNull())
    {
        std::vector<QString> fileNames;
        for (const caf::FilePath& fileName : m_elementPropertyFileNames.v())
        {
            fileNames.push_back(fileName.path());
        }
        geoMechData()->femPartResults()->addElementPropertyFiles(fileNames);
    }

    return fileOpenSuccess;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    //No longer in use. Filepaths are now of type caf::FilePath, and updated in RimProject on load.
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimGeoMechCase::allSpecialViews() const
{
    std::vector<Rim3dView*> views;
    for (size_t vIdx = 0; vIdx < geoMechViews.size(); ++vIdx)
    {
        views.push_back(geoMechViews[vIdx]);
    }
    return views;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    std::vector<PdmObjectHandle*> children;
    geoMechViews.childObjects(&children);

    for ( auto child : children ) uiTreeOrdering.add(child);

    uiTreeOrdering.add(&m_2dIntersectionViewCollection);
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
void RimGeoMechCase::setFormationNames(RimFormationNames* formationNames)
{
    activeFormationNames = formationNames;
    if (m_geoMechCaseData.notNull() && formationNames != nullptr)
    {
        m_geoMechCaseData->femPartResults()->setActiveFormationNames(formationNames->formationNamesData());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::addElementPropertyFiles(const std::vector<caf::FilePath>& fileNames)
{
    std::vector<QString> newFileNames;

    for (const caf::FilePath& newFileNameToPossiblyAdd : fileNames)
    {
        bool fileAlreadyAdded = false;

        for (const caf::FilePath& existingFileName : m_elementPropertyFileNames())
        {
            if (existingFileName == newFileNameToPossiblyAdd)
            {
                fileAlreadyAdded = true;
                break;
            }
        }
        if (!fileAlreadyAdded)
        {
            newFileNames.push_back(newFileNameToPossiblyAdd.path());
            m_elementPropertyFileNames.v().push_back(newFileNameToPossiblyAdd);
        }
    }
    
    this->updateConnectedEditors();
    
    if (m_geoMechCaseData.notNull())
    {
        geoMechData()->femPartResults()->addElementPropertyFiles(newFileNames);
    }
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

        std::vector<Rim3dView*> views = this->views();
        for ( Rim3dView* view : views )
        {
            if ( view  ) // Todo: only those using the variable actively
            {
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
    else if (changedField == &m_reloadElementPropertyFileCommand)
    {
        m_reloadElementPropertyFileCommand = false;
        reloadSelectedElementPropertyFiles();
        updateConnectedEditors();
    }
    else if (changedField == &m_closeElementPropertyFileCommand)
    {
        m_closeElementPropertyFileCommand = false;
        closeSelectedElementPropertyFiles();
        updateConnectedEditors();
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

        std::vector<Rim3dView*> views = this->views();
        for(Rim3dView* view : views)
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

struct descendingComparator
{
    template<class T>
    bool operator()(T const &a, T const &b) const { return a > b; }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::closeSelectedElementPropertyFiles()
{
    std::sort(m_elementPropertyFileNameIndexUiSelection.v().begin(), m_elementPropertyFileNameIndexUiSelection.v().end(), descendingComparator());

    std::vector<QString> filesToClose;

    for (size_t idx : m_elementPropertyFileNameIndexUiSelection.v())
    {
        filesToClose.push_back(m_elementPropertyFileNames.v().at(idx).path());
        m_elementPropertyFileNames.v().erase(m_elementPropertyFileNames.v().begin() + idx);
    }

    m_elementPropertyFileNameIndexUiSelection.v().clear();

    std::vector<RigFemResultAddress> addressesToDelete;

    if (m_geoMechCaseData.notNull())
    {
         addressesToDelete = geoMechData()->femPartResults()->removeElementPropertyFiles(filesToClose);
    }

    for (RimGeoMechView* view : geoMechViews())
    {
        for (RigFemResultAddress address : addressesToDelete)
        {
            if (address == view->cellResultResultDefinition()->resultAddress())
            {
                view->cellResult()->setResultAddress(RigFemResultAddress());
            }

            for (RimGeoMechPropertyFilter* propertyFilter : view->geoMechPropertyFilterCollection()->propertyFilters())
            {
                if (address == propertyFilter->resultDefinition->resultAddress())
                {
                    propertyFilter->resultDefinition->setResultAddress(RigFemResultAddress());
                }
            }
        }

        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::reloadSelectedElementPropertyFiles()
{
    std::vector<QString> filesToReload;

    for (size_t idx : m_elementPropertyFileNameIndexUiSelection.v())
    {
        filesToReload.push_back(m_elementPropertyFileNames.v().at(idx).path());
    }

    m_elementPropertyFileNameIndexUiSelection.v().clear();

    if (m_geoMechCaseData.notNull())
    {
        geoMechData()->femPartResults()->removeElementPropertyFiles(filesToReload);
        geoMechData()->femPartResults()->addElementPropertyFiles(filesToReload);
    }

    for (RimGeoMechView* view : geoMechViews())
    {
        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
   uiOrdering.add(&caseUserDescription);
   uiOrdering.add(&caseId);
   uiOrdering.add(&m_caseFileName);

   caf::PdmUiGroup* caseGroup = uiOrdering.addNewGroup("Case Options");
   caseGroup->add(&activeFormationNames);
   caseGroup->add(&m_cohesion);
   caseGroup->add(&m_frictionAngleDeg);

   caf::PdmUiGroup* elmPropGroup = uiOrdering.addNewGroup("Element Properties");
   elmPropGroup->add(&m_elementPropertyFileNameIndexUiSelection);
   elmPropGroup->add(&m_reloadElementPropertyFileCommand);
   elmPropGroup->add(&m_closeElementPropertyFileCommand);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_reloadElementPropertyFileCommand)
    {
        dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute)->m_buttonText = "Reload Case(s)";
    }
    if (field == &m_closeElementPropertyFileCommand)
    {
        dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute)->m_buttonText = "Close Case(s)";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechCase::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_elementPropertyFileNameIndexUiSelection)
    {
        for (size_t i = 0; i < m_elementPropertyFileNames.v().size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(m_elementPropertyFileNames.v().at(i).path(), (int)i, true, QIcon()));
        }
    }

    return options;
}
