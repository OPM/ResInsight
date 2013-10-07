### The case with caseid 1 has to be selected/active in ResInsight when running this test-script
### Coarsening and Dual porosity is not exercised by this tes yet. We need models

### CaseInfo riGetCurrentCase()
%!test
%! printf ("===== Testing ====> riGetCurrentCase\n");
%! caseInfo = riGetCurrentCase();
%! assert ( caseInfo.CaseId == 1 );
%! assert ( caseInfo.CaseName == "BRUGGE_0000" );
%! assert ( caseInfo.CaseType == "SourceCase" );
%! assert ( caseInfo.CaseGroupId == 0 );

### Vector[CaseInfo] riGetSelectedCases()
%!test
%! printf ("===== Testing ====> riGetSelectedCases\n");
%! caseInfoVector1 = riGetSelectedCases();
%! assert ( caseInfoVector1.CaseId == 1 );
%! assert ( caseInfoVector1.CaseName == "BRUGGE_0000" );
%! assert ( caseInfoVector1.CaseType == "SourceCase" );
%! assert ( caseInfoVector1.CaseGroupId == 0 );
 

### Vector[CaseGroupInfo] riGetCaseGroups()
%!test
%! printf ("===== Testing ====> riGetCaseGroups\n");
%! caseGroupInfoVector = riGetCaseGroups();
%! assert (rows(caseGroupInfoVector) == 2);
%! assert (caseGroupInfoVector(2).CaseGroupId == 1);
%! assert (caseGroupInfoVector(2).CaseGroupName == "Grid Case Group 2");

### Vector[CaseInfo] riGetCases([CaseGroupId])
%!test
%! printf ("===== Testing ====> riGetCases\n");
%! caseInfoVector3 = riGetCases();
%! assert(rows(caseInfoVector3) == 10);
%! assert(caseInfoVector3(2).CaseName == "BRUGGE_0040");
%! assert(caseInfoVector3(2).CaseType == "ResultCase");
%! assert(caseInfoVector3(3).CaseType == "StatisticsCase");
%! assert(caseInfoVector3(4).CaseType == "SourceCase");
%! caseinfoVector3 = riGetCases(1);
%! assert(rows(caseinfoVector3) == 3);


### Matrix[numActiveCells][9] riGetActiveCellInfo([CaseId], [PorosityModel = "Matrix"|"Fracture"] )
%!test
%! printf ("===== Testing ====> riGetActiveCellInfo\n");
%! ACInfo1 = riGetActiveCellInfo();
%! assert(rows(ACInfo1) == 43374);
%! assert(columns(ACInfo1) == 9);
%! ACInfo2 = riGetActiveCellInfo("Matrix");
%! assert(ACInfo1 == ACInfo2);
%! ACInfo3 = riGetActiveCellInfo(1, "Matrix");
%! assert(ACInfo1 == ACInfo3);
%! ACInfo4 = riGetActiveCellInfo(1);
%! assert(ACInfo1 == ACInfo4);

### Matrix[numCoarseGroups][6] riGetCoarseningInfo([CaseId])
%!xtest
%! printf ("===== Testing ====> riGetCoarseningInfo\n");
%! CoarseInfo1 = riGetCoarseningInfo();
%! assert(rows(CoarseInfo1) == 0);
%! assert(columns(CoarseInfo1) == 6);
%! CoarseInfo2 = riGetCoarseningInfo(1);
%! assert(CoarseInfo1 == CoarseInfo2);

### Matrix[numGrids][3] riGetGridDimensions([CaseId])
%!test
%! printf ("===== Testing ====> riGetGridDimensions\n");
%! GridDims1 = riGetGridDimensions();
%! assert(rows(GridDims1) == 1);
%! assert(columns(GridDims1) == 3);
%! GridDims2 = riGetGridDimensions(0);
%! assert(rows(GridDims2) == 2);
%! assert(columns(GridDims2) == 3);
%! assert( GridDims2(2,1) == 12);
%! assert( GridDims2(2,3) == 36);

### Vector[TimeStepDate] riGetTimestepDates([CaseId])
printf ("===== Testing ====> riGetTimestepDates\n");
TimeStepDates1 = riGetTimeStepDates();
TimeStepDates2 = riGetTimeStepDates(1);

### Vector[DecimalDay] riGetTimestepDays([CaseId])
printf ("===== Testing ====> riGetTimestepDays\n");
TimeStepDays1  = riGetTimeStepDays();
TimeStepDays2  = riGetTimeStepDays(1);

### Vector[PropertyInfo] riGetPropertyNames([CaseId] ], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riGetPropertyNames\n");
PropertyInfos1 = riGetPropertyNames();
PropertyInfos2 = riGetPropertyNames(1);
PropertyInfos3 = riGetPropertyNames("Matrix");
PropertyInfos4 = riGetPropertyNames(1, "Matrix");

### Matrix[numActiveCells][numTimestepsRequested] riGetActiveCellProperty([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riGetActiveCellProperty\n");
ActivePropData1  = riGetActiveCellProperty("SOIL");
ActivePropData2  = riGetActiveCellProperty("SOIL", "Matrix");
ActivePropData3  = riGetActiveCellProperty("SOIL", [1,3]);
ActivePropData4  = riGetActiveCellProperty("SOIL", [1,3], "Matrix");

ActivePropData5  = riGetActiveCellProperty(1, "SOIL");
ActivePropData6  = riGetActiveCellProperty(1, "SOIL", [1,3]);
ActivePropData7  = riGetActiveCellProperty(1, "SOIL", [1,3], "Matrix");
ActivePropData8  = riGetActiveCellProperty(1, "SOIL", "Matrix");

### Matrix[numI][numJ][numK][numTimestepsRequested] riGetGridProperty([CaseId], GridIndex , PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riGetGridProperty\n");
GridProps1 =  riGetGridProperty(   0 , "SOIL" );
GridProps2 =  riGetGridProperty(   0 , "SOIL", [1,3]);
GridProps3 =  riGetGridProperty(   0 , "SOIL", [1,3], "Matrix");
GridProps4 =  riGetGridProperty(   0 , "SOIL",  "Matrix");
GridProps5 =  riGetGridProperty(1, 0 , "SOIL" );
GridProps6 =  riGetGridProperty(1, 0 , "SOIL", [1,3]);
GridProps7 =  riGetGridProperty(1, 0 , "SOIL", [1,3], "Matrix");
GridProps8 =  riGetGridProperty(1, 0 , "SOIL",  "Matrix");


### riSetActiveCellProperty( Matrix[numActiveCells][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riSetActiveCellProperty\n");
riSetActiveCellProperty( ActivePropData1,     "PropertyName1" );

riSetActiveCellProperty( ActivePropData3,     "PropertyName2", [1,3]);
riSetActiveCellProperty( ActivePropData3,     "PropertyName3", [1,3], "Matrix");
riSetActiveCellProperty( ActivePropData1,     "PropertyName4", "Matrix");
riSetActiveCellProperty( ActivePropData1, 1,  "PropertyName5" );
riSetActiveCellProperty( ActivePropData3, 1,  "PropertyName6", [1,3]);
riSetActiveCellProperty( ActivePropData3, 1,  "PropertyName7", [1,3], "Matrix");
riSetActiveCellProperty( ActivePropData1, 1,  "PropertyName8", "Matrix");

### riSetGridProperty( Matrix[numI][numJ][numK][numTimeSteps], [CaseId], GridIndex, PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riSetGridProperty\n");
riSetGridProperty( GridProps1,     0, "PropertyName11" );
riSetGridProperty( GridProps2,     0, "PropertyName12", [1,3]);
riSetGridProperty( GridProps2,     0, "PropertyName13", [1,3], "Matrix");
riSetGridProperty( GridProps1,     0, "PropertyName14", "Matrix");
riSetGridProperty( GridProps1,  1, 0, "PropertyName15" );
riSetGridProperty( GridProps2,  1, 0, "PropertyName16", [1,3]);
riSetGridProperty( GridProps2,  1, 0, "PropertyName17", [1,3], "Matrix");
riSetGridProperty( GridProps1,  1, 0, "PropertyName18", "Matrix");


### Matrix[numI][numJ][numK][3] riGetCellCenters([CaseId], GridIndex)
printf ("===== Testing ====> riGetCellCenters\n");
CellCenters1 = riGetCellCenters(0);
CellCenters2 = riGetCellCenters(1, 0);

### Matrix[ActiveCells][3] riGetActiveCellCenters([CaseId], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riGetActiveCellCenters\n");
ActiveCellCenters1 =  riGetActiveCellCenters();
ActiveCellCenters2 =  riGetActiveCellCenters("Matrix");
ActiveCellCenters3 =  riGetActiveCellCenters(1, "Matrix");

### Matrix[numI][numJ][numK][8][3] riGetCellCorners([CaseId], GridIndex)
printf ("===== Testing ====> riGetCellCorners\n");
CellCorners1 = riGetCellCorners(0);
CellCorners2 = riGetCellCorners(1, 0);

### Matrix[ActiveCells][8][3] riGetActiveCellCorners([CaseId], [PorosityModel = "Matrix"|"Fracture"])
printf ("===== Testing ====> riGetActiveCellCorners\n");
ActiveCellCorners1 =  riGetActiveCellCorners();
ActiveCellCorners2 =  riGetActiveCellCorners(1);
ActiveCellCorners3 =  riGetActiveCellCorners(1, "Matrix");
ActiveCellCorners4 =  riGetActiveCellCorners("Matrix");

### Vector[WellNames] riGetWellNames([CaseId])
printf ("===== Testing ====> riGetWellNames\n");

WellNames1 = riGetWellNames();
WellNames2 = riGetWellNames(1);

### Vector[WellCellInfo] riGetWellCells([CaseId], WellName, TimeStep)
printf ("===== Testing ====> riGetWellCells\n");
WellCellInfos1 = riGetWellCells(1, WellNames1(1,:), 3);
WellCellInfos2 = riGetWellCells(WellNames1(1,:), 3);


### Vector[WellStatus] riGetWellStatus ([CaseId], WellName, [RequestedTimeSteps])
printf ("===== Testing ====> riGetWellStatus\n");
WellStatuses1 = riGetWellStatus(1, WellNames1(1,:), [1,3]);
disp(WellStatuses1(1));
WellStatuses2 = riGetWellStatus( WellNames1(1,:), [1,3]);
WellStatuses3 = riGetWellStatus(WellNames1(1,:));

endif