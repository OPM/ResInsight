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
%!test
%! printf ("===== Testing ====> riGetCoarseningInfo\n");
%! CoarseInfo1 = riGetCoarseningInfo();
%! assert(rows(CoarseInfo1) == 0);
%! assert(columns(CoarseInfo1) == 6);


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
%!test
%! printf ("===== Testing ====> riGetTimestepDates\n");
%! TimeStepDates1 = riGetTimeStepDates();
%! assert(rows(TimeStepDates1) == 11);
%! assert(TimeStepDates1(2).Year  == 1997);
%! assert(TimeStepDates1(2).Month == 01);
%! assert(TimeStepDates1(2).Day	  == 31);
%! assert(TimeStepDates1(2).Hour   == 0);
%! assert(TimeStepDates1(2).Minute == 0);
%! assert(TimeStepDates1(2).Second == 0);
%! TimeStepDates2 = riGetTimeStepDates(1);
%! assert(TimeStepDates2(7).Year  == 1997);
%! assert(TimeStepDates2(7).Month == 06);
%! assert(TimeStepDates2(7).Day	  == 30);
%! assert(TimeStepDates2(7).Hour   == 0);
%! assert(TimeStepDates2(7).Minute == 0);
%! assert(TimeStepDates2(7).Second == 0);

### Vector[DecimalDay] riGetTimestepDays([CaseId])
%!test
%! printf ("===== Testing ====> riGetTimestepDays\n");
%! TimeStepDays1  = riGetTimeStepDays();
%! assert(TimeStepDays1(1) == 0);
%! assert(TimeStepDays1(2) == 30);
%! assert(rows(TimeStepDays1) == 11);
%! TimeStepDays2  = riGetTimeStepDays(1);
%! assert(rows(TimeStepDays2) == 11);


### Vector[PropertyInfo] riGetPropertyNames([CaseId] ], [PorosityModel = "Matrix"|"Fracture"])
%!xtest
%! printf ("===== Testing ====> riGetPropertyNames\n");
%! PropertyInfos1 = riGetPropertyNames();
%! PropertyInfos2 = riGetPropertyNames(1);
%! PropertyInfos3 = riGetPropertyNames("Matrix");
%! PropertyInfos4 = riGetPropertyNames(1, "Matrix");
%! assert(PropertyInfos1(1).PropName == "PRESSURE");
%! assert(PropertyInfos1(1).PropType == "DynamicNative");
%! assert(PropertyInfos1(26).PropType == "StaticNative");

### Matrix[numActiveCells][numTimestepsRequested] riGetActiveCellProperty([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riGetActiveCellProperty\n");
%! ActivePropData1  = riGetActiveCellProperty("SWAT");
%! assert (rows(ActivePropData1) == rows(riGetActiveCellInfo()));
%! assert (columns(ActivePropData1) == rows(riGetTimeStepDays()));
%! ActivePropData2  = riGetActiveCellProperty("SWAT", "Matrix");
%! assert (ActivePropData2 == ActivePropData1);
%! ActivePropData3  = riGetActiveCellProperty("SWAT", [1,3]);
%! assert (columns(ActivePropData3) == 2);
%! assert (ActivePropData3(:,2) == ActivePropData1(:,3));
%! ActivePropData4  = riGetActiveCellProperty("SWAT", [1,3], "Matrix");
%! assert (ActivePropData3 == ActivePropData4);
%! ActivePropData5  = riGetActiveCellProperty(1, "SWAT");
%! assert (ActivePropData5 == ActivePropData1);
%! ActivePropData6  = riGetActiveCellProperty(1, "SWAT", [1,3]);
%! assert (ActivePropData6 == ActivePropData3);
%! ActivePropData7  = riGetActiveCellProperty(1, "SWAT", [1,3], "Matrix");
%! assert (ActivePropData7 == ActivePropData3);
%! ActivePropData8  = riGetActiveCellProperty(1, "SWAT", "Matrix");
%! assert (ActivePropData8 == ActivePropData1);

### Matrix[numI][numJ][numK][numTimestepsRequested] riGetGridProperty([CaseId], GridIndex , PropertyName, [RequestedTimeSteps], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riGetGridProperty\n");
%! GridProps1 =  riGetGridProperty(   0 , "SWAT" );
%! assert( ndims (GridProps1) == 4);
%! [ni, nj, nk, nts ] = size(GridProps1);
%! disp(nts);
%! assert(nts == 11);
%! assert(ni == 139);
%! assert(nj == 48);
%! assert(nk == 9);
%! assert(GridProps1(62,30,1,3), 0.40942, 0.00001);

%! GridProps2 =  riGetGridProperty(   0 , "SWAT", [1,3]);
%! assert( ndims (GridProps2) == 4);
%! [ni, nj, nk, nts ] = size(GridProps2);
%! assert(nts == 2);
%! assert(ni == 139);
%! assert(nj == 48);
%! assert(nk == 9);
%! assert(GridProps2(62,30,1,2), 0.40942, 0.00001);

%! GridProps3 =  riGetGridProperty(   0 , "SWAT", [1,3], "Matrix");
%! GridProps4 =  riGetGridProperty(   0 , "SWAT",  "Matrix");
%! GridProps5 =  riGetGridProperty(1, 0 , "SWAT" );
%! GridProps6 =  riGetGridProperty(1, 0 , "SWAT", [1,3]);
%! GridProps7 =  riGetGridProperty(1, 0 , "SWAT", [1,3], "Matrix");
%! GridProps8 =  riGetGridProperty(1, 0 , "SWAT",  "Matrix");

%! assert(GridProps3 == GridProps2);
%! assert(GridProps4 == GridProps1);
%! assert(GridProps5 == GridProps1);
%! assert(GridProps6 == GridProps2);
%! assert(GridProps7 == GridProps2);
%! assert(GridProps8 == GridProps1);
 
 
 ### riSetActiveCellProperty( Matrix[numActiveCells][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riSetActiveCellProperty\n");
%! ActivePropData1  = riGetActiveCellProperty("SWAT");
%! ActivePropData3  = riGetActiveCellProperty("SWAT", [1,3]);

%! riSetActiveCellProperty( ActivePropData1,     "PropertyName1" );
%! riSetActiveCellProperty( ActivePropData3,     "PropertyName2", [1,3]);
%! riSetActiveCellProperty( ActivePropData3,     "PropertyName3", [1,3], "Matrix");
%! riSetActiveCellProperty( ActivePropData1,     "PropertyName4", "Matrix");
%! riSetActiveCellProperty( ActivePropData1, 1,  "PropertyName5" );
%! riSetActiveCellProperty( ActivePropData3, 1,  "PropertyName6", [1,3]);
%! riSetActiveCellProperty( ActivePropData3, 1,  "PropertyName7", [1,3], "Matrix");
%! riSetActiveCellProperty( ActivePropData1, 1,  "PropertyName8", "Matrix");

%! assert(ActivePropData1 == riGetActiveCellProperty("PropertyName1"));
%! assert(ActivePropData3 == riGetActiveCellProperty("PropertyName2", [1,3]));
%! assert(ActivePropData3 == riGetActiveCellProperty("PropertyName3", [1,3]));
%! assert(ActivePropData1 == riGetActiveCellProperty("PropertyName4", "Matrix"));
%! assert(ActivePropData1 == riGetActiveCellProperty("PropertyName5"));
%! assert(ActivePropData3 == riGetActiveCellProperty( 1, "PropertyName6", [1,3]));
%! assert(ActivePropData3 == riGetActiveCellProperty( 1, "PropertyName7", [1,3], "Matrix"));
%! assert(ActivePropData1 == riGetActiveCellProperty( 1, "PropertyName8", "Matrix"));

### riSetGridProperty( Matrix[numI][numJ][numK][numTimeSteps], [CaseId], GridIndex, PropertyName, [TimeStepIndices], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riSetGridProperty\n");
%! GridProps1 =  riGetGridProperty(   0 , "SWAT" );
%! GridProps2 =  riGetGridProperty(   0 , "SWAT", [1,3]);

%! riSetGridProperty( GridProps1,     0, "PropertyName11" );
%! riSetGridProperty( GridProps2,     0, "PropertyName12", [1,3]);
%! riSetGridProperty( GridProps2,     0, "PropertyName13", [1,3], "Matrix");
%! riSetGridProperty( GridProps1,     0, "PropertyName14", "Matrix");
%! riSetGridProperty( GridProps1,  1, 0, "PropertyName15" );
%! riSetGridProperty( GridProps2,  1, 0, "PropertyName16", [1,3]);
%! riSetGridProperty( GridProps2,  1, 0, "PropertyName17", [1,3], "Matrix");
%! riSetGridProperty( GridProps1,  1, 0, "PropertyName18", "Matrix");

%! assert(GridProps1 == riGetGridProperty(    0, "PropertyName11"));
%! assert(GridProps2 == riGetGridProperty(    0, "PropertyName12", [1,3]));
%! assert(GridProps2 == riGetGridProperty(    0, "PropertyName13", [1,3], "Matrix"));
%! assert(GridProps1 == riGetGridProperty(    0, "PropertyName14", "Matrix"));
%! assert(GridProps1 == riGetGridProperty( 1, 0, "PropertyName15"));
%! assert(GridProps2 == riGetGridProperty( 1, 0, "PropertyName16", [1,3]));
%! assert(GridProps2 == riGetGridProperty( 1, 0, "PropertyName17", [1,3], "Matrix"));
%! assert(GridProps1 == riGetGridProperty( 1, 0, "PropertyName18", "Matrix"));


### Matrix[numI][numJ][numK][3] riGetCellCenters([CaseId], GridIndex)
%!test
%! printf ("===== Testing ====> riGetCellCenters\n");
%! CellCenters1 = riGetCellCenters(0);
%! CellCenters2 = riGetCellCenters(1, 0);
%! assert( ndims (CellCenters1) == 4);
%! [ni, nj, nk, idx ] = size(CellCenters1);
%! assert(idx == 3);
%! assert(ni == 139);
%! assert(nj == 48);
%! assert(nk == 9);
%! assert(CellCenters1(62,30,1, 1), 3489.2, 0.1);
%! assert(CellCenters1(62,30,1, 2), 1.5909e+004, 0.1);
%! assert(CellCenters1(62,30,1, 3), -5458.8, 0.1);
%! assert(CellCenters1 == CellCenters2);



### Matrix[ActiveCells][3] riGetActiveCellCenters([CaseId], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riGetActiveCellCenters\n");
%! ActiveCellCenters1 =  riGetActiveCellCenters();
%! ActiveCellCenters2 =  riGetActiveCellCenters("Matrix");
%! ActiveCellCenters3 =  riGetActiveCellCenters(1, "Matrix");

%! assert (rows(ActiveCellCenters1), rows(riGetActiveCellInfo()));
%! assert (columns(ActiveCellCenters1) == 3);
%! assert (ActiveCellCenters1(500,:) ,[3493.7,   9184.6,  -6074.4], [0.1, 0.1, 0.1]);
%! assert (ActiveCellCenters1 == ActiveCellCenters2);
%! assert (ActiveCellCenters1 == ActiveCellCenters3);


### Matrix[numI][numJ][numK][8][3] riGetCellCorners([CaseId], GridIndex)
%!test
%! printf ("===== Testing ====> riGetCellCorners\n");
%! CellCorners1 = riGetCellCorners(0);
%! CellCorners2 = riGetCellCorners(1, 0);

%! assert( ndims (CellCorners1) == 5);
%! [ni, nj, nk, cidx, idx ] = size(CellCorners1);
%! assert(idx == 3);
%! assert(cidx == 8);
%! assert(ni == 139);
%! assert(nj == 48);
%! assert(nk == 9);
%! assert(CellCorners1(62,30,1, 1, 1), 3207.4, 0.1);
%! assert(CellCorners1(62,30,1, 1, 2), 1.5781e+004, 1);
%! assert(CellCorners1(62,30,1, 1, 3), -5466.1, 0.1);
%! assert (CellCorners1 == CellCorners2);


### Matrix[ActiveCells][8][3] riGetActiveCellCorners([CaseId], [PorosityModel = "Matrix"|"Fracture"])
%!test
%! printf ("===== Testing ====> riGetActiveCellCorners\n");
%! ActiveCellCorners1 =  riGetActiveCellCorners();
%! ActiveCellCorners2 =  riGetActiveCellCorners(1);
%! ActiveCellCorners3 =  riGetActiveCellCorners(1, "Matrix");
%! ActiveCellCorners4 =  riGetActiveCellCorners("Matrix");

%! assert( ndims (ActiveCellCorners1) == 3);
%! [nactive, cidx, idx ] = size(ActiveCellCorners1);
%! assert(idx == 3);
%! assert(cidx == 8);
%! assert(nactive , rows(riGetActiveCellInfo()));

%! assert(ActiveCellCorners1(500,1, 1), 3207.2, 0.1);
%! assert(ActiveCellCorners1(500,1, 2), 9080.7, 0.1);
%! assert(ActiveCellCorners1(500,1, 3), -6076.8, 0.1);
%! assert (ActiveCellCorners1 , ActiveCellCorners2);
%! assert (ActiveCellCorners1 , ActiveCellCorners3);
%! assert (ActiveCellCorners1 , ActiveCellCorners4);

### Vector[WellNames] riGetWellNames([CaseId])
%!xtest
%! printf ("===== Testing ====> riGetWellNames\n");
%! WellNames1 = riGetWellNames();
%! WellNames2 = riGetWellNames(1);
%! assert (rows(WellNames1), 113);
%! assert (rows(WellNames1) == rows(WellNames2));

### Vector[WellCellInfo] riGetWellCells([CaseId], WellName, TimeStep)
%!test
%! printf ("===== Testing ====> riGetWellCells\n");
%! WellNames1 = riGetWellNames();
%! WellCellInfos1 = riGetWellCells(1, WellNames1{1}, 3);
%! WellCellInfos2 = riGetWellCells(WellNames1{1}, 3);


### Vector[WellStatus] riGetWellStatus ([CaseId], WellName, [RequestedTimeSteps])
%!test
%! printf ("===== Testing ====> riGetWellStatus\n");
%! WellNames1 = riGetWellNames();
%! WellStatuses1 = riGetWellStatus(1, WellNames1{1}, [1,3]);
%! disp(WellStatuses1(1));
%! WellStatuses2 = riGetWellStatus( WellNames1{1}, [1,3]);
%! WellStatuses3 = riGetWellStatus(WellNames1{1});

endif