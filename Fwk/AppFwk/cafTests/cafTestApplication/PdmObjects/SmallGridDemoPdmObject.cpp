#include "SmallGridDemoPdmObject.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( SmallGridDemoPdmObject, "SmallGridDemoPdmObject" );

SmallGridDemoPdmObject::SmallGridDemoPdmObject()
{
    CAF_PDM_InitObject( "Small Grid Demo Object",
                        "",
                        "This object is a demo of the CAF framework",
                        "This object is a demo of the CAF framework" );

    CAF_PDM_InitField( &m_intFieldStandard,
                       "Standard",
                       0,
                       "Standard",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldUseFullSpace,
                       "FullSpace",
                       0,
                       "Use Full Space For Both",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldUseFullSpaceLabel,
                       "FullSpaceLabel",
                       0,
                       "Total 3, Label MAX",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldUseFullSpaceField,
                       "FullSpaceField",
                       0,
                       "Total MAX, Label 1",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldWideLabel,
                       "WideLabel",
                       0,
                       "Wide Label",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldWideField,
                       "WideField",
                       0,
                       "Wide Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldLeft,
                       "LeftField",
                       0,
                       "Left Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldRight,
                       "RightField",
                       0,
                       "Right Field With More Text",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldWideBoth,
                       "WideBoth",
                       0,
                       "Wide Both",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );

    CAF_PDM_InitField( &m_intFieldWideBoth2,
                       "WideBoth2",
                       0,
                       "Wide Both",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldLeft2,
                       "LeftFieldInGrp",
                       0,
                       "Left Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldCenter,
                       "CenterFieldInGrp",
                       0,
                       "Center Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldRight2,
                       "RightFieldInGrp",
                       0,
                       "Right Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldLabelTop,
                       "FieldLabelTop",
                       0,
                       "Field Label Top",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    m_intFieldLabelTop.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    CAF_PDM_InitField( &m_stringFieldLabelHidden,
                       "FieldLabelHidden",
                       QString( "Hidden Label Field" ),
                       "Field Label Hidden",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    m_stringFieldLabelHidden.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    CAF_PDM_InitField( &m_intFieldWideBothAuto,
                       "WideBothAuto",
                       0,
                       "Wide ",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldLeftAuto,
                       "LeftFieldInGrpAuto",
                       0,
                       "Left Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldCenterAuto,
                       "CenterFieldInGrpAuto",
                       0,
                       "Center Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldRightAuto,
                       "RightFieldInGrpAuto",
                       0,
                       "Right Field",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldLabelTopAuto,
                       "FieldLabelTopAuto",
                       0,
                       "Field Label Top",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    m_intFieldLabelTopAuto.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    CAF_PDM_InitField( &m_stringFieldLabelHiddenAuto,
                       "FieldLabelHiddenAuto",
                       QString( "Hidden Label Field" ),
                       "Field Label Hidden",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    m_stringFieldLabelHiddenAuto.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    CAF_PDM_InitField( &m_intFieldLeftOfGroup,
                       "FieldLeftOfGrp",
                       0,
                       "Left of group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldRightOfGroup,
                       "FieldRightOfGrp",
                       0,
                       "Right of group wide label",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );

    CAF_PDM_InitField( &m_intFieldInsideGroup1,
                       "FieldInGrp1",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldInsideGroup2,
                       "FieldInGrp2",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldInsideGroup3,
                       "FieldInGrp3",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldInsideGroup4,
                       "FieldInGrp4",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldInsideGroup5,
                       "FieldInGrp5",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_intFieldInsideGroup6,
                       "FieldInGrp6",
                       0,
                       "Inside Group",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallGridDemoPdmObject::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_intFieldStandard );
    uiOrdering.add( &m_intFieldUseFullSpace );
    uiOrdering.add( &m_intFieldUseFullSpaceLabel, { .totalColumnSpan = 3 } );
    uiOrdering.add( &m_intFieldUseFullSpaceField, { .leftLabelColumnSpan = 1 } );
    uiOrdering.add( &m_intFieldWideLabel, { .totalColumnSpan = 4, .leftLabelColumnSpan = 3 } );
    uiOrdering.add( &m_intFieldWideField, { .totalColumnSpan = 4, .leftLabelColumnSpan = 1 } );
    uiOrdering.add( &m_intFieldLeft );
    uiOrdering.appendToRow( &m_intFieldRight );
    uiOrdering.add( &m_intFieldWideBoth, { .totalColumnSpan = 4, .leftLabelColumnSpan = 2 } );

    QString dynamicGroupName = QString( "Dynamic Group Text (%1)" ).arg( m_intFieldStandard() );

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Wide Group", { .totalColumnSpan = 4 } );
    group->add( &m_intFieldWideBoth2, { .totalColumnSpan = 6, .leftLabelColumnSpan = 3 } );
    group->add( &m_intFieldLeft2 );
    group->appendToRow( &m_intFieldCenter );
    group->appendToRow( &m_intFieldRight2 );
    group->add( &m_intFieldLabelTop, { .totalColumnSpan = 6 } );
    group->add( &m_stringFieldLabelHidden, { .totalColumnSpan = 6 } );

    caf::PdmUiGroup* autoGroup = uiOrdering.addNewGroup( "Automatic Full Width Group" );
    autoGroup->add( &m_intFieldWideBothAuto );
    autoGroup->add( &m_intFieldLeftAuto );
    autoGroup->appendToRow( &m_intFieldCenterAuto );
    autoGroup->appendToRow( &m_intFieldRightAuto );
    autoGroup->add( &m_intFieldLabelTopAuto );
    autoGroup->add( &m_stringFieldLabelHiddenAuto );

    uiOrdering.add( &m_intFieldLeftOfGroup );
    caf::PdmUiGroup* group2 =
        uiOrdering.addNewGroup( "Right Group", { .newRow = false, .totalColumnSpan = 2, .leftLabelColumnSpan = 0 } );
    group2->setEnableFrame( false );
    group2->add( &m_intFieldInsideGroup1 );

    caf::PdmUiGroup* group3 = uiOrdering.addNewGroup( "Narrow L", { .totalColumnSpan = 1 } );
    group3->add( &m_intFieldInsideGroup2 );
    uiOrdering.add( &m_intFieldRightOfGroup, { .newRow = false, .totalColumnSpan = 3, .leftLabelColumnSpan = 2 } );

    caf::PdmUiGroup* groupL = uiOrdering.addNewGroup( "Left Group", { .totalColumnSpan = 1 } );
    groupL->add( &m_intFieldInsideGroup3 );
    groupL->add( &m_intFieldInsideGroup5 );
    caf::PdmUiGroup* groupR = uiOrdering.addNewGroup( "Right Wide Group", { .newRow = false, .totalColumnSpan = 3 } );
    groupR->setEnableFrame( false );
    groupR->add( &m_intFieldInsideGroup4 );
    groupR->add( &m_intFieldInsideGroup6 );
}
