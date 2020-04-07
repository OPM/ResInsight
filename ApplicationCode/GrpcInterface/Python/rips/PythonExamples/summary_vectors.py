import rips

resinsight = rips.Instance.find()

project = resinsight.project
summary_cases = project.descendants(rips.SummaryCase)

# Assumes at least one summery case loaded
firstCase = summary_cases[0]

vector_name = "FOPT"
summary_data = firstCase.summary_vector_values(vector_name)

print("Data for summary vector " + vector_name)
print(summary_data.values)

time_steps = firstCase.available_time_steps()
print(time_steps.values)
