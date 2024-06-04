# Keep the values in the first LGR only

CInfo = riGetActiveCellInfo();
SOIL = riGetActiveCellProperty("SOIL");

Mask = (CInfo(:,1) != 1);

LGRSOIL = SOIL;
i = 0;
for i = (1:columns(LGRSOIL))
    LGRSOIL(Mask,i) = nan;
endfor

riSetActiveCellProperty(LGRSOIL, "LGRSOIL");