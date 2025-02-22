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

#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

class ImageProcessingDialog : public QDialog
{
    Q_OBJECT

public:
    ImageProcessingDialog( QWidget* parent = nullptr );

    std::vector<std::vector<int>> processedImageData() const;
    void                          setSourceImageData( std::vector<std::vector<int>> imageData );

    int areaThreshold() const;

public slots:
    void updateAndShowImages();

private slots:
    void performDilation();
    void performErosion();
    void showImages();

private:
    int  kernelAdjustedSize() const;
    void computeFinal();
    void resizeAndCenterDialog( double scale );

    void resizeEvent( QResizeEvent* event ) override;

private:
    QSpinBox*       kernelSpinBox;
    QSlider*        transparencySlider;
    QGraphicsView*  graphicsView;
    QGraphicsScene* graphicsScene;

    QCheckBox* showInput;
    QCheckBox* showDilated;
    QCheckBox* showEroded;
    QCheckBox* showFinal;

    QLineEdit* areaThresholdLineEdit;

    std::vector<std::vector<int>> sourceData, dilatedData, erodedData, processedData;
};
