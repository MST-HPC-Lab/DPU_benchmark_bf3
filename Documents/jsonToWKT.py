# This script is used to convert JSON format data downloaded from UCR-Star dataset to WKT format.
# 
# USAGE
# -----
# python3 jsonToWKT.py <input_file_path> <output_file_path>

import sys
import json

inputFile = open(sys.argv[1], 'r')
outputFile = open(sys.argv[2], 'w')

while True:
    line = inputFile.readline()
    if not line or len(line) < 5:
        break
    data = json.loads(line)
    outputFile.write(data["g"] + "\n")
  

  
inputFile.close()
outputFile.close()