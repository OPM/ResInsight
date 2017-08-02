#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

struct GridLocalCell
{
    int gridIndex;
    int i;
    int j;
    int k;
};

struct Connection
{
    GridLocalCell fromCell;
    GridLocalCell toCell;
};

void getNNCConnections(std::vector<Connection>& connections, const QString& hostName, quint16 port, const qint64& caseId)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    QString command = QString("GetNNCConnections %1").arg(caseId);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    while (socket.bytesAvailable() < (int)sizeof(quint64))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 connectionCount;
    quint64 byteCount;
    quint64 rowByteSize = sizeof(qint32) * 4 * 2;

    socketStream >> connectionCount;

    byteCount = connectionCount * rowByteSize;

    while (socket.bytesAvailable() < (int)byteCount)
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    connections.resize(connectionCount);

    for (size_t i = 0; i < connectionCount; ++i)
    {
        socketStream >> connections[i].fromCell.gridIndex;
        socketStream >> connections[i].fromCell.i >> connections[i].fromCell.j >> connections[i].fromCell.k;
        socketStream >> connections[i].toCell.gridIndex;
        socketStream >> connections[i].toCell.i >> connections[i].toCell.j >> connections[i].toCell.k;
    }

    return;
}

DEFUN_DLD(riGetNNCConnections, args, nargout,
    "Usage:\n"
    "\n"
    "  riGetNNCConnections([CaseId])\n"
    "\n"
    "This function returns a two dimensional matrix containing grid and IJK information for each NNC in the requested case.\n"
    "The columns contain the following information:\n"
    "[GridIdx, I, J, K]:\n"
    "    GridIdx :       The index of the grid the cell resides in. (Main grid has index 0)\n"
    "    I, J, K :       1-based index address of the cell in the grid.\n"
    "\n"
    "If the CaseId is not defined, ResInsight's Current Case is used.\n"
)
{
    int nargin = args.length();
    if (nargin > 1)
    {
        error("riGetNNCConnections: Too many arguments, CaseId are optional input arguments.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetNNCConnections: Missing output argument.\n");
        print_usage();
    }
    else
    {
        std::vector<Connection> connections;
        qint64 caseId = -1;

        if (nargin > 0)
        {
            if (args(0).is_numeric_type())
            {
                unsigned int argCaseId = args(0).uint_value();
                caseId = argCaseId;
            }
        }

        getNNCConnections(connections, "127.0.0.1", 40001, caseId);

        Cell cellValuesGridIndex(connections.size(), 2);
        Cell cellValuesI(connections.size(), 2);
        Cell cellValuesJ(connections.size(), 2);
        Cell cellValuesK(connections.size(), 2);

        for (size_t i = 0; i < connections.size(); ++i)
        {
            cellValuesGridIndex(i, 0) = connections[i].fromCell.gridIndex;
            cellValuesGridIndex(i, 1) = connections[i].toCell.gridIndex;

            cellValuesI(i, 0) = connections[i].fromCell.i;
            cellValuesI(i, 1) = connections[i].toCell.i;

            cellValuesJ(i, 0) = connections[i].fromCell.j;
            cellValuesJ(i, 1) = connections[i].toCell.j;

            cellValuesK(i, 0) = connections[i].fromCell.k;
            cellValuesK(i, 1) = connections[i].toCell.k;
        }

        octave_map m;

        m.assign(riOctavePlugin::cellIndex_gridIndex, cellValuesGridIndex);
        m.assign(riOctavePlugin::cellIndex_I, cellValuesI);
        m.assign(riOctavePlugin::cellIndex_J, cellValuesJ);
        m.assign(riOctavePlugin::cellIndex_K, cellValuesK);

        return octave_value(m);
    }

    return octave_value();
}