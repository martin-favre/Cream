import os
import sys
import subprocess
from subprocess import CalledProcessError
from subprocess import TimeoutExpired
import getMemUsage
import time
import re

def getFileSize(filename):
    return os.path.getsize(filename)


def purge(dir):
    for f in os.listdir(dir):
        if "massif.out." in f:
            os.remove(os.path.join(dir, f))


def getRam(matrix, filename):
    purge(".")
    subprocess.run(["valgrind", "--tool=massif", "--stacks=yes",
                    "--pages-as-heap=no", filename, matrix])
    ps = subprocess.Popen(('ls'), stdout=subprocess.PIPE)
    resultFilename = subprocess.check_output(
        ('grep', 'massif.out'), stdin=ps.stdout).decode(sys.stdout.encoding).strip()
    ps.wait()
    maxUsage = getMemUsage.get_mem_usage(resultFilename)
    return maxUsage

def getSpeed(matrix, filename):
    start = time.time()
    resultLines = subprocess.check_output((filename, matrix), stderr=subprocess.STDOUT, timeout=5).decode(sys.stdout.encoding)
    end = time.time() - start
    return end

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

def isValid(matrix, filename, solutionFilename):
    try:
        resultLines = subprocess.check_output((filename, matrix), stderr=subprocess.STDOUT, timeout=5).decode(sys.stdout.encoding)
    except CalledProcessError as e:
        print("ProcessError ", str(e))
        return False
    except TimeoutExpired as e:
        print("TimeoutExpired ", str(e))
        return False
    result = getSolutionFromString(resultLines)
    with open(solutionFilename) as solutionFile:
        solutionLines = solutionFile.read()
    solution = getSolutionFromString(solutionLines)
    return solution == result
    for i in range(len(resultLines)):
        if resultLines[i] != solutionLines[i]:
            return False
    return True


def tryint(s):
    try:
        return int(s)
    except:
        return s

def alphanum_key(s):
    """ Turn a string into a list of string and number chunks.
        "z23a" -> ["z", 23, "a"]
    """
    return [ tryint(c) for c in re.split('([0-9]+)', s) ]

def sort_nicely(l):
    """ Sort the given list in the way that humans expect.
    """
    l.sort(key=alphanum_key)

filename = sys.argv[1]
directory = sys.argv[2]
# filename = "../a.out"
fileSize = getFileSize(filename)
# directory = "../matrices"
matrices = [os.path.join(directory, f) for f in os.listdir(directory) if "matrix" in f]
matrices.sort(key=alphanum_key)
solutions = [os.path.join(directory, f)  for f in os.listdir(directory) if "solution" in f]
solutions.sort(key=alphanum_key)
timedout = []
speeds = []
rams = []

for index in range(len(matrices)):
    matrix = matrices[index]
    solution = solutions[index]
    valid = isValid(matrix, filename, solution)
    if not valid:
        break
    speed = getSpeed(matrix, filename)
    ram = getRam(matrix, filename)
    speeds.append(speed)
    rams.append(ram)

print("Filesize is " + str(fileSize))
print("Speeds are " + str(speeds))
print("Rams are " + str(rams))
