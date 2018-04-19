
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RifEclipseDataTableFormatter.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseInputFileTools.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseOutputFileTools.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRestartDataAccess.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRestartFilesetAccess.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryTools.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUnifiedRestartFileAccess.h
${CMAKE_CURRENT_LIST_DIR}/RifPerforationIntervalReader.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseInput.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseOutput.h
${CMAKE_CURRENT_LIST_DIR}/RifSummaryReaderInterface.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUserDataParserTools.h
${CMAKE_CURRENT_LIST_DIR}/RifColumnBasedUserDataParser.h
${CMAKE_CURRENT_LIST_DIR}/RifKeywordVectorParser.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderObservedData.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseSummary.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseRft.h
${CMAKE_CURRENT_LIST_DIR}/RifJsonEncodeDecode.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderInterface.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderMockModel.h
${CMAKE_CURRENT_LIST_DIR}/RifReaderSettings.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryAddress.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryAddressQMetaType.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRftAddress.h
${CMAKE_CURRENT_LIST_DIR}/RifWellPathImporter.h
${CMAKE_CURRENT_LIST_DIR}/RifHdf5ReaderInterface.h
${CMAKE_CURRENT_LIST_DIR}/RifColumnBasedUserData.h
${CMAKE_CURRENT_LIST_DIR}/RifKeywordVectorUserData.h
${CMAKE_CURRENT_LIST_DIR}/RifDataSourceForRftPlt.h
${CMAKE_CURRENT_LIST_DIR}/RifDataSourceForRftPltQMetaType.h
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUserDataKeywordTools.h
${CMAKE_CURRENT_LIST_DIR}/RifCsvUserData.h
${CMAKE_CURRENT_LIST_DIR}/RifCsvUserDataParser.h
${CMAKE_CURRENT_LIST_DIR}/RifWellPathFormationReader.h
${CMAKE_CURRENT_LIST_DIR}/RifWellPathFormationsImporter.h
${CMAKE_CURRENT_LIST_DIR}/RifElementPropertyTableReader.h
${CMAKE_CURRENT_LIST_DIR}/RifElementPropertyReader.h
${CMAKE_CURRENT_LIST_DIR}/RifStimPlanXmlReader.h
${CMAKE_CURRENT_LIST_DIR}/RifSummaryCaseRestartSelector.h
${CMAKE_CURRENT_LIST_DIR}/RifEnsembleParametersReader.h
${CMAKE_CURRENT_LIST_DIR}/RifCaseRealizationParametersReader.h
${CMAKE_CURRENT_LIST_DIR}/RifFileParseTools.h

# HDF5 file reader is directly included in ResInsight main CmakeList.txt
#${CMAKE_CURRENT_LIST_DIR}/RifHdf5Reader.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RifEclipseDataTableFormatter.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseInputFileTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseOutputFileTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRestartDataAccess.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRestartFilesetAccess.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUnifiedRestartFileAccess.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RifPerforationIntervalReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseInput.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseOutput.cpp
${CMAKE_CURRENT_LIST_DIR}/RifSummaryReaderInterface.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUserDataParserTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RifColumnBasedUserDataParser.cpp
${CMAKE_CURRENT_LIST_DIR}/RifKeywordVectorParser.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderObservedData.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseSummary.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseRft.cpp
${CMAKE_CURRENT_LIST_DIR}/RifJsonEncodeDecode.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderInterface.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderMockModel.cpp
${CMAKE_CURRENT_LIST_DIR}/RifReaderSettings.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryAddress.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseRftAddress.cpp
${CMAKE_CURRENT_LIST_DIR}/RifWellPathImporter.cpp
${CMAKE_CURRENT_LIST_DIR}/RifHdf5ReaderInterface.cpp
${CMAKE_CURRENT_LIST_DIR}/RifColumnBasedUserData.cpp
${CMAKE_CURRENT_LIST_DIR}/RifKeywordVectorUserData.cpp
${CMAKE_CURRENT_LIST_DIR}/RifDataSourceForRftPlt.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseUserDataKeywordTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RifCsvUserData.cpp
${CMAKE_CURRENT_LIST_DIR}/RifCsvUserDataParser.cpp
${CMAKE_CURRENT_LIST_DIR}/RifWellPathFormationReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifWellPathFormationsImporter.cpp
${CMAKE_CURRENT_LIST_DIR}/RifElementPropertyTableReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifElementPropertyReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifStimPlanXmlReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifSummaryCaseRestartSelector.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEnsembleParametersReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifCaseRealizationParametersReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RifFileParseTools.cpp

# HDF5 file reader is directly included in ResInsight main CmakeList.txt
#${CMAKE_CURRENT_LIST_DIR}/RifHdf5Reader.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "FileInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
