###################################################################################
# This example will connect to ResInsight, retrieve a list of cases and print info
# 
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight  = rips.Instance.find()
if resinsight is not None:
    # Get a list of all cases
    cases = resinsight.project.cases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print("Case name: " + case.name)
        print("Case grid path: " + case.grid_path())
        
        timesteps = case.time_steps()
        for t in timesteps:
            print("Year: " + str(t.year))
            print("Month: " + str(t.month))
        
    

