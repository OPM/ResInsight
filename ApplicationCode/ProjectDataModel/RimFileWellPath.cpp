#include "RimFileWellPath.h"
#include "cafUtils.h"
#include "RifWellPathImporter.h"
#include "RimTools.h"
#include "QFileInfo"
#include "QDir"


CAF_PDM_SOURCE_INIT(RimFileWellPath, "WellPath");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFileWellPath::RimFileWellPath()
{
    CAF_PDM_InitFieldNoDefault(&id,                 "WellPathId",                           "Id", "", "", "");
    id.uiCapability()->setUiReadOnly(true);
    id.xmlCapability()->setIOWritable(false);
    id.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&sourceSystem,       "SourceSystem",                         "Source System", "", "", "");
    sourceSystem.uiCapability()->setUiReadOnly(true);
    sourceSystem.xmlCapability()->setIOWritable(false);
    sourceSystem.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&utmZone,            "UTMZone",                              "UTM Zone", "", "", "");
    utmZone.uiCapability()->setUiReadOnly(true);
    utmZone.xmlCapability()->setIOWritable(false);
    utmZone.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&updateDate,         "WellPathUpdateDate",                   "Update Date", "", "", "");
    updateDate.uiCapability()->setUiReadOnly(true);
    updateDate.xmlCapability()->setIOWritable(false);
    updateDate.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&updateUser,         "WellPathUpdateUser",                   "Update User", "", "", "");
    updateUser.uiCapability()->setUiReadOnly(true);
    updateUser.xmlCapability()->setIOWritable(false);
    updateUser.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_surveyType,       "WellPathSurveyType",                   "Survey Type", "", "", "");
    m_surveyType.uiCapability()->setUiReadOnly(true);
    m_surveyType.xmlCapability()->setIOWritable(false);
    m_surveyType.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_filepath,                    "WellPathFilepath",     QString(""),    "File Path", "", "", "");
    m_filepath.uiCapability()->setUiReadOnly(true);
    CAF_PDM_InitField(&m_wellPathIndexInFile,         "WellPathNumberInFile",     -1,    "Well Number in File", "", "", "");
    m_wellPathIndexInFile.uiCapability()->setUiReadOnly(true);


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFileWellPath::~RimFileWellPath()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFileWellPath::filepath() const
{
    return m_filepath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::setFilepath(const QString& path)
{
    m_filepath = path;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimFileWellPath::wellPathIndexInFile() const
{
    return m_wellPathIndexInFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::setWellPathIndexInFile(int index)
{
    m_wellPathIndexInFile = index ;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::setSurveyType(QString surveyType) 
{ 
    m_surveyType = surveyType; 
    if (m_surveyType == "PLAN")
        setWellPathColor(cvf::Color3f(0.999f, 0.333f, 0.0f));
    else if (m_surveyType == "PROTOTYPE")
        setWellPathColor(cvf::Color3f(0.0f, 0.333f, 0.999f));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimWellPath::defineUiOrdering(uiConfigName, uiOrdering);
    
    caf::PdmUiGroup* fileInfoGroup =    uiOrdering.createGroupBeforeGroup("Simulation Well", "File");
    fileInfoGroup->add(&m_filepath);
    fileInfoGroup->add(&m_wellPathIndexInFile);

    if ( !id().isEmpty() )           uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &id);
    if ( !sourceSystem().isEmpty() ) uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &sourceSystem);
    if ( !utmZone().isEmpty() )      uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &utmZone);
    if ( !updateDate().isEmpty() )   uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &updateDate);
    if ( !updateUser().isEmpty() )   uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &updateUser);
    if ( !m_surveyType().isEmpty() ) uiOrdering.insertBeforeItem(m_datumElevation.uiCapability(), &m_surveyType);
}

//--------------------------------------------------------------------------------------------------
/// Read JSON or ascii file containing well path data
//--------------------------------------------------------------------------------------------------
bool RimFileWellPath::readWellPathFile(QString* errorMessage, RifWellPathImporter* wellPathImporter)
{
    if (caf::Utils::fileExists(m_filepath()))
    {
        RifWellPathImporter::WellData wellData = wellPathImporter->readWellData(m_filepath(), m_wellPathIndexInFile());
        RifWellPathImporter::WellMetaData wellMetaData = wellPathImporter->readWellMetaData(m_filepath(), m_wellPathIndexInFile());
        // General well info

        setName(wellData.m_name);
        id = wellMetaData.m_id;
        sourceSystem = wellMetaData.m_sourceSystem;
        utmZone = wellMetaData.m_utmZone;
        updateUser = wellMetaData.m_updateUser;
        setSurveyType(wellMetaData.m_surveyType);
        updateDate = wellMetaData.m_updateDate.toString("d MMMM yyyy");

        setWellPathGeometry(wellData.m_wellPathGeometry.p());
        return true;
    }
    else
    {
        if (errorMessage) (*errorMessage) = "Could not find the well path file: " + m_filepath();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFileWellPath::getCacheDirectoryPath()
{
    QString cacheDirPath = RimTools::getCacheRootDirectoryPathFromProject();
    cacheDirPath += "_wellpaths";
    return cacheDirPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFileWellPath::getCacheFileName()
{
    if (m_filepath().isEmpty())
    {
        return "";
    }

    QString cacheFileName;

    // Make the path correct related to the possibly new project filename
    QString newCacheDirPath = getCacheDirectoryPath();
    QFileInfo oldCacheFile(m_filepath);


    cacheFileName = newCacheDirPath + "/" + oldCacheFile.fileName();

    return cacheFileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::setupBeforeSave()
{
    // SSIHUB is the only source for populating Id, use text in this field to decide if the cache file must be copied to new project cache location
    if (!isStoredInCache())
    {
        return;
    }

    if (m_filepath().isEmpty())
    {
        return;
    }

    QDir::root().mkpath(getCacheDirectoryPath());

    QString newCacheFileName = getCacheFileName();

    // Use QFileInfo to get same string representation to avoid issues with mix of forward and backward slashes
    QFileInfo prevFileInfo(m_filepath);
    QFileInfo currentFileInfo(newCacheFileName);

    if (prevFileInfo.absoluteFilePath().compare(currentFileInfo.absoluteFilePath()) != 0)
    {
        QFile::copy(m_filepath, newCacheFileName);

        m_filepath = newCacheFileName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFileWellPath::isStoredInCache()
{
    // SSIHUB is the only source for populating Id, use text in this field to decide if the cache file must be copied to new project cache location
    return !id().isEmpty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileWellPath::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    RimWellPath::updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);

    if (isStoredInCache())
    {
        QString newCacheFileName = getCacheFileName();

        if (caf::Utils::fileExists(newCacheFileName))
        {
            m_filepath = newCacheFileName;
        }
    }
    else
    {
        m_filepath = RimTools::relocateFile(m_filepath(), newProjectPath, oldProjectPath, nullptr, nullptr);
    }
}

