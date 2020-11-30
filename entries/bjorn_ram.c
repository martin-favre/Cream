
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

typedef struct Vec2
{
    int x;
    int y;
} Vec2;

typedef struct Row
{
    int *values;
} Row;

typedef struct Matrix
{
    Row *rows;
    Vec2 size;
} Matrix;

typedef struct Path
{
    int *rows;
    int *scores;
    Vec2 position;
} Path;

int untestedPathSize = 0;
int completePathSize = 0;
// size_t getline(char **lineptr, size_t *n, FILE *stream);

// getline is usually only defined in linux
// if you are in windows and this already does exist for some reason
// and you get some form of duplicate definition error
// due to this function, just remove it. Or rename it.
// Also don't try to understand this function it's literally copy-pasted
#ifdef _WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL)
    {
        return -1;
    }
    if (stream == NULL)
    {
        return -1;
    }
    if (n == NULL)
    {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF)
    {
        return -1;
    }
    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
        {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF)
    {
        if ((p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
            {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n')
        {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif //_WIN32

void getNumbersFromLine(const char *line, const ssize_t lineSize, int outBuff[], int *outBuffSize)
{
    //worst case scenario, whole line is a number. +1 for null termination
    char *currNumberBuff = malloc(sizeof(char) * lineSize + 1);
    int currNumberSize = 0;
    for (int i = 0; i < lineSize; i++)
    {
        const char currChar = line[i];
        if (currChar == ' ' || currChar == '\n')
        {
            if (currNumberSize > 0)
            {
                currNumberBuff[currNumberSize] = 0; // null terminate so atoi works
                outBuff[*outBuffSize] = atoi(currNumberBuff);
                (*outBuffSize)++;
                currNumberSize = 0;
            }
            if (currChar == '\n')
            {
                break;
            }
        }
        else
        {
            currNumberBuff[currNumberSize] = currChar;
            currNumberSize++;
        }
    }
    free(currNumberBuff);
}


Matrix getMatrixFromFile(const char *filename)
{
    Matrix matrix;
    FILE *file = fopen(filename, "r");
    assert(file != NULL);
    {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        read = getline(&line, &len, file);
        int *numbers = malloc(sizeof(int) * 2);
        int numberOfNumbers = 0;
        getNumbersFromLine(line, read, numbers, &numberOfNumbers);
        free(line);
        assert(numberOfNumbers == 2);
        matrix.size.y = numbers[0];
        matrix.size.x = numbers[1];
        free(numbers);
    }
    matrix.rows = malloc(sizeof(Row) * matrix.size.y);
    int currentRow = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] != '\n') // skip random newlines
        {
            matrix.rows[currentRow].values = malloc(sizeof(int) * matrix.size.x);
            int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
            getNumbersFromLine(line, read, matrix.rows[currentRow].values, &unusedBecauseIAmABadBoiWhoDoesNoChecks);
            currentRow++;
        }
    }
    free(line);
    fclose(file);

    return matrix;
}

// Records whatever score is at this paths current position
void recordScore(Path *path, const Matrix *matrix)
{
    path->scores[path->position.x] = matrix->rows[path->position.y].values[path->position.x];
    path->rows[path->position.x] = path->position.y +1; // rows start from 1, not 0
}

void putOnUntestedStack(Path **list, Path path)
{
    (*list)[untestedPathSize] = path;
    untestedPathSize++;
}

Path popUntestedStack(const Path *list)
{
    untestedPathSize--;
    Path out = list[untestedPathSize];
    return out;
}

int isComplete(Path path, int maxX)
{
    return path.position.x >= maxX;
}

// allocate a new path
Path getNewPath(const Matrix *matrix)
{
    Path path = {0};
    path.rows = malloc(sizeof(int) * matrix->size.x);
    path.scores = malloc(sizeof(int) * matrix->size.x);
    return path;
}

// clone currentPath and move the clone it's next step
Path getNextPath(const Path *currentPath, const Matrix *matrix, char dir)
{
    Path nextPath = getNewPath(matrix);
    // Copy over all of the currentPaths knowledge
    nextPath.position.x = currentPath->position.x + 1;
    nextPath.position.y = currentPath->position.y;
    for (int i = 0; i < nextPath.position.x; i++)
    {
        nextPath.rows[i] = currentPath->rows[i];
        nextPath.scores[i] = currentPath->scores[i];
    }
    if (dir == 'u')
    {
        nextPath.position.y -= 1;
        // Flip around if we've reached the top
        if (nextPath.position.y < 0)
        {
            nextPath.position.y = matrix->size.y - 1;
        }
    }
    else if (dir == 'd')
    {
        nextPath.position.y += 1;
        // Flip around if we've reached the bottom
        if (nextPath.position.y >= matrix->size.y)
        {
            nextPath.position.y = 0;
        }
    } else {
        // We're going forward, no need to change y
    }
    return nextPath;
}

// Depth first search
void findPath(const Matrix *matrix, Path **completePaths)
{
    Path *untestedPaths = malloc(sizeof(Path) * 1000000);

    // Go through each column and start a search from each one
    for (int i = 0; i < matrix->size.y; i++)
    {
        Path anotherPath = getNewPath(matrix);
        anotherPath.position.x = 0;
        anotherPath.position.y = i;
        putOnUntestedStack(&untestedPaths, anotherPath);
    }

    while (untestedPathSize > 0)
    {
        Path currentPath = popUntestedStack(untestedPaths);
        // If the path is complete just store it away.
        // We will deal with you later
        if (isComplete(currentPath, matrix->size.x))
        {
            (*completePaths)[completePathSize] = currentPath;
            completePathSize++;
        }
        else
        {
            // make this Path save the current scores and stuff
            recordScore(&currentPath, matrix);
            char directions[] = {'u', 'd', 'f'};
            for (int i = 0; i < 3; i++)
            {
                char dir = directions[i];
                // Spawn a new Path from the current path
                // This path is basically just a clone
                Path nextPath = getNextPath(&currentPath, matrix, dir);
                putOnUntestedStack(&untestedPaths, nextPath);
            }
        }
    }
}

const Path *getLexiSmallestPath(const Path *pathA, const Path *pathB)
{
    for (int i = 0; i < pathA->position.x; i++)
    {
        if (pathA->rows[i] < pathB->rows[i])
        {
            return pathA;
        }
        else if (pathA->rows[i] > pathB->rows[i])
        {
            return pathB;
        }
    }
    return pathA;
}

int getPathScore(const Path *path)
{
    int sum = 0;
    for (int i = 0; i < path->position.x; i++)
    {
        sum += path->scores[i];
    }
    return sum;
}

// Usage: ./a.out path/to/matrix
int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    Matrix matrix = getMatrixFromFile(matrixFilename);

    // This will contain all resulting paths
    Path *completedPaths = malloc(sizeof(Path) * 600000);
    findPath(&matrix, &completedPaths);

    // We have now found all possible paths
    // Figure out which one is the B E S T
    int lowestScore = 9999;
    const Path *bestPath;
    for (int i = 0; i < completePathSize; i++)
    {
        const Path path = completedPaths[i];
        int score = getPathScore(&path);
        if (score < lowestScore)
        {
            lowestScore = score;
            bestPath = &completedPaths[i];
        }
        else if (score == lowestScore)
        {
            bestPath = getLexiSmallestPath(bestPath, &completedPaths[i]);
        }
    }

    for (int x = 0; x < bestPath->position.x; x++)
    {
        printf("%d ", bestPath->rows[x]);
    }
    printf("\n");
    printf("%d\n", getPathScore(bestPath));
    return 0;
}