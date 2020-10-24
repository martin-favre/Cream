import random
import sys
import bidir


def gen_matrix():
    matrixWidth = random.randint(2, 8)
    matrixHeight = random.randint(2, 8)

    outStr = str(matrixHeight) + " " + str(matrixWidth) + '\n'
    for x in range(matrixHeight):
        line = ""
        for y in range(matrixWidth):
            rand = random.randint(1, 9)
            line += str(rand) + " "
        outStr += line[:-1] + '\n'
    return outStr

for index in range(20):
    matrix = gen_matrix()
    matrixFilename = "matrices/matrix_" + str(index) 
    with open(matrixFilename, 'w') as outfile:
        outfile.write(matrix)
    solution = bidir.solveMatrix(matrixFilename)
    solutionFilename = "matrices/solution_" + str(index)
    with open(solutionFilename, 'w') as outfile:
        outfile.write(solution)

