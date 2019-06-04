import sys
import os

averages = []
for i in range(0, 10):
	values = []
	
	sum = 0.0
	count = 0
	for j in range(0, 1199516):
		sum += j
		count += 1

	averages.append(sum / count)

print (averages)
