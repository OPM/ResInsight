#include "RimOsduWellPath.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimOsduWellPath, "OsduWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellPath::RimOsduWellPath()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Osdu Well Path", ":/Well.svg", "", "", "OsduWellPath", "Well Path Loaded From Osdu" );

    CAF_PDM_InitFieldNoDefault( &m_wellId, "WellId", "Well Id" );
    m_wellId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellboreId, "WellboreId", "Wellbore Id" );
    m_wellboreId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellboreTrajectoryId, "WellboreTrajectoryId", "Wellbore Trajectory Id" );
    m_wellboreTrajectoryId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_fileId, "FileId", "File Id" );
    m_fileId.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOsduWellPath::~RimOsduWellPath()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPath::setWellId( const QString& wellId )
{
    m_wellId = wellId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellPath::wellId() const
{
    return m_wellId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPath::setWellboreId( const QString& wellboreId )
{
    m_wellboreId = wellboreId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellPath::wellboreId() const
{
    return m_wellboreId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPath::setWellboreTrajectoryId( const QString& wellboreTrajectoryId )
{
    m_wellboreTrajectoryId = wellboreTrajectoryId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellPath::wellboreTrajectoryId() const
{
    return m_wellboreTrajectoryId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPath::setFileId( const QString& fileId )
{
    m_fileId = fileId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOsduWellPath::fileId() const
{
    return m_fileId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOsduWellPath::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* osduGroup = uiOrdering.addNewGroup( "OSDU" );
    osduGroup->add( &m_wellId );
    osduGroup->add( &m_wellboreId );
    osduGroup->add( &m_wellboreTrajectoryId );
    osduGroup->add( &m_fileId );

    RimWellPath::defineUiOrdering( uiConfigName, uiOrdering );
}