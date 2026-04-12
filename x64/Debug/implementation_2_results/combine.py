"""
This script combines multiple CSV result files into a single text file.s.

Each CSV file corresponds to a different problem instance 
and is named in the format: <instance>-result.csv (e.g., N-be75eec_150-result.csv).

The requirements specifcally ask that all data to be in a single dataset.
Therefore, this script automatically merges all CSV files into one file. 

The script:
- Reads all CSV files from a directory
- Extracts the instance name from each filename
- Adds a new column 'instance' as the first column
- Keeps the header only once
- Writes all data into a single file (combined-results.txt)
"""

import glob
import os

files = sorted(glob.glob("./*.csv"))

with open("combined-results.txt", "w") as outfile:
    first = True

    for file in files:
        filename = os.path.basename(file)
        instance = filename.replace("-result.csv", "")

        with open(file, "r") as infile:
            lines = infile.readlines()

            if first:
                # Put 'instance' in front of existing header
                header = "instance," + lines[0]
                outfile.write(header)
                first = False

            # Add instance at the beginning of each row
            for line in lines[1:]:
                line = f"{instance}," + line
                outfile.write(line)

print("Combined results have been written to combined-results.txt")
