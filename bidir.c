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

const Path *getLexiSmallestPath(const Path *pathA, const Path *pathB);

Vec2 getSizeFromFile(FILE *file)
{
    char *line;
    size_t len;
    ssize_t read = getline(&line, &len, file);
    Vec2 out;
    sscanf(&line[0], "%d", &out.y);
    sscanf(&line[2], "%d", &out.x);
    return out;
}

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
        int *numbers = malloc(sizeof(int) * matrix.size.x);
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

        matrix.rows[currentRow].values = malloc(sizeof(int) * matrix.size.x);
        int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
        getNumbersFromLine(line, read, matrix.rows[currentRow].values, &unusedBecauseIAmABadBoiWhoDoesNoChecks);
        currentRow++;
    }
    free(line);
    fclose(file);

    return matrix;
}

int untestedPathSize = 0;
int completePathSize = 0;

void recordScore(Path *path, const Matrix *matrix)
{
    path->scores[path->position.x] = matrix->rows[path->position.y].values[path->position.x];
    path->rows[path->position.x] = path->position.y + 1;
}

void putInList(Path **list, Path path)
{
    (*list)[untestedPathSize] = path;
    untestedPathSize++;
}

Path getNextFromList(const Path *list)
{
    untestedPathSize--;
    Path out = list[untestedPathSize];
    return out;
}

int isComplete(const Path *path, int maxX)
{
    return path->position.x >= maxX;
}

Path getNewPath(const Matrix *matrix)
{
    Path path = {0};
    path.rows = malloc(sizeof(int) * matrix->size.x);
    path.scores = malloc(sizeof(int) * matrix->size.x);
    return path;
}

void freePath(Path *path)
{
    free(path->rows);
    free(path->scores);
}

Path getNextPath(const Path *currentPath, const Matrix *matrix, char dir)
{
    Path nextPath = getNewPath(matrix);
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
        if (nextPath.position.y < 0)
        {
            nextPath.position.y = matrix->size.y - 1;
        }
    }
    else if (dir == 'd')
    {
        nextPath.position.y += 1;
        if (nextPath.position.y >= matrix->size.y)
        {
            nextPath.position.y = 0;
        }
    }
    return nextPath;
}

int getScore(const Path *path)
{
    int sum = 0;
    for (int i = 0; i < path->position.x; i++)
    {
        sum += path->scores[i];
    }
    return sum;
}

const Path *getBestPath(const Path *pathA, const Path *pathB)
{
    const int scoreA = getScore(pathA);
    const int scoreB = getScore(pathB);
    if (scoreA < scoreB)
    {
        return pathA;
    }
    else if (scoreA == scoreB)
    {
        return getLexiSmallestPath(pathA, pathB);
    }
    return pathB;
}
void findPath(const Matrix *matrix, Path *bestPath)
{
    Path *untestedPaths = malloc(sizeof(Path) * 1000000);

    for (int i = 0; i < matrix->size.y; i++)
    {
        Path anotherPath = getNewPath(matrix);
        anotherPath.position.x = 0;
        anotherPath.position.y = i;
        putInList(&untestedPaths, anotherPath);
    }
    int bestPathSet = 0;
    while (untestedPathSize > 0)
    {
        Path currentPath = getNextFromList(untestedPaths);

        if (isComplete(&currentPath, matrix->size.x))
        {
            if (bestPathSet)
            {
                const Path *betterPath = getBestPath(bestPath, &currentPath);
                if (betterPath == &currentPath)
                {
                    freePath(bestPath);
                    *bestPath = currentPath;
                }
                else
                {
                    freePath(&currentPath);
                }
            }
            else
            {
                freePath(bestPath);
                *bestPath = currentPath;
                bestPathSet = 1;
            }
        }
        else
        {
            recordScore(&currentPath, matrix);
            char directions[] = {'u', 'd', 'f'};
            for (int i = 0; i < 3; i++)
            {
                char dir = directions[i];
                Path nextPath = getNextPath(&currentPath, matrix, dir);
                if (bestPathSet)
                {
                    const Path *betterPath = getBestPath(bestPath, &nextPath);
                    if (betterPath == &nextPath)
                    {
                        putInList(&untestedPaths, nextPath);
                    }
                    else
                    {
                        freePath(&nextPath);
                    }
                }
                else
                {
                    putInList(&untestedPaths, nextPath);
                }
            }
            freePath(&currentPath);
        }
    }
    free(untestedPaths);
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

void printMatrix(Matrix matrix)
{
    int i, j = 0;

    for (i = 0; i < matrix.size.y; i++)
    {
        for (j = 0; j < matrix.size.x; j++)
        {
            printf("%d ", matrix.rows[i].values[j]);
        }
        printf("\n");
    }
}

void freeMatrix(Matrix *matrix)
{
    for (int i = 0; i < matrix->size.y; i++)
    {
        free(matrix->rows[i].values);
    }
    free(matrix->rows);
}

int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    // matrixFilename = "matrices/bigmatrix2";
    // matrixFilename = "matrices/matrix_1";
    Matrix matrix = getMatrixFromFile(matrixFilename);
    // printMatrix(matrix);
    Path bestPath = getNewPath(&matrix);
    findPath(&matrix, &bestPath);

    for (int x = 0; x < bestPath.position.x; x++)
    {
        printf("%d ", bestPath.rows[x]);
    }
    printf("\n");
    printf("%d\n", getScore(&bestPath));
    freePath(&bestPath);
    freeMatrix(&matrix);
}