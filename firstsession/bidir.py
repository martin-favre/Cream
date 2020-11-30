from queue import Queue
import copy
import sys
class Pos:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
    def move(self, direction, maxY):
        newPos = Pos(self.x +1, self.y)
        if(direction == 'u'):
            newPos.y = self.y - 1
            if(newPos.y < 0):
                newPos.y = maxY-1
        elif(direction == 'd'):
            newPos.y = self.y + 1
            if(newPos.y >= maxY):
                newPos.y = 0
        return newPos

class Path:
    maxY = 0
    maxX = 0
    matrix = None
    def __init__(self, initialPos):
        self.currentPos = initialPos
        self.path = []
        self.scores = []
        self.rows = []
    
    def recordScore(self):
        self.scores.append(Path.matrix[self.currentPos.y][self.currentPos.x])
        self.rows.append(self.currentPos.y+1)

    def __eq__(self, other):
        return self.currentPos == other.currentPos and self.path == other.path and self.scores == other.scores and self.rows == other.rows

    def spawnNextPath(self, direction):
        nextPath = Path(self.currentPos.move(direction, Path.maxY))
        nextPath.path = self.path + [direction]
        nextPath.scores = copy.deepcopy(self.scores)
        nextPath.rows = copy.deepcopy(self.rows)
        return nextPath
        
    
    def isComplete(self):
        return self.currentPos.x == Path.maxX

def getLexiSmallestPath(pathA, pathB):
    for index, step in enumerate(pathA.rows):
        if step < pathB.rows[index]:
            return pathA
        elif step > pathB.rows[index]:
            return pathB
    return pathA

# matrix = [
#   [3,4,1,2,8,6],
#   [6,1,8,2,7,4],
#   [5,9,3,9,9,5],
#   [8,4,1,3,2,6],
#   [3,7,2,8,6,4]
# ]
def solveMatrix(path):
    infile = path
    with open(infile) as file:
        firstLine = file.readline()
        Path.maxY, Path.maxX = [int(x) for x in firstLine.split(' ')]
        matrix = []
        for line in file.readlines():
            matrix.append([int(x) for x in line.split(' ')])

    Path.matrix = matrix

    unTestedPaths = Queue()
    for row in range(len(matrix)):
        aPath = Path(Pos(0, row))
        unTestedPaths.put(aPath)

    directions = ['u', 'd', 'f']
    completePaths = []
    while not unTestedPaths.empty():
        currentPath = unTestedPaths.get_nowait()
        if(currentPath.isComplete()):
            completePaths.append(currentPath)
        else:
            currentPath.recordScore()
            for dire in directions:
                nextPath = currentPath.spawnNextPath(dire)
                unTestedPaths.put(nextPath)

    minScore = 99999
    bestPath = None
    for path in completePaths:
        score = sum(path.scores)
        if score < minScore:
            minScore = score
            bestPath = path
        elif score == minScore:
            bestPath = getLexiSmallestPath(bestPath, path)
    outstr = str(bestPath.rows).replace('[', '').replace(']', '').replace(',', ' ') + "\n"
    outstr += str(sum(bestPath.scores))
    return outstr
    # print()
    # print(sum(bestPath.scores))
# print("A best path:")
# print("Sum: " + str(sum(bestPath.scores)))
# print("scores " + str(bestPath.scores))
# print("rows " + str(bestPath.rows))
# print("path " + str(bestPath.path))
# print("")
