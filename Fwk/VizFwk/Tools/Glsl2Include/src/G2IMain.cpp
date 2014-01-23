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


#include "G2ICodeFile.h"
#include "G2IUtils.h"
#include <string>
#include <iostream>
#include <fstream>

#define G2I_VERSION_STRING "1.1"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv) 
{
    std::cout << "Running Glsl2Include, ver " << G2I_VERSION_STRING << std::endl;

    if (argc < 2)
    {
        std::cerr << "Missing source directory!" << std::endl;
        return EXIT_FAILURE;
    }

    if (argc < 3)
    {
        std::cerr << "Missing output file name!" << std::endl;
        return EXIT_FAILURE;
    }


    std::string sourceDirectory = argv[1];
    std::string outputFileName = argv[2];
    std::string tempOutputFileName = outputFileName + ".temp";
    std::cout << "sourceDirectory: " << sourceDirectory << std::endl;
    std::cout << "outputFile:      " << outputFileName << std::endl;
    std::cout << "tempOutputFile:  " << tempOutputFileName << std::endl;


    std::vector<std::string> sourceFiles = G2IUtils::getFilesInDirectorySorted(sourceDirectory, "*.glsl");
    if (sourceFiles.size() == 0)
    {
        std::cerr << "No .glsl files found in source directory: " << sourceDirectory << std::endl;
        return EXIT_FAILURE;
    }

    std::ofstream tempOutFile(tempOutputFileName);
    if (!tempOutFile.is_open())
    {
        std::cerr << "Error opening output file for writing: " << tempOutputFileName << std::endl;
        return EXIT_FAILURE;
    }

    tempOutFile << std::endl;
    tempOutFile << "// -----------------------------------------" << std::endl;
    tempOutFile << "// THIS IS A GENERATED FILE!!  DO NOT MODIFY" << std::endl;
    tempOutFile << "// -----------------------------------------" << std::endl;

    size_t numSrcFiles = sourceFiles.size();
    size_t ifile;
    for (ifile = 0; ifile < numSrcFiles; ifile++)
    {
        std::string inputFileName = sourceDirectory + sourceFiles[ifile];

        std::string baseName = G2IUtils::extractBaseName(inputFileName);
        std::string variableName = baseName + "_inl";
        //std::cout << "  inputFile:    " << inputFileName << std::endl;
        //std::cout << "  variableName: " << variableName << std::endl;
        
        CodeFile codeFile;
        if (!codeFile.readFile(inputFileName))
        {
            std::cerr << "Error reading input file: " << inputFileName << std::endl;
            return EXIT_FAILURE;
        }

        codeFile.trimAndConvertToSpaces();
        codeFile.removeBottomEmptyLines();
        codeFile.padAndQuote();

        if (!codeFile.appendToOpenFile(&tempOutFile, variableName))
        {
            std::cerr << "Error writing output file: " << tempOutputFileName << std::endl;
            return EXIT_FAILURE;
        }
    }

    tempOutFile.close();

    if (G2IUtils::fileContensEqual(tempOutputFileName, outputFileName))
    {
        std::cout << "No changes detected." << std::endl;
    }
    else
    {
        std::cout << "Detected changes, need to write output file: " << outputFileName << std::endl;
        std::cout << "Copying...";
        if (G2IUtils::copyFile(tempOutputFileName, outputFileName))
        {
            std::cout << "ok" << std::endl;
        }
        else
        {
            std::cout << "FAILED!" << std::endl;
            std::cout << "Error writing to:" << outputFileName << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
