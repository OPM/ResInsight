# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
print("ResInsight version: " + resinsight.version_string())

project = resinsight.project

summary_cases = project.descendants(rips.SummaryCase)

firstCase = summary_cases[0]

vector_name = "FOPT"
summary_data = firstCase.summary_vector_values(vector_name)

print("Data for summary vector " + vector_name)
print(summary_data.double_values)
