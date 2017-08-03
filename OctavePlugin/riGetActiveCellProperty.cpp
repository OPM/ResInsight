#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"

#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getActiveCellProperty(Matrix& propertyFrames, const QString &serverName, quint16 serverPort,
                        const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps, QString porosityModel)
{
    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
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
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
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

    quint64 totalByteCount = byteCount * timestepCount;

    double* internalMatrixData = propertyFrames.fortran_vec();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), totalByteCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error(errorMessages[i].toLatin1().data());
        }

        return;
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
    
    if (!(nargin > argIndices[2] && (args(argIndices[2]).is_matrix_type() || args(argIndices[2]).is_numeric_type()) && !args(argIndices[2]).is_string()))
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

    // Setup the argument list

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
