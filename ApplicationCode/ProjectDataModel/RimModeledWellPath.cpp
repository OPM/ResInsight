#include "RimModeledWellPath.h"
#include "cafPdmUiTableViewEditor.h"
#include "RimWellPathTarget.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT(RimModeledWellPath, "ModeledWellPath");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::RimModeledWellPath()
{
    CAF_PDM_InitObject("Modeled WellPath", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_geometryDefinition, "WellPathGeometryDef", "Trajectory", "", "", "");
    //m_geometryDefinition.uiCapability()->setUiHidden(true);
    m_geometryDefinition = new RimWellPathGeometryDef;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::~RimModeledWellPath()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.add(m_geometryDefinition());
    RimWellPath::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);
}

namespace caf
{
template<>
void caf::AppEnum< RimWellPathGeometryDef::WellStartType >::setUp()
{
    addItem(RimWellPathGeometryDef::START_AT_FIRST_TARGET, "START_AT_FIRST_TARGET",   "Start at First Target");
    addItem(RimWellPathGeometryDef::START_AT_SURFACE,      "START_AT_SURFACE",         "Start at Surface");
    addItem(RimWellPathGeometryDef::START_FROM_OTHER_WELL, "START_FROM_OTHER_WELL",   "Branch");
    addItem(RimWellPathGeometryDef::START_AT_AUTO_SURFACE, "START_AT_AUTO_SURFACE",   "Auto Surface");

    setDefault(RimWellPathGeometryDef::START_AT_FIRST_TARGET);
}
}

CAF_PDM_SOURCE_INIT(RimWellPathGeometryDef, "WellPathGeometryDef");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::RimWellPathGeometryDef()
{
    CAF_PDM_InitObject("Trajectory", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellStartType, "WellStartType", "Start Type", "", "", "");
    CAF_PDM_InitField(&m_startPos, "StartPos", cvf::Vec3d(0,0,0), "UTM Start Pos", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_parentWell, "ParentWell", "Parent Well", "", "", "");
    CAF_PDM_InitField(&m_kickoffDepthOrMD, "KickoffDepthOrMD", 100.0, "Kickoff Depth", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellTargets, "WellPathTargets", "Well Targets", "", "", "");
    m_wellTargets.uiCapability()->setUiEditorTypeName(caf::PdmUiTableViewEditor::uiEditorTypeName());
    //m_wellTargets.uiCapability()->setUiTreeHidden(true);
    m_wellTargets.uiCapability()->setUiTreeChildrenHidden(true);
    m_wellTargets.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    m_wellTargets.uiCapability()->setCustomContextMenuEnabled(true);


    m_wellTargets.push_back(new RimWellPathTarget());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef::~RimWellPathGeometryDef()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::insertTarget(RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert)
{
   size_t index = m_wellTargets.index(targetToInsertBefore);
   if (index < m_wellTargets.size()) m_wellTargets.insert(index, targetToInsert);
   else m_wellTargets.push_back(targetToInsert);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::deleteTarget(RimWellPathTarget* targetTodelete)
{
    m_wellTargets.removeChildObject(targetTodelete);
    delete targetTodelete;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&m_startPos == changedField)
    {
        std::cout << "fieldChanged" << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_wellStartType);
    if (m_wellStartType == START_FROM_OTHER_WELL)
    {
        uiOrdering.add(&m_parentWell);
        m_kickoffDepthOrMD.uiCapability()->setUiName("Measured Depth");
        uiOrdering.add(&m_kickoffDepthOrMD);
    }

    if (m_wellStartType == START_AT_SURFACE)
    {
        uiOrdering.add(&m_startPos);
        m_kickoffDepthOrMD.uiCapability()->setUiName("Kick Off Depth");
        uiOrdering.add(&m_kickoffDepthOrMD);
    }

    uiOrdering.add(&m_wellTargets);
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDef::defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, 
                                                     QMenu* menu, 
                                                     QWidget* fieldEditorWidget)
{
    caf::CmdFeatureMenuBuilder menuBuilder;
    
    menuBuilder << "RicNewWellPathListTargetFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteWellPathTargetFeature";

    menuBuilder.appendToMenu(menu);
}
