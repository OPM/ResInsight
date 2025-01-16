/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicPolygonFromImageDialog.h"

#include "RicCreateContourMapPolygonTools.h"

#include "RigPolygonTools.h"

#include <QGuiApplication>
#include <QScreen>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ImageProcessingDialog::ImageProcessingDialog( QWidget* parent /*= nullptr */ )
    : QDialog( parent )
{
    setWindowTitle( "Image Morphology Tool" );

    // Layout and widgets
    QVBoxLayout* mainLayout = new QVBoxLayout( this );

    QHBoxLayout* centerLayout = new QHBoxLayout( this );
    mainLayout->addLayout( centerLayout );

    QVBoxLayout* settingsLayout = new QVBoxLayout( this );
    centerLayout->addLayout( settingsLayout );

    QLabel* kernelLabel = new QLabel( "Kernel Size:", this );
    settingsLayout->addWidget( kernelLabel );

    kernelSpinBox = new QSpinBox( this );
    kernelSpinBox->setRange( 1, 31 );
    kernelSpinBox->setValue( 3 );
    settingsLayout->addWidget( kernelSpinBox );

    QLabel* transparencyLabel = new QLabel( "Transparency (0-100):", this );
    settingsLayout->addWidget( transparencyLabel );

    transparencySlider = new QSlider( Qt::Horizontal, this );
    transparencySlider->setRange( 0, 100 );
    transparencySlider->setValue( 90 );
    settingsLayout->addWidget( transparencySlider );

    showInput = new QCheckBox( "Show Input Image", this );
    showInput->setChecked( true );
    settingsLayout->addWidget( showInput );

    showDilated = new QCheckBox( "Show Dilated Image", this );
    showDilated->setChecked( false );
    settingsLayout->addWidget( showDilated );

    showEroded = new QCheckBox( "Show Eroded Image", this );
    showEroded->setChecked( false );
    settingsLayout->addWidget( showEroded );

    showFinal = new QCheckBox( "Show Final Image", this );
    showFinal->setChecked( true );
    settingsLayout->addWidget( showFinal );

    settingsLayout->addStretch();

    graphicsView  = new QGraphicsView( this );
    graphicsScene = new QGraphicsScene( this );
    graphicsView->setScene( graphicsScene );
    centerLayout->addWidget( graphicsView );

    // Give more space to graphics view
    centerLayout->setStretch( 0, 3 );
    centerLayout->setStretch( 1, 7 );

    // Create a button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
    mainLayout->addWidget( buttonBox );

    // Connect the button box signals
    connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

    // Connect signals and slots
    connect( showInput, &QCheckBox::clicked, this, &ImageProcessingDialog::showImages );
    connect( showEroded, &QCheckBox::clicked, this, &ImageProcessingDialog::showImages );
    connect( showDilated, &QCheckBox::clicked, this, &ImageProcessingDialog::showImages );
    connect( showFinal, &QCheckBox::clicked, this, &ImageProcessingDialog::showImages );

    connect( kernelSpinBox, &QSpinBox::valueChanged, this, &ImageProcessingDialog::updateAndShowImages );
    connect( transparencySlider, &QSlider::valueChanged, this, &ImageProcessingDialog::updateAndShowImages );

    resizeAndCenterDialog( 0.6 ); // 1.0 means to the full screen size
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::performDilation()
{
    if ( sourceData.empty() )
    {
        QMessageBox::warning( this, "Error", "No image loaded." );
        return;
    }

    int kernelSize = kernelAdjustedSize();
    dilatedData    = RigPolygonTools::dilate( sourceData, kernelSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::performErosion()
{
    if ( sourceData.empty() )
    {
        QMessageBox::warning( this, "Error", "No image loaded." );
        return;
    }

    int kernelSize = kernelAdjustedSize();
    erodedData     = RigPolygonTools::erode( sourceData, kernelSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::updateAndShowImages()
{
    performErosion();
    performDilation();
    computeFinal();
    showImages();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::showImages()
{
    if ( sourceData.empty() )
    {
        QMessageBox::warning( this, "Error", "No image loaded." );
        return;
    }

    const auto transparency = 180;
    auto       original     = RicCreateContourMapPolygonTools::convertBinaryToGrayscaleImage( sourceData, transparency );
    auto       dilatedImage = RicCreateContourMapPolygonTools::convertBinaryToImage( dilatedData, QColorConstants::Green, transparency );
    auto       erodedImage  = RicCreateContourMapPolygonTools::convertBinaryToImage( erodedData, QColorConstants::Red, transparency );
    auto       finalImage   = RicCreateContourMapPolygonTools::convertBinaryToImage( processedData, QColorConstants::Yellow, transparency );

    graphicsScene->clear();

    // Add a black background image
    if ( !original.isNull() )
    {
        QImage blackImage( original.size(), QImage::Format_RGB32 );
        blackImage.fill( Qt::black );
        QPixmap              basePixmap = QPixmap::fromImage( blackImage );
        QGraphicsPixmapItem* baseItem   = new QGraphicsPixmapItem( basePixmap );
        graphicsScene->addItem( baseItem );
    }

    double alpha = transparencySlider->value() / 100.0;

    if ( showInput->isChecked() )
    {
        QPixmap              basePixmap = QPixmap::fromImage( original );
        QGraphicsPixmapItem* baseItem   = new QGraphicsPixmapItem( basePixmap );
        baseItem->setOpacity( alpha );
        graphicsScene->addItem( baseItem );
    }

    if ( showDilated->isChecked() && !dilatedImage.isNull() )
    {
        QPixmap              dilatePixmap = QPixmap::fromImage( dilatedImage );
        QGraphicsPixmapItem* dilateItem   = new QGraphicsPixmapItem( dilatePixmap );
        dilateItem->setOpacity( alpha );
        graphicsScene->addItem( dilateItem );
    }

    if ( showEroded->isChecked() && !erodedImage.isNull() )
    {
        QPixmap              erodePixmap = QPixmap::fromImage( erodedImage );
        QGraphicsPixmapItem* erodeItem   = new QGraphicsPixmapItem( erodePixmap );
        erodeItem->setOpacity( alpha );
        graphicsScene->addItem( erodeItem );
    }

    if ( showFinal->isChecked() && !finalImage.isNull() )
    {
        QPixmap              erodePixmap = QPixmap::fromImage( finalImage );
        QGraphicsPixmapItem* erodeItem   = new QGraphicsPixmapItem( erodePixmap );
        erodeItem->setOpacity( alpha );
        graphicsScene->addItem( erodeItem );
    }

    graphicsView->fitInView( graphicsScene->sceneRect(), Qt::KeepAspectRatio );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> ImageProcessingDialog::processedImageData() const
{
    return processedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::setSourceImageData( std::vector<std::vector<int>> imageData )
{
    sourceData = imageData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int ImageProcessingDialog::kernelAdjustedSize() const
{
    // The kernel size should be odd and positive
    return kernelSpinBox->value() * 2 - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::computeFinal()
{
    if ( sourceData.empty() )
    {
        QMessageBox::warning( this, "Error", "No image loaded." );
        return;
    }

    auto floodFilled = RigPolygonTools::fillInterior( sourceData );

    int  kernelSize = kernelAdjustedSize();
    auto eroded     = RigPolygonTools::erode( floodFilled, kernelSize );
    auto dilated    = RigPolygonTools::dilate( eroded, kernelSize );
    processedData   = dilated;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::resizeAndCenterDialog( double scale )
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if ( !screen ) return;

    QRect screenGeometry = screen->geometry();

    int width  = static_cast<int>( screenGeometry.width() * scale );
    int height = static_cast<int>( screenGeometry.height() * scale );

    resize( width, height );

    int x = screenGeometry.x() + ( screenGeometry.width() - width ) / 2;
    int y = screenGeometry.y() + ( screenGeometry.height() - height ) / 2;

    move( x, y );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ImageProcessingDialog::resizeEvent( QResizeEvent* event )
{
    QDialog::resizeEvent( event );
    showImages();
}
