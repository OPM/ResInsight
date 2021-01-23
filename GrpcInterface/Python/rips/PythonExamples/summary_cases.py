# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code

# Specific summary case with case_id = 1
summary_case = resinsight.project.summary_case(case_id=1)
summary_case.print_object_info()

# All summary cases
summary_cases = resinsight.project.summary_cases()
for summary_case in summary_cases:
    print("Summary case found: ", summary_case.short_name)
