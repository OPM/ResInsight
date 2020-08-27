-- This reservoir simulation deck is made available under the Open Database
-- License: http://opendatacommons.org/licenses/odbl/1.0/. Any rights in
-- individual contents of the database are licensed under the Database Contents
-- License: http://opendatacommons.org/licenses/dbcl/1.0/

-- Copyright (C) 2015 Statoil

-- Layer 8
EQUALS
--  'MULTZ'  0.005    6 18  1 30  8  8 /
'MULTZ'  0.02    6 13 30 50  8  8 /
/

-- MZ layer 10
EQUALS
  'MULTZ'   0.005  6  14  11  18  10  10  /  C-3H
  'MULTZ'   0.03  14  29  11  25  10  10  /  C south east
  'MULTZ'   0.05  14  25  26  30  10  10  /  C-segm mid/B-2H
  'MULTZ'   0.25   6  29  11  37  10  10  /  C-segm middle
  'MULTZ'   0.5   17  17  42  54  10  10  /  C north west
  'MULTZ'   0.5    6  12  38  39  10  10  /  C north west
  'MULTZ'   0.5    8  12  40  40  10  10  /  C north west
  'MULTZ'   0.5   10  12  41  43  10  10  /  C north west   
  'MULTZ'   0.5   18  33  38  54  10  10  /  C1, D4 & D3
  'MULTZ'   0.5    6  13  44  52  10  10  /  B-4AH
  'MULTZ'   0.01  13  13  48  59  10  10  /  D-segm mid (B-4BH)
  'MULTZ'   0.01  14  14  49  59  10  10  /  D-segm mid (B-4BH)
  'MULTZ'   0.01  15  16  51  59  10  10  /  D-segm mid (B-4BH)
  'MULTZ'   0.05  17  19  55  99  10  10  /  E1
  'MULTZ'   0.05  14  14  60  62  10  10  /  E1
  'MULTZ'   0.05  15  15  60  65  10  10  /  E1
  'MULTZ'   0.05  16  16  60  69  10  10  /  E1 
  'MULTZ'   0.005  6   9  52  60  10  10  /  F-3H/E-2H
  'MULTZ'   0.005  9   9  53  57  10  10  /  F-3H/E-2H
  'MULTZ'   0.005 10  10  54  58  10  10  /  F-3H/E-2H
  'MULTZ'   0.005 11  11  55  58  10  10  /  F-3H/E-2H
/

-- MZ layer 15
EQUALS
  'MULTZ'   0.00003     6 29 11 21 15 15 / C south
  'MULTZ'   0.00005   6  29  22  39  15  15 /  C middle
  'MULTZ'   0.000001 19  29  39  49  15  15 /  C-1H
  'MULTZ'   1.0      19  29  38  45  17  17 /  C-1H
  'MULTZ'   0.005    16  19  48  61  15  15 /  E-1H/D-3H
  'MULTZ'   0.0008    8  18  40  40  15  15 /  C north
  'MULTZ'   0.0008    9  18  41  41  15  15 /
  'MULTZ'   0.0008   10  18  42  43  15  15 /
  'MULTZ'   0.0008   11  18  44  44  15  15 /
  'MULTZ'   0.0008   12  18  45  45  15  15 /
  'MULTZ'   0.0008   13  18  46  47  15  15 /
  'MULTZ'   0.0008   14  15  48  48  15  15 /
  'MULTZ'   0.0008   15  15  49  50  15  15 /
  
  'MULTZ'   0.01      12 12 46 56 15 15 / D-segm
  'MULTZ'   0.01      13 13 48 59 15 15 / D-segm
  'MULTZ'   0.01      14 14 49 62 15 15 / D-segm
  'MULTZ'   0.01      15 15 51 65 15 15 / D-segm
  'MULTZ'   0.01      16 19 62 69 15 15 / D-segm
  'MULTZ'   0.01      17 19 70 99 15 15 / D-segm
   MULTZ    0.0035      6  7 40 60 15 15 / D, E west
   MULTZ    0.0035      8  8 41 60 15 15 /
   MULTZ    0.0035      9  9 42 52 15 15 /
   MULTZ    0.0035     10 10 44 49 15 15 /
/

-- D-1H water
EQUALS
  'MULTZ'    1.0  22  24  21  22  11  11 /  
  'MULTZ'    0.1  21  25  17  19  15  15 /    
  'MULTZ'    1.0  22  24  17  19  17  17 / 
  'MULTZ'    1.0  22  24  15  17  18  18 /   
/  

-- B-1 & B-3 water
EQUALS
  'MULTZ'    0.1 12  13  34  35  15  15 /  
/ 

-- RFT D_-H
EQUALS
  'MULTZ'  0.1     16 19 47 53 18 18 /  D-3H
/
