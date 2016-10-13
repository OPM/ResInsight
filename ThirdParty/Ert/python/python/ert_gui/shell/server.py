import socket
from threading import Thread

from ert.server import ErtRPCServer
from ert_gui.shell import assertConfigLoaded, ErtShellCollection


class Server(ErtShellCollection):
    def __init__(self, parent):
        super(Server, self).__init__("server", parent)
        self.shellContext()["server_settings"] = self

        self.addShellFunction(name="start",
                              function=Server.startServer,
                              help_message="Start the ERT RPC Server")

        self.addShellFunction(name="stop",
                              function=Server.stopServer,
                              help_message="Stop the ERT RPC Server")

        self.addShellFunction(name="inspect",
                              function=Server.inspect,
                              help_message="Shows information about the current job queue")

        self.addShellProperty(name="hostname",
                              getter=Server.getHost,
                              setter=Server.setHost,
                              help_arguments="[hostname]",
                              help_message="Show or set the server hostname",
                              pretty_attribute="Hostname")

        self.addShellProperty(name="port",
                              getter=Server.getPort,
                              setter=Server.setPort,
                              help_arguments="[port]",
                              help_message="Show or set the server port number (0 = automatic)",
                              pretty_attribute="Port")

        self._server = None
        """ :type: ErtRPCServer """

        self._hostname = "localhost"
        self._port = 0

    def getHost(self):
        return self._hostname

    def setHost(self, hostname):
        self._hostname = hostname

    def getPort(self):
        return self._port

    def setPort(self, port):
        self._port = int(port)

    @assertConfigLoaded
    def startServer(self, line):
        port = self._port
        host = self._hostname

        if self._server is None:
            try:
                self._server = ErtRPCServer(self.ert(), host=host, port=port)
            except socket.error as e:
                print("Unable to start the server on port: %d" % port)
            else:
                thread = Thread(name="Shell Server Thread")
                thread.daemon = True
                thread.run = self._server.start
                thread.start()
                print("Server running on host: '%s' and port: %d" % (self._server.host, self._server.port))
        else:
            print("A server is already running at host: '%s' and port: %d" % (self._server.host, self._server.port))

    def _stopServer(self):
        if self._server is not None:
            self._server.stop()
            self._server = None
            print("Server stopped")

    def stopServer(self, line):
        if self._server is not None:
            self._stopServer()
        else:
            print("No server to stop")

    def cleanup(self):
        self._stopServer()
        ErtShellCollection.cleanup(self)

    def inspect(self, line):
        if self._server is not None:
            if self._server.isRunning():
                print("Waiting..: %d" % self._server.getWaitingCount())
                print("Running..: %d" % self._server.getRunningCount())
                print("Failed...: %d" % self._server.getFailedCount())
                print("Succeeded: %d" % self._server.getSuccessCount())
                print("Batch#...: %d" % self._server.getBatchNumber())
            else:
                print("Server is not running any simulations")
        else:
            print("No server is not available")


