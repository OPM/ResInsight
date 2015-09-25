/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


#include "cvfBase.h"

#include "gtest/gtest.h"
#include <stdio.h>

#include "cvfTrace.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv) 
{
    //printf("Running main() from LibCore_UnitTests.cpp\n");
    //printf("LibCore version: %s.%s-%s (%s) \n\n", CVF_MAJOR_VERSION, CVF_MINOR_VERSION, CVF_BUILD_NUMBER, CVF_SPECIAL_BUILD);

    cvf::Assert::setReportMode(cvf::Assert::CONSOLE);

    testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

    std::cout << "Please press <Enter> to close the window.";
    std::cin.get();

    return result;
}
