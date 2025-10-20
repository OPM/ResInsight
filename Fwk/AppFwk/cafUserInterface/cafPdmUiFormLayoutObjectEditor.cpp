//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmUiFormLayoutObjectEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmLogging.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiButton.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldEditorHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiLabel.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmXmlObjectHandle.h"

#include "cafAssert.h"

#include "QMinimizePanel.h"

#include <QCoreApplication>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFormLayoutObjectEditor::PdmUiFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFormLayoutObjectEditor::~PdmUiFormLayoutObjectEditor()
{
    // If there are field editor present, the usage of this editor has not cleared correctly
    // The intended usage is to call the method setPdmObject(NULL) before closing the dialog
    if ( !m_fieldViews.empty() )
    {
        CAF_PDM_LOG_WARNING( QString( "UI Form Layout Editor: Destructor called with %1 field views still present. "
                                      "Call setPdmObject(nullptr) before destruction." )
                                 .arg( m_fieldViews.size() ) );
    }
    CAF_ASSERT( m_fieldViews.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( const PdmUiOrdering& uiOrdering,
                                                                                               QWidget* containerWidget,
                                                                                               const QString& uiConfigName )
{
    QSize beforeSize = containerWidget->sizeHint();

    ensureWidgetContainsEmptyGridLayout( containerWidget );
    int stretch = recursivelyConfigureAndUpdateUiOrderingInGridLayout( uiOrdering, containerWidget, uiConfigName );

    QSize afterSize = containerWidget->sizeHint();
    if ( beforeSize != afterSize )
    {
        containerWidget->adjustSize();
    }

    return stretch > 0;
}

//--------------------------------------------------------------------------------------------------
/// Add all widgets at a recursion level in the form.
/// Returns the stretch factor that should be applied at the level above.
//--------------------------------------------------------------------------------------------------
int caf::PdmUiFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInGridLayout( const PdmUiOrdering& uiOrdering,
                                                                                           QWidget* containerWidgetWithGridLayout,
                                                                                           const QString& uiConfigName )
{
    int sumRowStretch = 0;
    CAF_ASSERT( containerWidgetWithGridLayout );

    QWidget* previousTabOrderWidget = nullptr;

    // Currently, only QGridLayout is supported
    QGridLayout* parentLayout = dynamic_cast<QGridLayout*>( containerWidgetWithGridLayout->layout() );
    CAF_ASSERT( parentLayout );

    PdmUiOrdering::TableLayout tableLayout = uiOrdering.calculateTableLayout( uiConfigName );

    int totalRows    = static_cast<int>( tableLayout.size() );
    int totalColumns = uiOrdering.nrOfColumns( tableLayout );

    for ( int currentRowIndex = 0; currentRowIndex < totalRows; ++currentRowIndex )
    {
        int currentColumn = 0;

        const PdmUiOrdering::RowLayout& uiItemsInRow = tableLayout[currentRowIndex];

        int columnsRequiredForCurrentRow = uiOrdering.nrOfRequiredColumnsInRow( uiItemsInRow );
        int nrOfExpandingItemsInRow      = uiOrdering.nrOfExpandingItemsInRow( uiItemsInRow );
        int spareColumnsInRow            = totalColumns - columnsRequiredForCurrentRow;

        std::div_t columnsDiv = { 0, 0 };
        if ( spareColumnsInRow && nrOfExpandingItemsInRow )
        {
            columnsDiv = std::div( spareColumnsInRow, nrOfExpandingItemsInRow );
        }

        for ( size_t i = 0; i < uiItemsInRow.size(); ++i )
        {
            PdmUiItem*                   currentItem   = uiItemsInRow[i].first;
            PdmUiOrdering::LayoutOptions currentLayout = uiItemsInRow[i].second;

            int minimumItemColumnSpan = 0, minimumLabelColumnSpan = 0, minimumFieldColumnSpan = 0;
            uiOrdering.nrOfColumnsRequiredForItem( uiItemsInRow[i],
                                                   &minimumItemColumnSpan,
                                                   &minimumLabelColumnSpan,
                                                   &minimumFieldColumnSpan );
            bool isExpandingItem = currentLayout.totalColumnSpan == PdmUiOrdering::MAX_COLUMN_SPAN;

            int spareColumnsToAssign = 0;
            if ( isExpandingItem )
            {
                spareColumnsToAssign += columnsDiv.quot;
                if ( i == 0 ) spareColumnsToAssign += columnsDiv.rem;
            }

            int itemColumnSpan = minimumItemColumnSpan + spareColumnsToAssign;

            if ( currentItem->isUiGroup() )
            {
                int groupStretchFactor = recursivelyAddGroupToGridLayout( currentItem,
                                                                          containerWidgetWithGridLayout,
                                                                          uiConfigName,
                                                                          parentLayout,
                                                                          currentRowIndex,
                                                                          currentColumn,
                                                                          itemColumnSpan );
                parentLayout->setRowStretch( currentRowIndex, groupStretchFactor );
                currentColumn += itemColumnSpan;
                sumRowStretch += groupStretchFactor;
            }
            else if ( auto* label = dynamic_cast<PdmUiLabel*>( currentItem ) )
            {
                if ( auto qLabel = findOrCreateLabel( containerWidgetWithGridLayout, label, uiConfigName ) )
                {
                    parentLayout->addWidget( qLabel, currentRowIndex, currentColumn, 1, itemColumnSpan, Qt::AlignTop );
                    currentColumn += itemColumnSpan;
                }
                else
                {
                    CAF_PDM_LOG_ERROR( QString( "UI Form Layout Editor: Failed to create label for text '%1'." )
                                           .arg( label->uiName( uiConfigName ) ) );
                }
            }
            else if ( auto* button = dynamic_cast<PdmUiButton*>( currentItem ) )
            {
                if ( auto qButton = findOrCreateButton( containerWidgetWithGridLayout, button, uiConfigName ) )
                {
                    parentLayout->addWidget( qButton, currentRowIndex, currentColumn, 1, itemColumnSpan, button->alignment() );
                    currentColumn += itemColumnSpan;
                }
                else
                {
                    CAF_PDM_LOG_ERROR( QString( "UI Form Layout Editor: Failed to create button for text '%1'." )
                                           .arg( button->uiName( uiConfigName ) ) );
                }
            }
            else
            {
                PdmUiFieldEditorHandle* fieldEditor = nullptr;
                PdmUiFieldHandle*       field       = dynamic_cast<PdmUiFieldHandle*>( currentItem );

                if ( field )
                    fieldEditor = findOrCreateFieldEditor( containerWidgetWithGridLayout, field, uiConfigName );

                if ( fieldEditor )
                {
                    // Also assign required item space that isn't taken up by field and label
                    spareColumnsToAssign += minimumItemColumnSpan - ( minimumLabelColumnSpan + minimumFieldColumnSpan );

                    // Place the widget(s) into the correct parent and layout
                    QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                    if ( fieldCombinedWidget )
                    {
                        parentLayout->addWidget( fieldCombinedWidget, currentRowIndex, currentColumn, 1, itemColumnSpan );
                        parentLayout->setRowStretch( currentRowIndex, fieldEditor->rowStretchFactor() );
                        sumRowStretch += fieldEditor->rowStretchFactor();
                    }
                    else
                    {
                        QWidget* fieldEditorWidget = fieldEditor->editorWidget();
                        if ( !fieldEditorWidget ) continue;

                        int fieldColumnSpan = minimumFieldColumnSpan;

                        QWidget*                    fieldLabelWidget = fieldEditor->labelWidget();
                        PdmUiItemInfo::LabelPosType labelPos         = PdmUiItemInfo::HIDDEN;

                        if ( fieldLabelWidget )
                        {
                            labelPos = field->uiLabelPosition( uiConfigName );

                            if ( labelPos == PdmUiItemInfo::HIDDEN )
                            {
                                fieldLabelWidget->hide();
                            }
                            else if ( labelPos == PdmUiItemInfo::TOP )
                            {
                                QVBoxLayout* labelAndFieldVerticalLayout = new QVBoxLayout();
                                parentLayout->addLayout( labelAndFieldVerticalLayout,
                                                         currentRowIndex,
                                                         currentColumn,
                                                         1,
                                                         itemColumnSpan,
                                                         Qt::AlignTop );
                                labelAndFieldVerticalLayout->addWidget( fieldLabelWidget, 0, Qt::AlignTop );
                                labelAndFieldVerticalLayout->addWidget( fieldEditorWidget, 1, Qt::AlignTop );

                                // Apply margins determined by the editor type
                                // fieldLabelWidget->setContentsMargins(fieldEditor->labelContentMargins());
                                currentColumn += itemColumnSpan;
                            }
                            else
                            {
                                CAF_ASSERT( labelPos == PdmUiItemInfo::LEFT );
                                int leftLabelColumnSpan = minimumLabelColumnSpan;
                                if ( currentLayout.leftLabelColumnSpan == PdmUiOrdering::MAX_COLUMN_SPAN &&
                                     currentLayout.totalColumnSpan != PdmUiOrdering::MAX_COLUMN_SPAN )
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign;
                                    spareColumnsToAssign = 0;
                                }
                                else if ( currentLayout.leftLabelColumnSpan == PdmUiOrdering::MAX_COLUMN_SPAN )
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign / 2;
                                    spareColumnsToAssign -= spareColumnsToAssign / 2;
                                }

                                parentLayout->addWidget( fieldLabelWidget,
                                                         currentRowIndex,
                                                         currentColumn,
                                                         1,
                                                         leftLabelColumnSpan,
                                                         Qt::AlignTop );
                                currentColumn += leftLabelColumnSpan;

                                // Apply margins determined by the editor type
                                fieldLabelWidget->setContentsMargins( fieldEditor->labelContentMargins() );
                            }
                        }

                        if ( labelPos != PdmUiItemInfo::TOP ) // Already added if TOP
                        {
                            fieldColumnSpan += spareColumnsToAssign;

                            CAF_ASSERT( fieldColumnSpan >= 1 && "Need at least one column for the field" );
                            fieldColumnSpan = std::max( 1, fieldColumnSpan );

                            parentLayout->addWidget( fieldEditorWidget,
                                                     currentRowIndex,
                                                     currentColumn,
                                                     1,
                                                     fieldColumnSpan,
                                                     Qt::AlignTop );
                            currentColumn += fieldColumnSpan;
                        }

                        if ( previousTabOrderWidget )
                        {
                            QWidget::setTabOrder( previousTabOrderWidget, fieldEditorWidget );
                        }
                        previousTabOrderWidget = fieldEditorWidget;

                        parentLayout->setRowStretch( currentRowIndex, fieldEditor->rowStretchFactor() );
                        sumRowStretch += fieldEditor->rowStretchFactor();
                    }
                    fieldEditor->updateUi( uiConfigName );
                }
            }
        }

        CAF_ASSERT( currentColumn <= totalColumns );
    }
    containerWidgetWithGridLayout->updateGeometry();
    // The magnitude of the stretch should not be sent up, only if there was stretch or not
    return sumRowStretch;
}

//--------------------------------------------------------------------------------------------------
/// Create a group and add widgets. Return true if the containing row needs to be stretched.
//--------------------------------------------------------------------------------------------------
int caf::PdmUiFormLayoutObjectEditor::recursivelyAddGroupToGridLayout( PdmUiItem*     currentItem,
                                                                       QWidget*       containerWidgetWithGridLayout,
                                                                       const QString& uiConfigName,
                                                                       QGridLayout*   parentLayout,
                                                                       int            currentRowIndex,
                                                                       int            currentColumn,
                                                                       int            itemColumnSpan )
{
    PdmUiGroup* group = static_cast<PdmUiGroup*>( currentItem );

    QMinimizePanel* groupBox = findOrCreateGroupBox( containerWidgetWithGridLayout, group, uiConfigName );

    int stretch = recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, groupBox->contentFrame(), uiConfigName );

    /// Insert the group box at the correct position of the parent layout
    parentLayout->addWidget( groupBox, currentRowIndex, currentColumn, 1, itemColumnSpan );

    return stretch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiFormLayoutObjectEditor::isUiGroupExpanded( const PdmUiGroup* uiGroup ) const
{
    if ( uiGroup->hasForcedExpandedState() ) return uiGroup->forcedExpandedState();

    auto kwMapPair = m_objectKeywordGroupUiNameExpandedState.find( pdmObject()->xmlCapability()->classKeyword() );
    if ( kwMapPair != m_objectKeywordGroupUiNameExpandedState.end() )
    {
        QString keyword = uiGroup->keyword();

        auto uiNameExpStatePair = kwMapPair->second.find( keyword );
        if ( uiNameExpStatePair != kwMapPair->second.end() )
        {
            return uiNameExpStatePair->second;
        }
    }

    return uiGroup->isExpandedByDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel* caf::PdmUiFormLayoutObjectEditor::findOrCreateGroupBox( QWidget*       parent,
                                                                        PdmUiGroup*    group,
                                                                        const QString& uiConfigName )
{
    QString         groupBoxKey = group->keyword();
    QMinimizePanel* groupBox    = nullptr;

    // Find or create groupBox
    std::map<QString, QPointer<QMinimizePanel>>::iterator it;
    it = m_groupBoxes.find( groupBoxKey );

    if ( it == m_groupBoxes.end() )
    {
        auto newBoxIt = m_newGroupBoxes.find( groupBoxKey );
        if ( newBoxIt != m_newGroupBoxes.end() )
        {
            // This error happens when the same group is added to the same parent multiple times
            // https://github.com/OPM/ResInsight/issues/11731
            // Suggested approach: Make sure that all objects and groups in PdmUiOrdering have unique names.

            auto message = "Detected duplicate group box with keyword: " + groupBoxKey;
            CAF_PDM_LOG_ERROR( QString( "UI Form Layout Editor: %1. This may cause layout issues. Ensure unique group "
                                        "names in PdmUiOrdering." )
                                   .arg( message ) );
            CAF_ASSERT( false && message.toStdString().data() );
        }

        groupBox = new QMinimizePanel( parent );
        groupBox->enableFrame( group->enableFrame() );
        groupBox->setTitle( group->uiName( uiConfigName ) );
        groupBox->setObjectName( group->keyword() );
        connect( groupBox, SIGNAL( expandedChanged( bool ) ), this, SLOT( groupBoxExpandedStateToggled( bool ) ) );

        m_newGroupBoxes[groupBoxKey] = groupBox;
    }
    else
    {
        groupBox = it->second;
        CAF_ASSERT( groupBox );
        m_newGroupBoxes[groupBoxKey] = groupBox;
    }

    QMargins contentMargins;
    if ( group->enableFrame() )
    {
        contentMargins = QMargins( 6, 6, 6, 6 );
    }

    ensureWidgetContainsEmptyGridLayout( groupBox->contentFrame(), contentMargins );

    // Set Expanded state
    bool isExpanded = isUiGroupExpanded( group );
    groupBox->setExpanded( isExpanded );

    // Update the title to be able to support dynamic group names
    groupBox->setTitle( group->uiName( uiConfigName ) );
    groupBox->updateGeometry();
    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* caf::PdmUiFormLayoutObjectEditor::findOrCreateLabel( QWidget* parent, PdmUiLabel* label, const QString& uiConfigName )
{
    QString labelKey = label->uiName( uiConfigName );
    QLabel* qLabel   = nullptr;

    // Find or create label
    std::map<QString, QPointer<QLabel>>::iterator it;
    it = m_labels.find( labelKey );

    if ( it == m_labels.end() )
    {
        auto newLabelIt = m_newLabels.find( labelKey );
        if ( newLabelIt != m_newLabels.end() )
        {
            auto message = "Detected duplicate label with text: " + labelKey;
            CAF_PDM_LOG_ERROR( QString( "UI Form Layout Editor: %1. This may cause layout issues. Ensure unique label "
                                        "texts in PdmUiOrdering." )
                                   .arg( message ) );
        }

        qLabel = new QLabel( parent );
        qLabel->setText( label->uiName( uiConfigName ) );
        qLabel->setWordWrap( true );
        qLabel->setObjectName( labelKey );

        m_newLabels[labelKey] = qLabel;
    }
    else
    {
        qLabel = it->second;
        CAF_ASSERT( qLabel );
        m_newLabels[labelKey] = qLabel;
    }

    // Update the text to support dynamic label content
    qLabel->setText( label->uiName( uiConfigName ) );
    return qLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPushButton* caf::PdmUiFormLayoutObjectEditor::findOrCreateButton( QWidget*       parent,
                                                                   PdmUiButton*   button,
                                                                   const QString& uiConfigName )
{
    QString      buttonKey = button->uiName( uiConfigName );
    QPushButton* qButton   = nullptr;

    // Find or create button
    std::map<QString, QPointer<QPushButton>>::iterator it;
    it = m_buttons.find( buttonKey );

    if ( it == m_buttons.end() )
    {
        auto newButtonIt = m_newButtons.find( buttonKey );
        if ( newButtonIt != m_newButtons.end() )
        {
            auto message = "Detected duplicate button with text: " + buttonKey;
            CAF_PDM_LOG_ERROR( QString( "UI Form Layout Editor: %1. This may cause layout issues. Ensure unique button "
                                        "texts in PdmUiOrdering." )
                                   .arg( message ) );
        }

        qButton = new QPushButton( parent );
        qButton->setText( button->uiName( uiConfigName ) );
        qButton->setObjectName( buttonKey );

        // Set size policy to size the button to its content rather than fill available space
        qButton->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed );

        // Set icon if available
        auto icon = button->uiIcon( uiConfigName );
        if ( icon && !icon->isNull() )
        {
            qButton->setIcon( *icon );
        }

        // Connect callback if available
        auto callback = button->clickCallback();
        if ( callback )
        {
            QObject::connect( qButton, &QPushButton::clicked, [callback]() { callback(); } );
        }

        m_newButtons[buttonKey] = qButton;
    }
    else
    {
        qButton = it->second;
        CAF_ASSERT( qButton );
        m_newButtons[buttonKey] = qButton;
    }

    // Update the text and icon to support dynamic button content
    qButton->setText( button->uiName( uiConfigName ) );
    auto icon = button->uiIcon( uiConfigName );
    if ( icon && !icon->isNull() )
    {
        qButton->setIcon( *icon );
    }

    return qButton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiFieldEditorHandle* caf::PdmUiFormLayoutObjectEditor::findOrCreateFieldEditor( QWidget*          parent,
                                                                                        PdmUiFieldHandle* field,
                                                                                        const QString&    uiConfigName )
{
    caf::PdmUiFieldEditorHandle* fieldEditor = nullptr;

    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it = m_fieldViews.find( field->fieldHandle() );

    if ( it == m_fieldViews.end() )
    {
        fieldEditor = PdmUiFieldEditorHelper::createFieldEditorForField( field, uiConfigName );

        if ( fieldEditor )
        {
            m_fieldViews[field->fieldHandle()] = fieldEditor;
            fieldEditor->setContainingEditor( this );
            fieldEditor->setUiField( field );
            fieldEditor->createWidgets( parent );
        }
        else
        {
            // This happens if no editor is available for a given field
            // If the macro for registering the editor is put as the single statement
            // in a cpp file, a dummy static class must be used to make sure the compile unit
            // is included
            //
            // See cafPdmUiCoreColor3f and cafPdmUiCoreVec3d
            //
            // Default editors are registered in cafPdmUiDefaultObjectEditor.cpp
            //
            // Exclude childArray and child object fields from this warning. A PdmChildArrayFieldHandle field
            // typically use a list or table editor, see RimWellPathGeometryDef::m_wellTargets

            bool isChildArrayField  = dynamic_cast<PdmChildArrayFieldHandle*>( field->fieldHandle() ) != nullptr;
            bool isChildObjectField = dynamic_cast<PdmChildFieldHandle*>( field->fieldHandle() ) != nullptr;
            if ( !isChildArrayField && !isChildObjectField )
            {
                QString fieldTypeName = field ? field->fieldHandle()->keyword() : "unknown";
                CAF_PDM_LOG_ERROR(
                    QString( "UI Form Layout Editor: No field editor available for field type '%1' in config '%2'. "
                             "Check that the field editor is properly registered." )
                        .arg( fieldTypeName )
                        .arg( uiConfigName.isEmpty() ? "default" : uiConfigName ) );
            }
        }
    }
    else
    {
        fieldEditor = it->second;
    }

    if ( fieldEditor )
    {
        m_usedFields.insert( field->fieldHandle() );
    }

    return fieldEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::ensureWidgetContainsEmptyGridLayout( QWidget* containerWidget,
                                                                            QMargins contentMargins )
{
    CAF_ASSERT( containerWidget );
    QLayout* layout = containerWidget->layout();
    if ( layout != nullptr )
    {
        // Remove all items from the layout, then reparent the layout to a temporary
        // This is because you cannot remove a layout from a widget but it gets moved when reparenting.
        QLayoutItem* item;
        while ( ( item = layout->takeAt( 0 ) ) != 0 )
        {
            delete item;
        }
        QWidget().setLayout( layout );
    }

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setContentsMargins( contentMargins );
    containerWidget->setLayout( gridLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::groupBoxExpandedStateToggled( bool isExpanded )
{
    if ( !this->pdmObject()->xmlCapability() ) return;

    QString         objKeyword = this->pdmObject()->xmlCapability()->classKeyword();
    QMinimizePanel* panel      = dynamic_cast<QMinimizePanel*>( this->sender() );

    if ( !panel ) return;

    m_objectKeywordGroupUiNameExpandedState[objKeyword][panel->objectName()] = isExpanded;

    // Required to update the layout when the group box is expanded
    // https://github.com/OPM/ResInsight/issues/12119
    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::cleanupBeforeSettingPdmObject()
{
    // Trigger a hide event the editor to allow it to clean up. This is required to persist the changes in
    // the editor when changing to a different dock widget. Hide all editors before deleting them.
    // https://github.com/OPM/ResInsight/issues/12599
    for ( const auto& [fieldHandle, fieldEditor] : m_fieldViews )
    {
        if ( fieldEditor && fieldEditor->editorWidget() )
        {
            fieldEditor->editorWidget()->hide();
        }
    }

    // Delete all field editors, make sure this is done after the field editors have been hidden
    for ( const auto& [fieldHandle, fieldEditor] : m_fieldViews )
    {
        if ( fieldEditor )
        {
            delete fieldEditor;
        }
    }
    m_fieldViews.clear();

    m_newGroupBoxes.clear();
    m_newLabels.clear();
    m_newButtons.clear();

    std::map<QString, QPointer<QMinimizePanel>>::iterator groupIt;
    for ( groupIt = m_groupBoxes.begin(); groupIt != m_groupBoxes.end(); ++groupIt )
    {
        QMinimizePanel* groupBox = groupIt->second;
        if ( groupBox )
        {
            // https://github.com/OPM/ResInsight/issues/9719
            // When opening a summary file from the command line, it seems like the groupBoxes are not deleted when
            // using groupBox->deleteLater()

            delete groupBox;
        }
    }

    m_groupBoxes.clear();

    std::map<QString, QPointer<QLabel>>::iterator labelIt;
    for ( labelIt = m_labels.begin(); labelIt != m_labels.end(); ++labelIt )
    {
        QLabel* label = labelIt->second;
        if ( label )
        {
            delete label;
        }
    }

    m_labels.clear();

    std::map<QString, QPointer<QPushButton>>::iterator buttonIt;
    for ( buttonIt = m_buttons.begin(); buttonIt != m_buttons.end(); ++buttonIt )
    {
        QPushButton* button = buttonIt->second;
        if ( button )
        {
            delete button;
        }
    }

    m_buttons.clear();

    // Note: Layouts are now managed by Qt's parent-child ownership system.
    // When added to parent layouts via addLayout(), Qt automatically handles cleanup.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    caf::PdmUiOrdering config;
    if ( pdmObject() )
    {
        caf::PdmUiObjectHandle* uiObject = uiObj( pdmObject() );
        if ( uiObject )
        {
            uiObject->uiOrdering( uiConfigName, config );
        }
    }

    // Set all fieldViews to be unvisited

    m_usedFields.clear();

    // Set all group Boxes to be unvisited
    m_newGroupBoxes.clear();

    recursivelyConfigureAndUpdateTopLevelUiOrdering( config, uiConfigName );

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector<PdmFieldHandle*> fvhToRemoveFromMap;
    for ( auto oldFvIt = m_fieldViews.begin(); oldFvIt != m_fieldViews.end(); ++oldFvIt )
    {
        if ( m_usedFields.count( oldFvIt->first ) == 0 )
        {
            // The old field editor is not present anymore, get rid of it
            delete oldFvIt->second;
            fvhToRemoveFromMap.push_back( oldFvIt->first );
        }
    }

    for ( size_t i = 0; i < fvhToRemoveFromMap.size(); ++i )
    {
        m_fieldViews.erase( fvhToRemoveFromMap[i] );
    }

    // Remove all unmentioned group boxes

    std::map<QString, QPointer<QMinimizePanel>>::iterator itOld;
    std::map<QString, QPointer<QMinimizePanel>>::iterator itNew;

    for ( itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld )
    {
        itNew = m_newGroupBoxes.find( itOld->first );
        if ( itNew == m_newGroupBoxes.end() )
        {
            // The old groupBox is not present anymore, get rid of it
            if ( !itOld->second.isNull() ) delete itOld->second;
        }
    }
    m_groupBoxes = m_newGroupBoxes;

    // Clean up unused labels
    std::map<QString, QPointer<QLabel>>::iterator labelMapIt;
    for ( labelMapIt = m_labels.begin(); labelMapIt != m_labels.end(); ++labelMapIt )
    {
        std::map<QString, QPointer<QLabel>>::iterator it = m_newLabels.find( labelMapIt->first );
        if ( it == m_newLabels.end() )
        {
            // The old label is not present anymore, get rid of it
            if ( !labelMapIt->second.isNull() ) delete labelMapIt->second;
        }
    }
    m_labels = m_newLabels;

    // Clean up unused buttons
    std::map<QString, QPointer<QPushButton>>::iterator buttonMapIt;
    for ( buttonMapIt = m_buttons.begin(); buttonMapIt != m_buttons.end(); ++buttonMapIt )
    {
        std::map<QString, QPointer<QPushButton>>::iterator it = m_newButtons.find( buttonMapIt->first );
        if ( it == m_newButtons.end() )
        {
            // The old button is not present anymore, get rid of it
            if ( !buttonMapIt->second.isNull() ) delete buttonMapIt->second;
        }
    }
    m_buttons = m_newButtons;

    // Notify pdm object when widgets have been created
    caf::PdmUiObjectHandle* uiObject = uiObj( pdmObject() );
    if ( uiObject )
    {
        uiObject->onEditorWidgetsCreated();
    }
}

//--------------------------------------------------------------------------------------------------
/// Unused. Should probably remove
//--------------------------------------------------------------------------------------------------
void caf::PdmUiFormLayoutObjectEditor::recursiveVerifyUniqueNames( const std::vector<PdmUiItem*>& uiItems,
                                                                   const QString&                 uiConfigName,
                                                                   std::set<QString>*             fieldKeywordNames,
                                                                   std::set<QString>*             groupNames )
{
    for ( size_t i = 0; i < uiItems.size(); ++i )
    {
        if ( uiItems[i]->isUiGroup() )
        {
            PdmUiGroup*                    group         = static_cast<PdmUiGroup*>( uiItems[i] );
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = group->keyword();

            if ( groupNames->find( groupBoxKey ) != groupNames->end() )
            {
                // It is not supported to have two groups with identical names
                CAF_ASSERT( false );
            }
            else
            {
                groupNames->insert( groupBoxKey );
            }

            recursiveVerifyUniqueNames( groupChildren, uiConfigName, fieldKeywordNames, groupNames );
        }
        else
        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>( uiItems[i] );

            QString fieldKeyword = field->fieldHandle()->keyword();
            if ( fieldKeywordNames->find( fieldKeyword ) != fieldKeywordNames->end() )
            {
                // It is not supported to have two fields with identical names
                CAF_ASSERT( false );
            }
            else
            {
                fieldKeywordNames->insert( fieldKeyword );
            }
        }
    }
}
