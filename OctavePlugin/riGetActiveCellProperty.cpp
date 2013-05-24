#include <QtNetwork>
#include <octave/oct.h>
#include "riSettings.h"

void getActiveCellProperty(Matrix& propertyFrames, const QString &serverName, quint16 serverPort,
                        const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps, QString porosityModel)
{
    const int Timeout = riOctavePlugin::timeOutMilliSecs;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(Timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // Create command as a string with arguments , and send it:

    QString command;
    command += "GetActiveCellProperty " + QString::number(caseId) + " " + propertyName + " " + porosityModel;

    for (int i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(Timeout))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 timestepCount;
    quint64 byteCount;
    size_t  activeCellCount;

    socketStream >> timestepCount;
    socketStream >> byteCount;

    activeCellCount = byteCount / sizeof(double);
    propertyFrames.resize(activeCellCount, timestepCount);

    if (!(byteCount && timestepCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    // Wait for available data for each timestep, then read data for each timestep

    for (size_t tIdx = 0; tIdx < timestepCount; ++tIdx)
    {
        while (socket.bytesAvailable() < (int)byteCount)
        {
            if (!socket.waitForReadyRead(Timeout))
            {
                error((("Waiting for timestep data number: ") + QString::number(tIdx)+  ": " + socket.errorString()).toLatin1().data());
                octave_stdout << "Active cells: " << activeCellCount << ", Timesteps: " << timestepCount << std::endl;
                return ;
            }
           OCTAVE_QUIT;
        }

        qint64 bytesRead = 0;
        double * internalMatrixData = propertyFrames.fortran_vec();

#if 0
        // Raw data transfer. Faster. Not possible when dealing with coarsening
        // bytesRead = socket.read((char*)(internalMatrixData + tIdx * activeCellCount), byteCount);
#else
        // Compatible transfer. Now the only one working
        for (size_t cIdx = 0; cIdx < activeCellCount; ++cIdx)
        {
            socketStream >> internalMatrixData[tIdx * activeCellCount + cIdx];

            if (socketStream.status() == QDataStream::Ok) bytesRead += sizeof(double);
        }
#endif

        if ((int)byteCount != bytesRead)
        {
            error("Could not read binary double data properly from socket");
            octave_stdout << "Active cells: " << activeCellCount << ", Timesteps: " << timestepCount << std::endl;
        }

        OCTAVE_QUIT;
    }

    QString tmp = QString("riGetActiveCellProperty : Read %1").arg(propertyName);

    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from case with Id: %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " Active cells : " << activeCellCount << ", Timesteps : " << timestepCount << std::endl;

    return;
}



DEFUN_DLD (riGetActiveCellProperty, args, nargout,
           "Usage:\n"
           "\n"
           "Matrix[numActiveCells][numTimestepsRequested] riGetActiveCellProperty([CaseId], PropertyName, [RequestedTimeSteps], [PorosityModel = \"Matrix\"|\"Fracture\"] )"
           "\n"
           "This function returns a two dimensional matrix: [ActiveCells][Num TimestepsRequested] containing the requested property data from the case with CaseId."
           "If the case contains coarse-cells, the results are expanded onto the active cells."
           "If the CaseId is not defined, ResInsight’s Current Case is used."
           "The RequestedTimeSteps must contain a list of 1-based indices to the requested timesteps. If not defined, all the timesteps are returned."
           )
{
    if (nargout < 1)
    {
        error("riGetActiveCellProperty: Missing output argument.\n");
        print_usage();
        return octave_value_list ();
    }

    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetActiveCellProperty: Too few arguments. The name of the property requested is neccesary.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 4)
    {
        error("riGetActiveCellProperty: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);
    argIndices.push_back(2);
    argIndices.push_back(3);

    // Check if we have a CaseId:
    if (!args(argIndices[0]).is_numeric_type())
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps

    if (!(nargin > argIndices[2] && args(argIndices[2]).is_matrix_type()))
    {
        argIndices[2] = -1;
        for (size_t aIdx = 3; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a PorosityModel

    int lastArgumentIndex = argIndices[3] ;
    if (!(nargin > argIndices[3] && args(argIndices[3]).is_string()))
    {
        argIndices[3] = -1;
        for (size_t aIdx = 4; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riGetActiveCellProperty: Unexpected argument after the PorosityModel.\n");
        print_usage();
        return octave_value_list ();
    }

    // The actual values are
    Matrix propertyFrames;
    int caseId = -1;
    std::string propertyName = "UNDEFINED";
    int32NDArray requestedTimeSteps;
    std::string porosityModel = "Matrix";

    if (argIndices[0] >= 0) caseId              = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) propertyName        = args(argIndices[1]).char_matrix_value().row_as_string(0);
    if (argIndices[2] >= 0) requestedTimeSteps  = args(argIndices[2]).int32_array_value();
    if (argIndices[3] >= 0) porosityModel       = args(argIndices[3]).string_value();

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riGetActiveCellProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    getActiveCellProperty(propertyFrames, "127.0.0.1", 40001, caseId, propertyName.c_str(), requestedTimeSteps, porosityModel.c_str());

    return octave_value(propertyFrames);
}
