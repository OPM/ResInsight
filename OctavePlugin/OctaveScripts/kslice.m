# Set all values to "Undefined" exept k-layers from 17 to 20

CInfo = riGetActiveCellInfo();
SOIL = riGetActiveCellProperty("SOIL");

Mask = (CInfo(:,4) < 17) | (CInfo(:,4) > 20)
LGRSOIL = SOIL;
i = 0;
for i = (1:columns(LGRSOIL))
    LGRSOIL(Mask,i) = nan;
endfor

riSetActiveCellProperty(LGRSOIL, "KSlice");