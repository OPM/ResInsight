
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RifEclipseDataTableFormatter.h
${CEE_CURRENT_LIST_DIR}RifEclipseInputFileTools.h
${CEE_CURRENT_LIST_DIR}RifEclipseOutputFileTools.h
${CEE_CURRENT_LIST_DIR}RifEclipseRestartDataAccess.h
${CEE_CURRENT_LIST_DIR}RifEclipseRestartFilesetAccess.h
${CEE_CURRENT_LIST_DIR}RifEclipseSummaryTools.h
${CEE_CURRENT_LIST_DIR}RifEclipseUnifiedRestartFileAccess.h
${CEE_CURRENT_LIST_DIR}RifPerforationIntervalReader.h
${CEE_CURRENT_LIST_DIR}RifReaderEclipseInput.h
${CEE_CURRENT_LIST_DIR}RifReaderEclipseOutput.h
${CEE_CURRENT_LIST_DIR}RifSummaryReaderInterface.h
${CEE_CURRENT_LIST_DIR}RifEclipseUserDataParserTools.h
${CEE_CURRENT_LIST_DIR}RifColumnBasedUserDataParser.h
${CEE_CURRENT_LIST_DIR}RifKeywordVectorParser.h
${CEE_CURRENT_LIST_DIR}RifReaderObservedData.h
${CEE_CURRENT_LIST_DIR}RifReaderEclipseSummary.h
${CEE_CURRENT_LIST_DIR}RifReaderEclipseRft.h
${CEE_CURRENT_LIST_DIR}RifJsonEncodeDecode.h
${CEE_CURRENT_LIST_DIR}RifReaderInterface.h
${CEE_CURRENT_LIST_DIR}RifReaderMockModel.h
${CEE_CURRENT_LIST_DIR}RifReaderSettings.h
${CEE_CURRENT_LIST_DIR}RifEclipseSummaryAddress.h
${CEE_CURRENT_LIST_DIR}RifEclipseSummaryAddressQMetaType.h
${CEE_CURRENT_LIST_DIR}RifEclipseRftAddress.h
${CEE_CURRENT_LIST_DIR}RifWellPathImporter.h
${CEE_CURRENT_LIST_DIR}RifHdf5ReaderInterface.h
${CEE_CURRENT_LIST_DIR}RifColumnBasedUserData.h
${CEE_CURRENT_LIST_DIR}RifKeywordVectorUserData.h
${CEE_CURRENT_LIST_DIR}RifDataSourceForRftPlt.h
${CEE_CURRENT_LIST_DIR}RifDataSourceForRftPltQMetaType.h
${CEE_CURRENT_LIST_DIR}RifEclipseUserDataKeywordTools.h
${CEE_CURRENT_LIST_DIR}RifCsvUserData.h
${CEE_CURRENT_LIST_DIR}RifCsvUserDataParser.h
${CEE_CURRENT_LIST_DIR}RifWellPathFormationReader.h
${CEE_CURRENT_LIST_DIR}RifWellPathFormationsImporter.h
${CEE_CURRENT_LIST_DIR}RifElementPropertyTableReader.h
${CEE_CURRENT_LIST_DIR}RifElementPropertyReader.h
${CEE_CURRENT_LIST_DIR}RifStimPlanXmlReader.h

# HDF5 file reader is directly included in ResInsight main CmakeList.txt
#${CEE_CURRENT_LIST_DIR}RifHdf5Reader.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RifEclipseDataTableFormatter.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseInputFileTools.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseOutputFileTools.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseRestartDataAccess.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseRestartFilesetAccess.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseUnifiedRestartFileAccess.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseSummaryTools.cpp
${CEE_CURRENT_LIST_DIR}RifPerforationIntervalReader.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseInput.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseOutput.cpp
${CEE_CURRENT_LIST_DIR}RifSummaryReaderInterface.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseUserDataParserTools.cpp
${CEE_CURRENT_LIST_DIR}RifColumnBasedUserDataParser.cpp
${CEE_CURRENT_LIST_DIR}RifKeywordVectorParser.cpp
${CEE_CURRENT_LIST_DIR}RifReaderObservedData.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseSummary.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseRft.cpp
${CEE_CURRENT_LIST_DIR}RifJsonEncodeDecode.cpp
${CEE_CURRENT_LIST_DIR}RifReaderInterface.cpp
${CEE_CURRENT_LIST_DIR}RifReaderMockModel.cpp
${CEE_CURRENT_LIST_DIR}RifReaderSettings.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseSummaryAddress.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseRftAddress.cpp
${CEE_CURRENT_LIST_DIR}RifWellPathImporter.cpp
${CEE_CURRENT_LIST_DIR}RifHdf5ReaderInterface.cpp
${CEE_CURRENT_LIST_DIR}RifColumnBasedUserData.cpp
${CEE_CURRENT_LIST_DIR}RifKeywordVectorUserData.cpp
${CEE_CURRENT_LIST_DIR}RifDataSourceForRftPlt.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseUserDataKeywordTools.cpp
${CEE_CURRENT_LIST_DIR}RifCsvUserData.cpp
${CEE_CURRENT_LIST_DIR}RifCsvUserDataParser.cpp
${CEE_CURRENT_LIST_DIR}RifWellPathFormationReader.cpp
${CEE_CURRENT_LIST_DIR}RifWellPathFormationsImporter.cpp
${CEE_CURRENT_LIST_DIR}RifElementPropertyTableReader.cpp
${CEE_CURRENT_LIST_DIR}RifElementPropertyReader.cpp
${CEE_CURRENT_LIST_DIR}RifStimPlanXmlReader.cpp

# HDF5 file reader is directly included in ResInsight main CmakeList.txt
#${CEE_CURRENT_LIST_DIR}RifHdf5Reader.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "FileInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
