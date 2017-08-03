#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>
#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getSelectedCells(int32NDArray& selectedCellInfo, const QString &hostName, quint16 port, const qint64& caseId)
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

    // Create command and send it:

    QString command = QString("GetSelectedCells %1").arg(caseId);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 columnCount;
    quint64 byteCountForOneTimestep;
    size_t  selectedCellCount;

    socketStream >> columnCount;
    socketStream >> byteCountForOneTimestep;

    selectedCellCount = byteCountForOneTimestep / sizeof(qint32);

    dim_vector dv (2, 1);
    dv(0) = selectedCellCount;
    dv(1) = columnCount;
    selectedCellInfo.resize(dv);

    if (!(byteCountForOneTimestep && columnCount))
    {
        error ("No selected cells found in ResInsight");
        return;
    }

    qint32* internalMatrixData = (qint32*)selectedCellInfo.fortran_vec()->mex_get_data();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), columnCount * byteCountForOneTimestep, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error(errorMessages[i].toLatin1().data());
        }

        OCTAVE_QUIT;
    }

    QString tmp = QString("riGetSelectedCells : Read selected cell info");

    octave_stdout << tmp.toStdString() << " Selected cells: " << selectedCellCount << ", Columns: " << columnCount << std::endl;

    return;
}



DEFUN_DLD (riGetSelectedCells, args, nargout,
           "Usage:\n"
           "\n"
           "  Matrix[numSelectedCells][5] riGetSelectedCells()\n"
           "\n"
           "This function returns a two dimensional matrix containing cell info for each selected cell.\n"
           "The columns contain the following information:\n"
           "[CaseId, GridIdx, I, J, K]\n"
           "    CaseId  :       The ID of the case the cell resides in.\n"
           "    GridIdx :       The index of the grid the cell resides in. (Main grid has index 0)\n"
           "    I, J, K :       1-based index address of the cell in the grid.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetSelectedCells: Too many arguments.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetSelectedCells: Missing output argument.\n");
        print_usage();
    }
    else
    {
        qint64 caseId = -1;

        if (nargin > 0)
        {
            unsigned int argCaseId = args(0).uint_value();
            caseId = argCaseId;
        }

        int32NDArray propertyFrames;

        getSelectedCells(propertyFrames, "127.0.0.1", 40001, caseId);

        return octave_value(propertyFrames);
    }

    return octave_value();
}

