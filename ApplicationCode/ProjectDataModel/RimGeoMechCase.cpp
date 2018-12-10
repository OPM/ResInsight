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
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifOdbReader.h"

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimFormationNames.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"
#include "RimWellLogPlotCollection.h"

#include "cafPdmUiPropertyViewDialog.h"
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
    : m_applyTimeFilter(false)
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
void RimGeoMechCase::reloadDataAndUpdate()
{
    if (this->geoMechData())
    {
        m_geoMechCaseData = nullptr;
        std::string errMsg;
        if (this->openGeoMechCase(&errMsg) == CASE_OPEN_ERROR)
        {
            RiaLogging::error(QString::fromStdString(errMsg));
        }
        for (auto v : geoMechViews())
        {
            v->loadDataAndUpdate();
            v->setCurrentTimeStep(v->currentTimeStep());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createAndAddReservoirView()
{
    RimGeoMechView* gmv = new RimGeoMechView();
    size_t i = geoMechViews().size();
    gmv->setName(QString("View %1").arg(i + 1));
    
    gmv->setGeoMechCase(this);

    geoMechViews.push_back(gmv);
    return gmv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::CaseOpenStatus RimGeoMechCase::openGeoMechCase(std::string* errorMessage)
{
    // If read already, return
    if (this->m_geoMechCaseData.notNull()) return CASE_OPEN_OK;

    if (!caf::Utils::fileExists(m_caseFileName().path()))
    {
        return CASE_OPEN_ERROR;
    }

    cvf::ref<RigGeoMechCaseData> geoMechCaseData = new RigGeoMechCaseData(m_caseFileName().path().toStdString());
    bool fileOpenSuccess = geoMechCaseData->open(errorMessage);
    if (!fileOpenSuccess)
    {
        return CASE_OPEN_ERROR;
    }

    std::vector<std::string> stepNames;
    if (!geoMechCaseData->readTimeSteps(errorMessage, &stepNames))
    {
        return CASE_OPEN_ERROR;
    }

    std::vector<std::pair<QString, QDateTime>> timeSteps;
    for (const std::string& timeStepStringStdString : stepNames)
    {
        QString timeStepString = QString::fromStdString(timeStepStringStdString);
        timeSteps.push_back(std::make_pair(timeStepString, dateTimeFromTimeStepString(timeStepString)));
    }

    m_timeStepFilter->setTimeStepsFromFile(timeSteps);

    if (m_applyTimeFilter)
    {
        m_applyTimeFilter = false; // Clear when we've done this once.

        caf::PdmUiPropertyViewDialog propertyDialog(nullptr, m_timeStepFilter, "Time Step Filter", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        propertyDialog.resize(QSize(400, 400));

        // Push arrow cursor onto the cursor stack so it takes over from the wait cursor.
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        int propertyReturnValue = propertyDialog.exec();
        // Pop arrow cursor off the cursor stack so that the previous (wait) cursor takes over.
        QApplication::restoreOverrideCursor();
        if (propertyReturnValue != QDialog::Accepted)
        {
            return CASE_OPEN_CANCELLED;
        }
        m_timeStepFilter->updateFilteredTimeStepsFromUi();
    }

    // Continue reading the open file
    if (!geoMechCaseData->readFemParts(errorMessage, m_timeStepFilter->filteredTimeSteps()))
    { 
        return CASE_OPEN_ERROR;
    }
    
    if (activeFormationNames())
    {
        geoMechCaseData->femPartResults()->setActiveFormationNames(activeFormationNames()->formationNamesData());
    }
    else
    {
        geoMechCaseData->femPartResults()->setActiveFormationNames(nullptr);
    }

    std::vector<QString> fileNames;
    for (const caf::FilePath& fileName : m_elementPropertyFileNames.v())
    {
        fileNames.push_back(fileName.path());
    }
    geoMechCaseData->femPartResults()->addElementPropertyFiles(fileNames);    

    m_geoMechCaseData = geoMechCaseData;

    return CASE_OPEN_OK;
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

    if (!m_2dIntersectionViewCollection->views().empty())
    {
        uiTreeOrdering.add(&m_2dIntersectionViewCollection);
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::timeStepDates() const
{
    QStringList timeStrings = timeStepStrings();

    return RimGeoMechCase::vectorOfValidDateTimesFromTimeStepStrings(timeStrings);
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
        std::vector<std::string> stepNames = rigCaseData->femPartResults()->filteredStepNames();
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
       std::vector<std::string> stepNames = rigCaseData->femPartResults()->filteredStepNames();
       if (frameIdx < static_cast<int>(stepNames.size()))
       {
           return QString::fromStdString(stepNames[frameIdx]);
       }
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
double RimGeoMechCase::cohesion() const
{
    return m_cohesion;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::frictionAngleDeg() const
{
    return m_frictionAngleDeg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::setApplyTimeFilter(bool applyTimeFilter)
{
    m_applyTimeFilter = applyTimeFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimGeoMechCase::displayModelOffset() const
{
    auto bb = this->allCellsBoundingBox();
    if (bb.isValid())
    {
        return this->allCellsBoundingBox().min();
    }

    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::vectorOfValidDateTimesFromTimeStepStrings(const QStringList& timeStepStrings)
{
    std::vector<QDateTime> dates;

    QString dateFormat = "yyyyMMdd";

    for (const QString& timeStepString : timeStepStrings)
    {
        QDateTime dateTime = dateTimeFromTimeStepString(timeStepString);
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
QDateTime RimGeoMechCase::dateTimeFromTimeStepString(const QString& timeStepString)
{
    QString dateFormat = "yyyyMMdd";
    QString dateStr = subStringOfDigits(timeStepString, dateFormat.size());
    return QDateTime::fromString(dateStr, dateFormat);
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
                geomView->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
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


   caf::PdmUiGroup* timeStepFilterGroup = uiOrdering.addNewGroup("Time Step Filter");
   timeStepFilterGroup->setCollapsedByDefault(true);
   m_timeStepFilter->uiOrdering(uiConfigName, *timeStepFilterGroup);

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

    options = RimCase::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_elementPropertyFileNameIndexUiSelection)
    {
        for (size_t i = 0; i < m_elementPropertyFileNames.v().size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(m_elementPropertyFileNames.v().at(i).path(), (int)i, true, QIcon()));
        }
    }

    return options;
}
