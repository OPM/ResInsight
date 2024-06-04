
SATNUM = riGetActiveCellProperty("SATNUM");
SOIL = riGetActiveCellProperty( "SOIL");

# Set all SOIL data to 0.0 for all cells with SATNUM different 
# from 7 for timestep 1

GENERATED = (SATNUM == 7) .* SOIL(:, 1);

riSetActiveCellProperty(GENERATED, "SOIL_IN_SATNUM_Eq_7");