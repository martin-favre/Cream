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
    char *currNumberBuff = (char *)malloc(sizeof(char) * lineSize + 1);
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
        int *numbers = (int *)malloc(sizeof(int) * 2);
        int numberOfNumbers = 0;
        getNumbersFromLine(line, read, numbers, &numberOfNumbers);
        free(line);
        assert(numberOfNumbers == 2);
        matrix.size.y = numbers[0];
        matrix.size.x = numbers[1];
        free(numbers);
    }
    matrix.rows = (Row*)malloc(sizeof(Row) * matrix.size.y);
    int currentRow = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] != '\n') // skip random newlines
        {
            matrix.rows[currentRow].values = (int *)malloc(sizeof(int) * matrix.size.x);
            int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
            getNumbersFromLine(line, read, matrix.rows[currentRow].values, &unusedBecauseIAmABadBoiWhoDoesNoChecks);
            currentRow++;
        }
    }
    free(line);
    fclose(file);

    return matrix;
}


// copyMatrix
Matrix copyMatrix(const Matrix *matrix)
{
    Matrix new_matrix;

    new_matrix.size.y = matrix->size.y;
    new_matrix.size.x = matrix->size.x;

    new_matrix.rows = (Row*)malloc(sizeof(Row) * matrix->size.y);
    int currentRow = 0;

    while (currentRow < matrix->size.y)
    {

        new_matrix.rows[currentRow].values = (int *)malloc(sizeof(int) * matrix->size.x);

        memcpy(new_matrix.rows[currentRow].values, matrix->rows[currentRow].values, sizeof(int) * matrix->size.x);
        currentRow++;

    }

    return new_matrix;
}

// getNextColIndex; no need of const int max_col
Vec2 getNextColIndex(const int row, const int col, const int max_row, const int max_col, char dir)
{
    Vec2 nextPos;
    // Copy over all of the currentPaths knowledge
    nextPos.x = col + 1;
    nextPos.y = row;

    if (dir == 'u')
    {
        nextPos.y -= 1;
        // Flip around if we've reached the top
        if (nextPos.y < 0)
        {
            nextPos.y = max_row - 1;
        }
    }
    else if (dir == 'd')
    {
        nextPos.y += 1;
        // Flip around if we've reached the bottom
        if (nextPos.y >= max_row)
        {
            nextPos.y = 0;
        }
    } else {
        // We're going forward, no need to change y
    }
    return nextPos;
}

// Find the bets path using greedy search
void findBestPath(const Matrix *matrix, Matrix *matScore, Matrix *matPath) {
    char directions[] = {'u', 'd', 'f'};
    // define temp variables
    //int scores[3];
    //int rowPath[3];
    //int min_score = 0;
    //Vec2 position;

    for (int col = matrix->size.x - 2; col >= 0; col--) {
        for (int row = 0; row < matrix->size.y; row++) {
            int index = 0;
            int min_score = 0;
            int scores[3];
            int rowPath[3];
            Vec2 position;

            for (int i = 0; i < 3; i++) {
                char dir = directions[i];

                position = getNextColIndex(row, col, matrix->size.y, matrix->size.x, dir);
                scores[i] = matScore->rows[position.y].values[position.x];
                rowPath[i] = position.y;

                if (i > 0 && (min_score >= scores[i])) {
                    //printf("%d and %d \n", rowPath[index], rowPath[i]);
                    if ((min_score == scores[i]) && (rowPath[index] < rowPath[i])) {
                        //index = i; // do not change
                        //printf("%d and %d \n", rowPath[index], rowPath[i]);
                        rowPath[i] = rowPath[index];// actually not needed

                        //printf("%d and %d \n", rowPath[index], rowPath[i]);
                    }
                    else {
                        index = i;
                    }
                    min_score = scores[i];
                }
                else if(i == 0){
                    index = 0;
                    min_score = scores[0];
                }
            }
            matScore->rows[row].values[col] += scores[index];
            matPath->rows[row].values[col] = rowPath[index];
        }
    }

}






// Usage: ./a.out path/to/matrix
int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    Matrix matrix = getMatrixFromFile(matrixFilename);
    Matrix matScore = copyMatrix(&matrix);
    Matrix matPath = copyMatrix(&matrix);

    findBestPath(&matrix, &matScore, &matPath);

    int minValue = matScore.rows[0].values[0];
    int minRow = 0;


    for (int row = 1; row < matrix.size.y; row++) {
        if (minValue > matScore.rows[row].values[0]){
            minValue = matScore.rows[row].values[0];
            minRow = row;
        }
    }

    //printf("\n minValue %d \n", minValue);

    //printf("\n The path is \n");
    printf("%d ", minRow+1);
    //printf("%d ", minRow+1);
    for (int col = 0; col < matrix.size.x-1; col++) {
        minRow = matPath.rows[minRow].values[col];
        printf("%d ", minRow+1);
    }
    printf("\n");
    printf("%d\n", minValue);
    printf("\n");

    /*
    for (int row = 0; row < matrix.size.x; row++) {
        for (int col = 0; col < matrix.size.y; col++) {
            printf("%d ", matrix.rows[row].values[col]);
        }
        printf("\n");
    }
    printf("\n");
    for (int row = 0; row < matrix.size.x; row++) {
        for (int col = 0; col < matrix.size.y; col++) {
            printf("%d ", matScore.rows[row].values[col]);
        }
        printf("\n");
    }
    printf("\n");
    for (int row = 0; row < matrix.size.y; row++) {
        for (int col = 0; col < matrix.size.x; col++) {
            printf("%d ", matPath.rows[row].values[col]);
        }
        printf("\n");
    }
     */
    return 0;
}
