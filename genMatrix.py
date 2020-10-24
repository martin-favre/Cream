import random
import sys

outFilename = sys.argv[1]
matrixWidth = random.randint(2, 10)
matrixHeight = random.randint(2, 10)

outStr = str(matrixHeight) + " " + str(matrixWidth) + '\n'
for x in range(matrixHeight):
    line = ""
    for y in range(matrixWidth):
        rand = random.randint(1, 9)
        line += str(rand) + " "
    outStr += line[:-1] + '\n'

with open(outFilename, 'w') as outfile:
    outfile.write(outStr)
