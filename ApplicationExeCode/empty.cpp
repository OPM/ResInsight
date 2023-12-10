//
// This empty file is used by ResInsightDummyTarget
//
// Creating a dummy target with an empty file is used to have a different target than ResInsight.
// This target is used to find the full path to the build folder.
//
// $<TARGET_FILE_DIR:ResInsightDummyTarget>
//
// If we use the ResInsight target ($<TARGET_FILE_DIR:ResInsight>), we get a circular dependency.
//
