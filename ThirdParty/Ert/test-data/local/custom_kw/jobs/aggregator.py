#! /usr/bin/env python

if __name__ == "__main__":
    files = {"PERLIN_1": "perlin_1.txt",
             "PERLIN_2": "perlin_2.txt",
             "PERLIN_3": "perlin_3.txt"}

    with open("aggregated.txt", "w") as output_file:
        sum_of_sum = 0.0
        for key in files:
            sum = 0.0
            with open(files[key], "r") as input_file:
                sum += float(input_file.readline())
            sum_of_sum += sum
            output_file.write("%s %f\n" % (key, sum))

        if sum_of_sum < 0:
            state = "Negative"
        elif abs(sum_of_sum) < 0.00000001:
            state = "Zero"
        else:
            state = "Positive"

        output_file.write("STATE %s" % state)


