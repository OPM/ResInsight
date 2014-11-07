import socket

class TestClient(object):
    def __init__(self , port , host = "localhost"):
        self.socket = socket.socket( socket.AF_INET , socket.SOCK_STREAM)
        self.port = port
        self.host = host
        self.socket.connect((self.host , self.port))

        
    def send(self , data):
        self.socket.sendall( data )
        
    def recv(self):
        data = self.socket.recv(1024)
        return data
    
    @classmethod
    def sendRecv(cls , port , data):
        client = TestClient(port)
        client.send( data )
        return client.recv()
