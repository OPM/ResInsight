/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiuCellSelectionTool.h"

#include "RicFlyToObjectFeature.h"
#include "RicIdentifyAndSelectCellFeature.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"

#include "RigMainGrid.h"

#include "Riu3dSelectionManager.h"
#include "RiuViewer.h"

#include <QAction>
#include <QButtonGroup>
#include <QDialog>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCellSelectionTool::RiuCellSelectionTool( QWidget* parent /*= nullptr */ )
    : QDialog( parent )
{
    setupUI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCellSelectionTool* RicIdentifyAndSelectCellFeature::dialog()
{
    static RiuCellSelectionTool* dlg = nullptr;
    if ( !dlg )
    {
        dlg = new RiuCellSelectionTool();
    }
    return dlg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCellSelectionTool::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout( this );

    QButtonGroup* inputTypeGroup = new QButtonGroup( this );
    m_xyzRadio                   = new QRadioButton( "XYZ Coordinates" );
    m_ijkRadio                   = new QRadioButton( "IJK Cell Indices" );
    inputTypeGroup->addButton( m_xyzRadio );
    inputTypeGroup->addButton( m_ijkRadio );
    m_xyzRadio->setChecked( true );

    mainLayout->addWidget( m_xyzRadio );
    mainLayout->addWidget( m_ijkRadio );

    m_coordinateLabel = new QLabel( "Coordinates:" );
    m_coordinateEdit  = new QLineEdit();
    m_coordinateEdit->setPlaceholderText( "Enter coordinates (space or comma separated)" );

    m_cellLabel = new QLabel( "IJK:" );
    m_cellEdit  = new QLineEdit();
    m_cellEdit->setPlaceholderText( "Enter IJK (space or comma separated)" );

    QGridLayout* labelLayout = new QGridLayout();
    labelLayout->addWidget( m_coordinateLabel, 0, 0 );
    labelLayout->addWidget( m_coordinateEdit, 0, 1 );
    labelLayout->addWidget( m_cellLabel, 1, 0 );
    labelLayout->addWidget( m_cellEdit, 1, 1 );
    mainLayout->addLayout( labelLayout );

    m_submitButton = new QPushButton( "Select Cell" );
    mainLayout->addWidget( m_submitButton );

    m_appendButton = new QPushButton( "Append Cell" );
    mainLayout->addWidget( m_appendButton );

    mainLayout->addStretch();

    connect( m_submitButton, &QPushButton::clicked, this, &RiuCellSelectionTool::validateAndSelect );
    connect( m_appendButton, &QPushButton::clicked, this, &RiuCellSelectionTool::validateAndAppend );
    connect( m_xyzRadio, &QRadioButton::toggled, this, &RiuCellSelectionTool::updateVisibleUiItems );

    updateVisibleUiItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCellSelectionTool::validateAndSelect()
{
    if ( auto activeView = RiaApplication::instance()->activeReservoirView() )
    {
        if ( auto selItem = createSelectionItemFromInput() )
        {
            Riu3dSelectionManager::instance()->setSelectedItem( selItem );

            activeView->updateDisplayModelForCurrentTimeStepAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCellSelectionTool::validateAndAppend()
{
    if ( auto activeView = RiaApplication::instance()->activeReservoirView() )
    {
        if ( auto selItem = createSelectionItemFromInput() )
        {
            Riu3dSelectionManager::instance()->appendItemToSelection( selItem );

            activeView->updateDisplayModelForCurrentTimeStepAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVector<double> RiuCellSelectionTool::parseDoubleValues( const QString& input )
{
    // Flexible regex to extract three double numbers with any surrounding text
    QRegularExpression              coordRegex( R"([-+]?\d+\.?\d*)" );
    QRegularExpressionMatchIterator it = coordRegex.globalMatch( input );

    QVector<double> coords;
    while ( it.hasNext() && coords.size() < 3 )
    {
        QRegularExpressionMatch match = it.next();
        coords.append( match.captured().toDouble() );
    }

    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem* RiuCellSelectionTool::createSelectionItemFromInput()
{
    auto eclipseView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( !eclipseView || !eclipseView->mainGrid() )
    {
        return nullptr;
    }

    size_t cellIndex = cvf::UNDEFINED_SIZE_T;
    auto   mainGrid  = eclipseView->mainGrid();

    if ( m_xyzRadio->isChecked() )
    {
        QVector<double> coords = parseDoubleValues( m_coordinateEdit->text().trimmed() );
        if ( coords.size() != 3 )
        {
            return nullptr;
        }

        cvf::Vec3d targetPoint = cvf::Vec3d( coords[0], coords[1], -coords[2] );

        // Use the main grid to find the cell index. Look at close points 0.5 in length along all main axis if the target point is not in a
        // cell
        auto findCandidateCell = [&]( const cvf::Vec3d& source, double distance ) -> size_t
        {
            std::vector<cvf::Vec3d> points = { source,
                                               source + cvf::Vec3d( 0.0, 0.0, distance ),
                                               source + cvf::Vec3d( distance, 0.0, 0.0 ),
                                               source + cvf::Vec3d( -distance, 0.0, 0.0 ),
                                               source + cvf::Vec3d( 0.0, distance, 0.0 ),
                                               source + cvf::Vec3d( 0.0, -distance, 0.0 ),
                                               source + cvf::Vec3d( 0.0, 0.0, -distance ) };

            for ( const auto& p : points )
            {
                auto candidate = mainGrid->findReservoirCellIndexFromPoint( source );
                if ( candidate != cvf::UNDEFINED_SIZE_T ) return candidate;
            }
            return cvf::UNDEFINED_SIZE_T;
        };

        double iSize, jSize, kSize;
        mainGrid->characteristicCellSizes( &iSize, &jSize, &kSize );

        auto distance = std::min( { iSize, jSize, kSize } ) * 0.4;

        auto candidateCellIndex = findCandidateCell( cvf::Vec3d( coords[0], coords[1], -coords[2] ), distance );
        if ( candidateCellIndex == cvf::UNDEFINED_SIZE_T ) return nullptr;

        cellIndex = candidateCellIndex;
        if ( auto ijk = mainGrid->ijkFromCellIndex( cellIndex ) )
        {
            // 1-based for user input
            m_cellEdit->setText( QString( "%1 %2 %3" ).arg( ijk->i() + 1 ).arg( ijk->j() + 1 ).arg( ijk->k() + 1 ) );
        }
    }
    else
    {
        QVector<double> ijkValues = parseDoubleValues( m_cellEdit->text().trimmed() );
        if ( ijkValues.size() != 3 )
        {
            return nullptr;
        }

        // 0-based for internal use
        int i = ijkValues[0] - 1;
        int j = ijkValues[1] - 1;
        int k = ijkValues[2] - 1;

        if ( mainGrid->isCellValid( i, j, k ) )
        {
            cellIndex = mainGrid->cellIndexFromIJK( i, j, k );
        }
    }

    if ( cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        size_t gridIndex = 0;
        return new RiuEclipseSelectionItem( eclipseView, gridIndex, cellIndex );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCellSelectionTool::updateVisibleUiItems()
{
    bool isXYZ = m_xyzRadio->isChecked();
    m_coordinateEdit->setVisible( isXYZ );
    m_coordinateLabel->setVisible( isXYZ );

    m_cellEdit->setReadOnly( isXYZ );
}
