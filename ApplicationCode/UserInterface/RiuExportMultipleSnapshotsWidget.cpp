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

#include "RiuExportMultipleSnapshotsWidget.h"

#include "RimMultiSnapshotDefinition.h"
#include "RimProject.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiTableView.h"
#include "cafSelectionManager.h"

#include <QAbstractItemView>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMenu>
#include <QTableView>
#include <QWidget>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuExportMultipleSnapshotsWidget::RiuExportMultipleSnapshotsWidget(QWidget* parent, RimProject* project)
    : QDialog(parent),
    m_rimProject(project)
{
    setWindowTitle("Export Multiple Snapshots");

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);

    m_pdmTableView = new caf::PdmUiTableView(this);
    m_pdmTableView->tableView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pdmTableView->tableView()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_pdmTableView->tableView(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    m_pdmTableView->setListField(&(project->multiSnapshotDefinitions()));

    // Set active child array to be able to use generic delete
    caf::SelectionManager::instance()->setActiveChildArrayFieldHandle(&(project->multiSnapshotDefinitions()));

    dialogLayout->addWidget(m_pdmTableView);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    dialogLayout->addWidget(buttonBox);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuExportMultipleSnapshotsWidget::~RiuExportMultipleSnapshotsWidget()
{
    m_pdmTableView->setListField(nullptr);

    caf::SelectionManager::instance()->setActiveChildArrayFieldHandle(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuExportMultipleSnapshotsWidget::customMenuRequested(QPoint pos)
{
    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();

    QMenu menu;
    menu.addAction(commandManager->action("PdmListField_DeleteItem","Delete row"));

    QAction* newRowAction = new QAction("Add new row", this);
    connect(newRowAction, SIGNAL(triggered()), SLOT(addSnapshotItem()));
    menu.addAction(newRowAction);
    
    // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
    QPoint globalPos = m_pdmTableView->tableView()->viewport()->mapToGlobal(pos);

    menu.exec(globalPos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuExportMultipleSnapshotsWidget::addSnapshotItem()
{
    if (!m_rimProject) return;

    RimMultiSnapshotDefinition* multiSnapshot = new RimMultiSnapshotDefinition();
    // TODO: init with available time step from 

    m_rimProject->multiSnapshotDefinitions.push_back(multiSnapshot);
    m_rimProject->multiSnapshotDefinitions.uiCapability()->updateConnectedEditors();
}
