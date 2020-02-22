addpath("/home/builder/Projects/ResInsightBuildDir/OctavePlugin");

tic();
PORO = riGetActiveCellProperty("PORO");
PERMX = riGetActiveCellProperty("PERMX");
IJK = riGetMainGridDimensions();

GENERATED = PORO .* PERMX;

riSetActiveCellProperty(GENERATED, "PORO*PERMX");

elapsed_time = toc();
disp("Elapsed time: "), disp(elapsed_time)