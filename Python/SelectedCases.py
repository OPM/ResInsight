from ResInsight import ResInsight
import grpc
import sys

def run():
	try:
		port = str(50051)
		if len(sys.argv) > 1:
			port = sys.argv[1]
		resInsight  = ResInsight("localhost:" + port)
		caseInfos = resInsight.ProjectInfo.SelectedCases(ResInsight.Empty())
		
		print ("Got " + str(len(caseInfos.case_info)) + " cases: ")
		for caseInfo in caseInfos.case_info:
			print(caseInfo.name)
		
	except grpc.RpcError as e:
		if e.code() == grpc.StatusCode.NOT_FOUND:
			print("Case id not found")
		else:
			print("Other error")

if __name__ == '__main__':
    run()
