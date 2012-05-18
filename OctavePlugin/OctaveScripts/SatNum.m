addpath("/home/builder/Projects/ResInsightBuildDir/OctavePlugin");
SATNUM = riGetActiveCellProperty("MSW_CASE", "SATNUM");
SOIL = riGetActiveCellProperty("MSW_CASE", "SOIL");

# Set all SOIL data to 0.0 for all cells with SATNUM different 
# from 7 for timestep 1

GENERATED = (SATNUM == 7) .* SOIL(:, 1);

riSetActiveCellProperty(GENERATED, "MSW_CASE", "SOIL_IN_SATNUM_Eq_7");