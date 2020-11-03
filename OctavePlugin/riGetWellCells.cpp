#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getWellCells(  std::vector<int>& cellIs, 
    std::vector<int>& cellJs, 
    std::vector<int>& cellKs,
    std::vector<int>& gridIndices,
    std::vector<int>& cellStatuses,
    std::vector<int>& branchIds,
    std::vector<int>& segmentIds,
    const QString &hostName, quint16 port, 
    const qint64& caseId, const QString& wellName, int requestedTimeStep)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error("Connection: %s",socket.errorString().toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command;
    command += QString("GetWellCells") + " " + QString::number(caseId) + " " + wellName + " " +  QString::number(requestedTimeStep) ;

    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;

    if (byteCount == 0)
    {
        return;
    }

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 cellCount;
    socketStream >> cellCount;

    octave_stdout << "riGetWellCells: Number of cells in well " << wellName.toLatin1().data() << " : " <<  cellCount << std::endl;

    cellIs        .reserve(cellCount);      
    cellJs        .reserve(cellCount);
    cellKs        .reserve(cellCount);
    gridIndices   .reserve(cellCount);
    cellStatuses  .reserve(cellCount);
    branchIds     .reserve(cellCount);
    segmentIds    .reserve(cellCount);

    qint32 i, j, k, gIdx, cStat, bId, sId;

    for (size_t cIdx = 0; cIdx < cellCount; cIdx++)
    {
        socketStream >> i;
        socketStream >> j;
        socketStream >> k;
        socketStream >> gIdx;
        socketStream >> cStat;
        socketStream >> bId;
        socketStream >> sId;

        cellIs.push_back     (i); 
        cellJs.push_back     (j); 
        cellKs.push_back     (k);
        gridIndices.push_back (gIdx);
        cellStatuses.push_back(cStat);
        branchIds.push_back  (bId);
        segmentIds.push_back (sId);
    }

    return;
}



DEFUN_DLD (riGetWellCells, args, nargout,
    "Usage:\n"
    "\n"
    "   riGetWellCells ([CaseId], WellName, TimeStep) \n"
    "\n"
    "This function returns the cells defined in the specified well for the time step requested \n"
    "as a vector of Structures. \n"
    "The Structure is defined as:\n"
    "WellCellInfo  { \n"
    "    I, J, K 		= int	 # Index to the cell in the grid\n"
    "    GridIndex	    = int	 # the index of the grid. Main grid has index 0.\n"
    "    CellStatus	    = int	 # is either 0 or 1, meaning the cell is closed or open respectively\n"
    "    BranchId	    = int	 # Branch id of the branch intersecting the cell\n"
    "    SegmentId	    = int	 # Branch segment id of the branch intersecting the cell\n"
    "}\n"
    "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
    )
{
    if (nargout != 1)
    {
        error("riGetWellCells: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
        return octave_value();
    }

    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riGetWellCells: Too few arguments, this function needs at least the well name and a timestep as input.\n");
        print_usage();
        return octave_value();
    }

    if (nargin > 3)
    {
        error("riGetWellCells: Too many arguments, this function takes at most three arguments.\n");
        print_usage();
        return octave_value();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0); // caseId
    argIndices.push_back(1); // WellName
    argIndices.push_back(2); // TimeStep

    // Check if we do not have a CaseId:
    if (args(argIndices[0]).is_string()) // Check if first argument is a text. If it is, the caseId is missing
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    if (!args(argIndices[1]).is_string()) // Check if the WellName argument is actually a string
    {
        error("riGetWellCells: Missing Well Name. this function needs at least the well name and a timestep as input.\n");
        print_usage();
        return octave_value();
    }

    if (!riOctavePlugin::isOctaveValueNumeric(args(argIndices[2]))) // Check if the TimeStep argument is actually a number
    {
        error("riGetWellCells: The last argument must be a timestep index.\n");
        print_usage();
        return octave_value();
    }

    // Setup the argument list

    int caseId = -1;
    std::string wellName = "UNDEFINED";
    int requestedTimeStep = -1;

    if (argIndices[0] >= 0) caseId              = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) wellName            = args(argIndices[1]).char_matrix_value().row_as_string(0);
    if (argIndices[2] >= 0) requestedTimeStep   = args(argIndices[2]).int_value();

    if (wellName == "UNDEFINED")
    {
        error("riGetWellCells: The argument must be a text containing the well name.\n");
        print_usage();
        return octave_value();
    }

    if (requestedTimeStep == -1)
    {
        error("riGetWellCells: The last argument must be a timestep index (1 - timestepCount).\n");
        print_usage();
        return octave_value();
    }

    std::vector<int> cellIs, cellJs, cellKs;
    std::vector<int> gridIndices;
    std::vector<int> cellStatuses;
    std::vector<int> branchIds;
    std::vector<int> segmentIds;

    getWellCells( cellIs, cellJs, cellKs,
        gridIndices,
        cellStatuses,
        branchIds,
        segmentIds,
        "127.0.0.1", 40001, 
        caseId, QString::fromStdString(wellName), requestedTimeStep);

    size_t cellCount = cellIs.size();

    if (cellJs.size() != cellCount 
        || cellKs.size() != cellCount
        || gridIndices.size() != cellCount
        || cellStatuses.size() != cellCount
        || branchIds.size() != cellCount
        || segmentIds.size() != cellCount )
    {
        error("riGetWellCells: Inconsistent data received from ResInsight.\n");
        return octave_value();
    }


    // Create cells with N items for each field in the data structure

    Cell cellIscv      (cellCount, 1); 
    Cell cellJscv      (cellCount, 1); 
    Cell cellKscv      (cellCount, 1);
    Cell gridIndicescv  (cellCount, 1);
    Cell cellStatusescv (cellCount, 1);
    Cell branchIdscv   (cellCount, 1);
    Cell segmentIdscv  (cellCount, 1);

    for (size_t i = 0; i < cellCount; i++)
    {
        cellIscv       (i) = cellIs     [i];
        cellJscv       (i) = cellJs     [i];
        cellKscv       (i) = cellKs     [i];
        gridIndicescv  (i) = gridIndices [i];
        cellStatusescv (i) = cellStatuses[i];
        branchIdscv    (i) = branchIds  [i];
        segmentIdscv   (i) = segmentIds [i];
    }

    // Build a map between the field name and field cell values

    octave_map m;

    m.assign(riOctavePlugin::wellCellInfo_I,            cellIscv      );
    m.assign(riOctavePlugin::wellCellInfo_J,            cellJscv      );
    m.assign(riOctavePlugin::wellCellInfo_K,            cellKscv      );
    m.assign(riOctavePlugin::wellCellInfo_GridIndex	,   gridIndicescv );
    m.assign(riOctavePlugin::wellCellInfo_CellStatus,   cellStatusescv);
    m.assign(riOctavePlugin::wellCellInfo_BranchId,     branchIdscv   );
    m.assign(riOctavePlugin::wellCellInfo_SegmentId,    segmentIdscv  );

    return octave_value(m);

}

