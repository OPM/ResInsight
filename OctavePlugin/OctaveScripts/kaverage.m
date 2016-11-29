
CInfo = riGetActiveCellInfo();
SOIL = riGetActiveCellProperty("SOIL");

SOIL_KAverage = SOIL;
SOIL_KAverage(:) = nan;

mini = min(CInfo(:,2))
maxi = max(CInfo(:,2))

minj = min(CInfo(:,3))
maxj = max(CInfo(:,3))

for i = mini:maxi
  for j = minj:maxj 
    Mask = (CInfo(:,1) == 0) & (CInfo(:,2) == i) & (CInfo(:,3) == j) ;

    for ts = (1:columns(SOIL)) 
      SOIL_KAverage(Mask, ts) = mean(SOIL(Mask, ts));
    endfor
  endfor
endfor

riSetActiveCellProperty(SOIL_KAverage, "SOIL_KAverage");