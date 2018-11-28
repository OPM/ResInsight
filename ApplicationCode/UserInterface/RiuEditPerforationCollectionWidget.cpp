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

#include "RiuEditPerforationCollectionWidget.h"

#include "RimPerforationCollection.h"

#include "RiuTools.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiTableView.h"
#include "cafSelectionManager.h"

#include <QAbstractItemView>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMenu>
#include <QTableView>
#include <QWidget>
#include <QHeaderView>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuEditPerforationCollectionWidget::RiuEditPerforationCollectionWidget(QWidget* parent, RimPerforationCollection* perforationCollection)
    : QDialog(parent, RiuTools::defaultDialogFlags()),
    m_perforationCollection(perforationCollection)
{
    setWindowTitle("Edit Perforation Intervals");

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);

    m_pdmTableView = new caf::PdmUiTableView(this);
    m_pdmTableView->tableView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pdmTableView->tableView()->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pdmTableView->enableHeaderText(false);

    connect(m_pdmTableView->tableView(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    m_pdmTableView->setChildArrayField(&(m_perforationCollection->m_perforations));

    QHeaderView* verticalHeader = m_pdmTableView->tableView()->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Interactive);

    m_pdmTableView->tableView()->resizeColumnsToContents();

    // Set active child array to be able to use generic delete
    caf::SelectionManager::instance()->setActiveChildArrayFieldHandle(&(m_perforationCollection->m_perforations));

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
RiuEditPerforationCollectionWidget::~RiuEditPerforationCollectionWidget()
{
    m_pdmTableView->setChildArrayField(nullptr);

    caf::SelectionManager::instance()->setActiveChildArrayFieldHandle(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuEditPerforationCollectionWidget::customMenuRequested(QPoint pos)
{
    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();

    QMenu menu;
    menu.addAction(commandManager->action("PdmListField_AddItem","New row"));
    menu.addAction(commandManager->action("PdmListField_DeleteItem","Delete row"));
    
    // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
    QPoint globalPos = m_pdmTableView->tableView()->viewport()->mapToGlobal(pos);

    menu.exec(globalPos);
}
