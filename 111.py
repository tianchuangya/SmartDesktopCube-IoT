import csv
with open("data/laboratory.csv", "r") as f:
    reader = csv.reader(f)
    data = list(reader)
print(len(data[0]))