addpath("/home/builder/Projects/ResInsightBuildDir/OctavePlugin");
PORO = riGetActiveCellProperty("PORO");
PERMX = riGetActiveCellProperty("PERMX");
IJK = riGetMainGridDimensions();

GENERATED = PORO .* PERMX;

GENERATED(10:IJK(1):IJK(1)*IJK(2)*IJK(3)) = 100;
GENERATED(10*IJK(1):1:11*IJK(1)) = 100;
GENERATED(11*IJK(1):1:12*IJK(1)) = 100;
GENERATED(12*IJK(1):1:13*IJK(1)) = 100;
riSetActiveCellProperty(GENERATED, "PORO*PERMX");