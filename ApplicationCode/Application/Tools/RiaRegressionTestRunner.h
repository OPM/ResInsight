/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include <QSize>
#include <QStringList>

class QDir;

//==================================================================================================
//
//==================================================================================================
class RiaRegressionTestRunner
{
public:
    static RiaRegressionTestRunner* instance();

    void executeRegressionTests(const QString& regressionTestPath, const QStringList& testFilter);
    void executeRegressionTests();

    bool isRunningRegressionTests() const;

    static void updateRegressionTest(const QString& testRootPath);

private:
    RiaRegressionTestRunner();

    void runRegressionTest(const QString& testRootPath, const QStringList& testFilter);

    static void  removeDirectoryWithContent(QDir& dirToDelete);
    static void  regressionTestConfigureProject();
    static void  resizeMaximizedPlotWindows();
    static QSize regressionDefaultImageSize();

private:
    const QString     m_rootPath;
    const QStringList m_testFilter;
    bool              m_runningRegressionTests;
};
