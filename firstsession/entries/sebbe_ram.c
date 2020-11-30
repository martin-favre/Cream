#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

typedef struct vec2
{
    int x;
    int y;
} vec2;

typedef struct Row
{
    int *values;
} Row;

typedef struct Matrix
{
    Row *rows;
    vec2 size;
} Matrix;

/**********************************/
/**
/** MY NEO STRUCTS 
/** @Author Sebastian Kullengren
/**
/**********************************/
typedef struct NeoElement
{
    int sum;
	int noSteps;
	int* nextSteps;
} NeoElement;

typedef struct NeoColumn
{
    NeoElement *neoElements; // rows
} NeoColumn;

typedef struct NeoMatrix
{
    NeoColumn *neoColumns;
    vec2 size;
} NeoMatrix;

int DEBUG = 0; // set to 1 for more printouts

/**********************************/

//if in windows
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
    int i;
    for(i = 0; i < lineSize; i++)
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
    //free(currNumberBuff);
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

        matrix.rows[currentRow].values = malloc(sizeof(int) * matrix.size.x);
        int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
        getNumbersFromLine(line, read, matrix.rows[currentRow].values, &unusedBecauseIAmABadBoiWhoDoesNoChecks);
        currentRow++;
    }
    free(line);
    fclose(file);

    return matrix;
}

void freeMatrix(Matrix *matrix)
{
    int i;
    for(i = 0; i < matrix->size.y; i++)
    {
        free(matrix->rows[i].values);
    }
    free(matrix->rows);
}

void printMatrix(const Matrix *matrix)
{
    int i;
    for( i = 0; i < matrix->size.y; i++)
    {
        int j;
        for(j = 0; j < matrix->size.x; j++)
        {
            printf("%d ", matrix->rows[i].values[j]);
        }
        printf("\n");
    }
}

/**********************************/
/**
/** MY FUNCTIONS 
/** @Author Sebastian Kullengren
/**
/**********************************/
void printDebug(char* msg){
	if(DEBUG) printf("%s",msg);
}

void printNeoMatrix(const NeoMatrix *matrix)
{
    int i;
    for(i = 0; i < matrix->size.y; i++)
    {
        int j;
        for(j = 0; j < matrix->size.x; j++)
        {
            printf("%d ", matrix->neoColumns[j].neoElements[i].sum);
        }
        printf("\n");
    }
}

// Simple calculation of next step, given from what row and the max rows in a column
int nextStep(int thisRow, int noRows, const NeoColumn *column)
{
	printDebug("\nENTERING nextStep");
	// Next steps are as follows:
	// We want 'a' to be lowest index, 'b' second lowest, and 'c' highest.
	// By default this means => a(up) b(mid) c(down)
	int a = thisRow - 1;
	int b = thisRow;
	int c = thisRow + 1;
	
	// column top edge case => a(mid) b(down) c(up, wrapping around)
	if(thisRow == 0){
		a = 0;
		b = 1;
		c = noRows - 1;
	}
	// column bottom edge case => a(down, wrapping around) b(up) c(mid)
	else if(thisRow == noRows - 1) {
		a = 0;
		b = thisRow - 1;
		c = thisRow;
	}

	int nextRow = a; // prio to lowest index
	if (column->neoElements[b].sum < column->neoElements[nextRow].sum) {
		nextRow = b;
	}
	if(column->neoElements[c].sum < column->neoElements[nextRow].sum) {
		nextRow = c;
	}
	if(DEBUG)printf(", (from row %d to row %d)", thisRow + 1, nextRow + 1);
	return nextRow;
}

// Collapses a NeoMatrix to a single column, recursively
NeoMatrix* compressMatrix(NeoMatrix *matrix)
{
	printDebug("\nENTERING compressMatrix");

	int noRows = matrix->size.y;
	int noColumns = matrix->size.x;

    // check we have reached single column matrix
	if(noColumns < 2) {
		return matrix;
	}
	
	// start work to collapse columnB into columnA

	NeoColumn *columnB = matrix->neoColumns + noColumns - 1;
	NeoColumn *columnA = matrix->neoColumns + noColumns - 2;

	// For each row of columnA, aggregate the info of each row with *shortest path* of columnB 
	// into each row of columnA.
	// We end up with a columnA of rows that has a total sum of its own value and the value of the 
	// next row with shortest path. Each element also has the full list of the steps taken by the
	// columnB elements.
    int thisRow;
	for(thisRow = 0; thisRow < noRows; thisRow++) {
		int noNewSteps = columnB->neoElements[thisRow].noSteps + 1;
		columnA->neoElements[thisRow].noSteps = noNewSteps;
		columnA->neoElements[thisRow].nextSteps = malloc(sizeof(int) * noNewSteps);
		
		// get the next step for this row
		int nextElement = nextStep(thisRow, noRows, columnB);
		columnA->neoElements[thisRow].nextSteps[0] = nextElement; // first step
		
		// we also want to append all the steps that the next step element has taken
        int i;
		for(i = 0; i < noNewSteps - 1; i++){
			columnA->neoElements[thisRow].nextSteps[i + 1] = columnB->neoElements[nextElement].nextSteps[i];
		}
		
		// now we simply add the sums together
		int newSum = columnA->neoElements[thisRow].sum + columnB->neoElements[nextElement].sum;
		columnA->neoElements[thisRow].sum = newSum;
	}

	// Free up Column B, as Column A has all info
    int i;
	for(i = 0; i < noRows; i++){
		if(columnB->neoElements[i].noSteps > 0) {
			free(columnB->neoElements[i].nextSteps);
			columnB->neoElements[i].noSteps = 0;
		}
	}
	free(columnB->neoElements);
	
	// matrix has now shrunk by one column
	noColumns--;
	matrix->size.x = noColumns;
	
	printDebug("\n Compression done!\n");
	if(DEBUG)printNeoMatrix(matrix);
	
	// Recursion, keep going until we have single column
	return compressMatrix(matrix);
}

// Convert a Matrix struct* to a NeoMatrix struct*
// This will allocate new memory of same size as the Matrix
NeoMatrix* matrixToNeoMatrix(const Matrix *matrix)
{
	printDebug("\nENTERING matrixToNeoMatrix");
	NeoMatrix* neoMatrix = malloc(sizeof(NeoMatrix));
	int noRows = matrix->size.y;
	int noColumns = matrix->size.x;
    neoMatrix->size.x = noColumns;
	neoMatrix->size.y = noRows;
	neoMatrix->neoColumns = malloc(sizeof(NeoColumn) * noColumns);
    int i;
	for(i = 0; i < noColumns; i++)
	{
        int j;
		neoMatrix->neoColumns[i].neoElements = malloc(sizeof(NeoElement) * noRows);
		for(j = 0; j < noRows; j++)
		{
			neoMatrix->neoColumns[i].neoElements[j].sum = matrix->rows[j].values[i];
			neoMatrix->neoColumns[i].neoElements[j].noSteps = 0;
		}
	}	
	return neoMatrix;
}

void printShortestPath(const Matrix *matrix)
{	
	printDebug("\nENTERING printShortestPath");
	int noRows = matrix->size.y;
	
	// Convert matrix to a NeoMatrix
	NeoMatrix *neoMatrix;
    neoMatrix = matrixToNeoMatrix(matrix);
	// Compress the matrix to get a single column with short path sums
    compressMatrix(neoMatrix);
	NeoElement *rows = neoMatrix->neoColumns->neoElements;
	// go through column to find row with lowest sum
	NeoElement shortestPathEntry = rows[0];
	int shortestStartRow = 0;
    int row;
	for(row = 1; row < noRows; row++) {
		if(rows[row].sum < shortestPathEntry.sum) {
			shortestPathEntry = rows[row];
			shortestStartRow = row;
		}
	}
	
	printDebug("\n#### Printing shortest path ####\n");
	printf("%d ", shortestStartRow + 1);
        int i;
	for(i = 0; i < shortestPathEntry.noSteps; i++)
	{
		printf("%d ", shortestPathEntry.nextSteps[i] + 1);
	}
	printf("\n%d", shortestPathEntry.sum);
	printf("\n");
	
	// Free memory
	for(i = 0; i < noRows; i++){
		free(rows[i].nextSteps);
	}
	free(rows);
	free(neoMatrix->neoColumns);
	free(neoMatrix);
}

// Usage:
// ./output mymatrixfile 
// optional:
// ./output mymatrixfile debug
int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
	if(argv[2])	DEBUG = 1;
    Matrix matrix = getMatrixFromFile(matrixFilename);

    // printMatrix(&matrix);
	printShortestPath(&matrix);
    //freeMatrix(&matrix);
    return 0;
}
