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

Matrix getMatrixFromFile(const char* filename)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE* file = fopen(filename, "r");
    assert(file != NULL);
    read = getline(&line, &len, file);
    Matrix matrix;
    sscanf(&line[0], "%d", &matrix.size.y);
    sscanf(&line[2], "%d", &matrix.size.x);
    matrix.rows = (Row *)malloc(sizeof(Row *) * matrix.size.y);
    int currentRow = 0;
    while ((read = getline(&line, &len, file)) != -1)
    {

        // printf("Retrieved line of length %zu:\n", read);
        // printf("%s", line);
        matrix.rows[currentRow].values = (int *)malloc(sizeof(int *) * matrix.size.x);
        int currentCol = 0;
        for (int i = 0; i < read; i++)
        {
            char currChar = line[i];
            if (currChar == ' ')
                continue;
            if (currChar == '\n')
                break;
            sscanf(&currChar, "%d", &matrix.rows[currentRow].values[currentCol]);
            currentCol++;
        }
        currentRow++;
    }
    fclose(file);
    return matrix;
}

typedef struct Path
{
    int *rows;
    int *scores;
    Vec2 position;
} Path;

int untestedPathSize = 0;
int completePathSize = 0;

void recordScore(Path *path, const Matrix *matrix)
{
    path->scores[path->position.x] = matrix->rows[path->position.y].values[path->position.x];
    path->rows[path->position.x] = path->position.y +1;
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

int isComplete(Path path, int maxX)
{
    return path.position.x >= maxX;
}

Path getNewPath(const Matrix *matrix)
{
    Path path = {0};
    path.rows = malloc(sizeof(int) * matrix->size.x);
    path.scores = malloc(sizeof(int) * matrix->size.x);
    return path;
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

void findPath(const Matrix *matrix, Path **completePaths)
{
    Path *untestedPaths = malloc(sizeof(Path) * 100000);

    for (int i = 0; i < matrix->size.y; i++)
    {
        Path anotherPath = getNewPath(matrix);
        anotherPath.position.x = 0;
        anotherPath.position.y = i;
        putInList(&untestedPaths, anotherPath);
    }

    while (untestedPathSize > 0)
    {
        Path currentPath = getNextFromList(untestedPaths);
        // printf("got a path\n");
        if (isComplete(currentPath, matrix->size.x))
        {
            (*completePaths)[completePathSize] = currentPath;
            completePathSize++;
        }
        else
        {
            recordScore(&currentPath, matrix);
            char directions[] = {'u', 'd', 'f'};
            for (int i = 0; i < 3; i++)
            {
                char dir = directions[i];
                Path nextPath = getNextPath(&currentPath, matrix, dir);
                putInList(&untestedPaths, nextPath);
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

int getScore(const Path *path)
{
    int sum = 0;
    for (int i = 0; i < path->position.x; i++)
    {
        sum += path->scores[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    // printf("%s", matrixFilename);
    Matrix matrix = getMatrixFromFile(matrixFilename);
    ;
    Path *completedPaths = malloc(sizeof(Path) * 100000);
    findPath(&matrix, &completedPaths);

    int lowestScore = 9999;
    const Path *bestPath;
    for (int i = 0; i < completePathSize; i++)
    {
        const Path path = completedPaths[i];
        int score = getScore(&path);
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
    printf("%d\n", getScore(bestPath));
}