//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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


#include "QSRStdInclude.h"
#include "QSRRunPanel.h"
#include "QSRMainWindow.h"

#include "cvfuSnippetFactory.h"
#include "cvfuTestSnippet.h"
#include "cvfqtUtils.h"

#if QT_VERSION >= 0x050000
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QLineEdit>
#else
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QLineEdit>
#endif
using cvfu::TestSnippet;
using cvfu::SnippetInfo;
using cvfu::SnippetRegistry;


//=================================================================================================================================
//
// QSRRunPanel
//
//=================================================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRRunPanel::QSRRunPanel(QDockWidget* parent)
:   QWidget(parent)
{
    //m_availableSnippets;
    m_currSnippetIndex = -1;

    // Create main layout of the panel by passing ourselves to constructor
    QGridLayout* mainLayout = new QGridLayout(this);

    QLabel* filterLabel = new QLabel("Filter:", this);
    m_categoryFilterCombo = new QComboBox(this);
    m_categoryFilterCombo->addItem("Tests", cvfu::SC_TEST);
    m_categoryFilterCombo->addItem("Heavy tests", cvfu::SC_TEST_HEAVY);
    m_categoryFilterCombo->addItem("All tests", cvfu::SC_ALL_TESTS);
    m_categoryFilterCombo->addItem("All categories", cvfu::SC_ALL);
    m_categoryFilterCombo->setCurrentIndex(0);
    connect(m_categoryFilterCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotCategoryComboCurrentIndexChanged()));
    mainLayout->addWidget(filterLabel, 0, 0);
    mainLayout->addWidget(m_categoryFilterCombo, 0, 1);

    m_infoLabel = new QLabel(this);
    m_currSnippetEdit = new QLineEdit(this);
    connect(m_currSnippetEdit, SIGNAL(returnPressed()), SLOT(slotCurrSnippetEditReturnPressed()));
    mainLayout->addWidget(m_infoLabel, 1, 0);
    mainLayout->addWidget(m_currSnippetEdit, 1, 1);

    m_prevButton = new QPushButton("&Prev", this);
    m_nextButton = new QPushButton("&Next", this);
    connect(m_prevButton, SIGNAL(clicked(bool)), SLOT(slotPrevButtonClicked()));
    connect(m_nextButton, SIGNAL(clicked(bool)), SLOT(slotNextButtonClicked()));
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_prevButton);
    buttonLayout->addWidget(m_nextButton);
    mainLayout->addLayout(buttonLayout, 2, 0, 1, 2);

    mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 3, 0, 1, 2);

    updateAvailableSnippets();
    updateCurrSnippetInfoWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::updateAvailableSnippets()
{
    cvfu::SnippetCategory category = cvfu::SC_ALL;

    int comboIndex = m_categoryFilterCombo->currentIndex();
    if (comboIndex != -1)
    {
        category = static_cast<cvfu::SnippetCategory>(m_categoryFilterCombo->itemData(comboIndex).toInt());
    }

    m_availableSnippets = SnippetRegistry::instance()->availableSnippets(category);
    m_currSnippetIndex = -1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::updateCurrSnippetInfoWidgets()
{
    size_t numSnippets = m_availableSnippets.size();

    QString labelText = QString("Current (of %1):").arg(numSnippets);
    m_infoLabel->setText(labelText);

    QString curr("??");
    if (m_currSnippetIndex >= 0) curr = QString::number(m_currSnippetIndex + 1);
    m_currSnippetEdit->setText(curr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::executeCurrentSnippet()
{
    int numSnippets = static_cast<int>(m_availableSnippets.size());

    if (m_currSnippetIndex < 0 || m_currSnippetIndex >= numSnippets)
    {
        QApplication::beep();
        return;
    }

    SnippetInfo si = m_availableSnippets[m_currSnippetIndex];
    ref<TestSnippet> snippet = SnippetRegistry::instance()->createSnippet(si.id);
    QSRMainWindow::instance()->executeTestSnippetInNewWidget(si.id, snippet.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::slotCategoryComboCurrentIndexChanged()
{
    updateAvailableSnippets();
    updateCurrSnippetInfoWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::slotCurrSnippetEditReturnPressed()
{
    int numSnippets = static_cast<int>(m_availableSnippets.size());

    QString currSnip = m_currSnippetEdit->text();
    bool ok = false;
    int curr = currSnip.toInt(&ok) - 1;
    if (ok)
    {
        if (curr >= 0 && curr < numSnippets && curr != m_currSnippetIndex)
        {
            m_currSnippetIndex = curr;
            executeCurrentSnippet();
        }
    }

    updateCurrSnippetInfoWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::slotNextButtonClicked()
{
    int numSnippets = static_cast<int>(m_availableSnippets.size());
    m_currSnippetIndex++;
    if (m_currSnippetIndex >= numSnippets)
    {
        m_currSnippetIndex = static_cast<int>(numSnippets) - 1;
    }

    executeCurrentSnippet();
    updateCurrSnippetInfoWidgets();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRRunPanel::slotPrevButtonClicked()
{
    m_currSnippetIndex--;
    if (m_currSnippetIndex < 0)
    {
        m_currSnippetIndex = 0;
    }

    executeCurrentSnippet();
    updateCurrSnippetInfoWidgets();
}


// -------------------------------------------------------
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QSRRunPanel.cpp"
#endif

