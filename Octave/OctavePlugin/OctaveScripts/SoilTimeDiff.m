
SOIL = riGetActiveCellProperty("SOIL");

SOILDIFF = SOIL;

# Calculate change of oil saturation from timestep to timestep
i = 0;
for timestep = SOIL
	if (i > 0) 
		SOILDIFF(:,i) = timestep - SOIL(:,i);
	endif
	i++;
endfor
SOILDIFF(:,i) = 0;

riSetActiveCellProperty(SOILDIFF, "SOILDIFF");

