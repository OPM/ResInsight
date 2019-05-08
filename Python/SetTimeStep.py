from ResInsight import ResInsight
import grpc
import sys

def run():
	try:
		port = str(50051)
		if len(sys.argv) > 1:
			port = sys.argv[1]
		resInsight  = ResInsight("localhost:" + port)
		commands = resInsight.Commands.Execute(ResInsight.SetTimeStep(0, 2))

	except grpc.RpcError as e:
		if e.code() == grpc.StatusCode.NOT_FOUND:
			print("Case id not found")
		else:
			print("Other error")

if __name__ == '__main__':
    run()
