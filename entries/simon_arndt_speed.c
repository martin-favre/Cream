/********************************************************************
To the matrix, we have added an array 'sums' which is used for storing
the weight of the best partial path from the edge to each element.
These sums are initialized to the same value as 'values'.

The path finding has two phases, performed by the functions
computeSumsToMatrix and findShortestPath.

In the first phase, one column at a time is inspected, starting at the next to
last one, and going to the left.
For each element in a column, we look at the three neighbors to the right which
we can go to, pick the one with the smallest sum and compute the weight for
the element as the sum of the number on the element and the smallest neighbor sum.

In the second phase, we look at the first column and find the element
with the smallest sum.
That is the final answer for the sum. We print out the row number of the
element.
Next, we find the neighbor to the right which yielded that sum. It is
the one which itself has the lowest sum among the three neighbors. We print
out its row number, and continues to the right in this way to the last column.
If several elements have the same sum, we will pick the one we find first,
thereby satisfying the rule for which path to choose among several equally good
ones.
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define NUMTYPE int
#define findShortestPath f1
#define computeSumsToMatrix f2

typedef struct vec2
{
    NUMTYPE x;
    NUMTYPE y;
} vec2;

typedef struct Row
{
    NUMTYPE *values;
    NUMTYPE *sums;
} Row;

typedef struct Matrix
{
    Row *rows;
    vec2 size;
} Matrix;

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
#endif
static void getNumbersFromLine(const char *line, const ssize_t lineSize, NUMTYPE outBuff[], int *outBuffSize, NUMTYPE sumBuff[])
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
                int conversion = atoi(currNumberBuff);
                outBuff[*outBuffSize] = conversion;
                sumBuff[*outBuffSize] = conversion;
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

static Matrix getMatrixFromFile(const char *filename)
{
    Matrix matrix;
    FILE *file = fopen(filename, "r");
    //assert(file != NULL);
    {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        read = getline(&line, &len, file);
        NUMTYPE *numbers = malloc(sizeof(NUMTYPE) * matrix.size.x);
        int numberOfNumbers = 0;
        getNumbersFromLine(line, read, numbers, &numberOfNumbers, numbers);
        free(line);
       // assert(numberOfNumbers == 2);
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

        matrix.rows[currentRow].values = malloc(sizeof(NUMTYPE) * matrix.size.x);
        matrix.rows[currentRow].sums = malloc(sizeof(NUMTYPE) * matrix.size.x);
        int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
        getNumbersFromLine(line, read, matrix.rows[currentRow].values, &unusedBecauseIAmABadBoiWhoDoesNoChecks, matrix.rows[currentRow].sums);
        currentRow++;
    }
    free(line);
    fclose(file);

    return matrix;
}

/*
Find the neighbor with the lowest value.
*/
static void findSumNeighbor(const Matrix *matrix, int *bestSum_p,
                      int *row_p, int startRow, int column)
{
    int bestSum = 0;
    int bestI = 0;
    for (int posNeighbor = -1; posNeighbor < 2; posNeighbor++)
    { //posNeightbor -1 0 1
        int rowPos = startRow + posNeighbor; // Row number for neighbour
        if (rowPos < 0)
        {
            rowPos += matrix->size.y;
        }
        if (rowPos > matrix->size.y - 1)
        {
            rowPos = 0;
        }
        int sum = matrix->rows[rowPos].sums[column];
        if (sum < bestSum || bestSum == 0 || (sum == bestSum && rowPos < bestI))
        {
            bestSum = sum;
            bestI = rowPos;
        }
    }
    *bestSum_p = bestSum;
    *row_p = bestI;
}


/*
* Create a new matrix containing the sums from all
* columns from right to left.
*/
static void computeSumsToMatrix(const Matrix *matrix)
{
    for (int j = matrix->size.x-2; j >= 0 ; j--)
    {
        for (int i = 0; i < matrix->size.y; i++)
        {
            int v = matrix->rows[i].values[j];
            int bestSum;
            int bestRow; // Dummy
            findSumNeighbor(matrix, &bestSum, &bestRow, i, j+1);
            matrix->rows[i].sums[j] = v + bestSum;
        }
    }
}


//Find the shortest lexically path
static void findShortestPath(const Matrix *matrix)
{
    int startI = 0;
    int bestSum = 0;
    int bestSum0 = 0;
    // Check smallest first column
    for (int j = 0; j < 1; j++)
    {
        int bestI = 0;
        for (int i = 0; i < matrix->size.y; i++)
        {
            int v = matrix->rows[i].sums[j];
            if (v < bestSum || bestSum == 0)
            {
                bestSum = v;
                bestI = i;
            }

        }
        startI = bestI;
    }
    //Save sum used for sum print
    bestSum0 = bestSum;
    printf("%d", startI + 1);
    //Rest of columns
    for (int j = 1; j < matrix->size.x; j++)
    {
        int bestI = 0;
        int bestSum = 0;
        findSumNeighbor(matrix, &bestSum, &bestI, startI, j);
        startI = bestI;
        printf(" %d", bestI+1);
    }
    printf("\n%d", bestSum0);
    printf("\n");
}

// Usage:
// ./a.out path/to/mymatrixfile
int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    Matrix matrix = getMatrixFromFile(matrixFilename);

    computeSumsToMatrix(&matrix);
    findShortestPath(&matrix);
    return 0;
}