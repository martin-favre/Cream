import random
import sys
import subprocess
#import bidir


def gen_matrix(index):
    matrixWidth = index
    matrixHeight = index

    outStr = str(matrixHeight) + " " + str(matrixWidth) + '\n'
    for x in range(matrixHeight):
        line = ""
        for y in range(matrixWidth):
            rand = random.randint(1, 9)
            line += str(rand) + " "
        outStr += line[:-1] + '\n'
    return outStr

def getSolutionFromString(string):
    string = string.split('\n')
    string = [x.strip() for x in string]
    string = list(filter(lambda a: a != '', string))
    if len(string) != 2:
        raise Exception("Wrong number of lines in outout")
    output = {}
    output["rows"] = [int(x) for x in string[0].split()]
    output["score"] = int(string[1])
    return output

def getSolution(matrixFilename):
    resultLines = subprocess.check_output(("./a.out", matrixFilename), stderr=subprocess.STDOUT, timeout=100).decode(sys.stdout.encoding)
    return resultLines

for index in range(3, 25):
    matrix = gen_matrix(index)
    matrixFilename = "matrices/" + "matrix_" +  str(index)
    with open(matrixFilename, 'w+') as outfile:
        outfile.write(matrix)
    solution = getSolution(matrixFilename)
    
        # solution = bidir.solveMatrix(matrixFilename)
    solutionFilename = "matrices/" + "solution_" + str(index)
    with open(solutionFilename, 'w') as outfile:
        outfile.write(solution)

