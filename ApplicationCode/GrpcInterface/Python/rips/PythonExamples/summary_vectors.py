import rips
import time

resinsight = rips.Instance.find()

project = resinsight.project

# Use the following commented lines to import a file from disk
# filename = "path/to/file/1_R001_REEK-0.SMSPEC"
# summary_case = project.import_summary_case(filename)

# Assumes at least one summery case loaded with case_id 1
summary_case = project.summary_case(1)
if summary_case is None:
    print("No summary case found")
    exit()

vector_name = "FOPT"
summary_data = summary_case.summary_vector_values(vector_name)

print("Data for summary vector " + vector_name)
print(summary_data.values)

time_steps = summary_case.available_time_steps()
print(time_steps.values)

summary_data_sampled = summary_case.resample_values("FOPT", "QUARTER")
print("\nResampled data")

for t, value in zip(summary_data_sampled.time_steps, summary_data_sampled.values):
    print(time.strftime("%a, %d %b %Y ", time.gmtime(t)) + " | " + str(value))
