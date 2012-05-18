#include <QtNetwork>
#include <octave/oct.h>


void setEclipseProperty(const Matrix& propertyFrames, const QString &hostName, quint16 port, QString caseName, QString propertyName)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int Timeout = 5 * 1000;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(Timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command("SetProperty ");
    command += caseName + " " + propertyName;
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(QDataStream::Qt_4_0);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Write property data header

    dim_vector mxDims = propertyFrames.dims();

    qint64 cellCount = mxDims.elem(0);
    qint64 timeStepCount = mxDims.elem(1);
    qint64 timeStepByteCount = cellCount * sizeof(double);

    socketStream << (qint64)(timeStepCount);
    socketStream << (qint64)timeStepByteCount;

    const double* internalData = propertyFrames.fortran_vec();
    int dataWritten = socket.write((const char *)internalData, timeStepByteCount*timeStepCount);

    if (dataWritten == timeStepByteCount*timeStepCount)
    {
        QString tmp = QString("riSetActiveCellProperty : Wrote %1").arg(propertyName);

        if (caseName.isEmpty())
        {
            tmp += QString(" to active case.");
        }
        else
        {
            tmp += QString(" to %1.").arg(caseName);
        }
        octave_stdout << tmp.toStdString() << " Active Cells : " << cellCount << " Time steps : " << timeStepCount << std::endl;
    }
    else
    {
        error("Was not able to write the proper amount of data to ResInsight:");
        octave_stdout << " Active Cells : " << cellCount << "Time steps : " << timeStepCount << " Data Written: " << dataWritten << " Should have written: " << timeStepCount * cellCount * sizeof(double) << std::endl;
    }

    while(socket.bytesToWrite())
    {
        // octave_stdout << "Bytes to write: " << socket.bytesToWrite() << std::endl;
        socket.waitForBytesWritten(Timeout);
        OCTAVE_QUIT;
    }

    return;
}



DEFUN_DLD (riSetActiveCellProperty, args, nargout,
           "Usage:\n"
           "\n"
           "\triSetActiveCellProperty( Matrix(nActiveCells, nTimesteps), [CaseName/CaseIndex], PropertyName )\n"
           "\n"
           "Interprets the supplied matrix as an eclipse property set, and puts the data into\n"
           "ResInsight as a \"Generated\" property with the name \"PropertyName\". The property\n"
           "is added to the active case if no case specification is given, or to the Eclipse Case\n"
           "named \"CaseName\" or to the case number \"CaseIndex\". "
           )
{
    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riSetActiveCellProperty: Too few arguments. The data matrix and the name of the property requested is neccesary\n");
        print_usage();
    }
    else
    {
        Matrix propertyFrames = args(0).matrix_value();

        if (error_state)
        {
            error("riSetActiveCellProperty: The supplied first argument is not a valid Matrix");
            return octave_value_list ();
        }

        charMatrix caseName;
        charMatrix propertyName;
         if (nargin > 2)
         {
            caseName = args(1).char_matrix_value();
            propertyName = args(2).char_matrix_value();
         }
         else
         {
             propertyName = args(1).char_matrix_value();
         }

         if (error_state)
         {
             error("setEclipseProperty: The supplied Case / Property names are invalid");
             return octave_value_list ();
         }

        if (nargin > 2)
            setEclipseProperty(propertyFrames, "127.0.0.1", 40001,  caseName.row_as_string(0).c_str(), propertyName.row_as_string(0).c_str());
        else
            setEclipseProperty(propertyFrames, "127.0.0.1", 40001, "", propertyName.row_as_string(0).c_str());
    }

    return octave_value_list ();
}

