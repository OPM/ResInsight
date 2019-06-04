addpath("C:/cmake_build/ResInsightQt5/OctavePlugin");

tic();

timeSteps = riGetTimeStepDates(0)
disp("Number of time steps: "), disp(size(timeSteps))
for i = 1:size(timeSteps)
  SOIL = riGetActiveCellProperty(0, "SOIL", i);
  avg = mean(SOIL)
endfor
elapsed_time = toc();
disp("Elapsed time: "), disp(elapsed_time)