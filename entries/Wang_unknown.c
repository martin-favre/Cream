#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

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

typedef struct nodeS
{
	vec2 cell;
	int value;
} nodeS;

typedef struct pathS
{
	nodeS *node_p;
	int len;   //Nr of nodes in this path
} pathS;

typedef struct neighborInfoS
{
	int value[3];
	vec2 neighborCell[3];
} neighborInfoS;


// getline is usually only defined in linux
// if you are in windows and this already does exist for some reason
// and you get some form of duplicate definition error
// due to this function, just remove it. Or rename it.
// Also don't try to understand this function it's literally copy-pasted
#ifndef _WIN32
size_t getOneLine(char **lineptr, size_t *n, FILE *stream)
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
        read = getOneLine(&line, &len, file);
        int *numbers = malloc(sizeof(int) * 10);
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

    while ((read = getOneLine(&line, &len, file)) != -1)
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

void freeMatrix(Matrix *matrix)
{
    for (int i = 0; i < matrix->size.y; i++)
    {
        free(matrix->rows[i].values);
    }
    free(matrix->rows);
}

void printMatrix(const Matrix *matrix)
{
    for (int i = 0; i < matrix->size.y; i++)
    {
        for (int j = 0; j < matrix->size.x; j++)
        {
            printf("%d ", matrix->rows[i].values[j]);
        }
        printf("\n");
    }
}

/* *************************************  Algorithm to find the best path  ***************************************/

/* Init the pathGroup based on a specific cell. Add a new path with only one node into the pathGroup*/
void initPathGroup(pathS **pathPointerArray_pp, vec2 *cell_p, int value)
{
	pathS * path_p = (pathS*)malloc(sizeof(pathS));
	nodeS * node_p = (nodeS*)malloc(sizeof(nodeS));
	
	node_p->value = value;
	node_p->cell.x = cell_p->x;
	node_p->cell.y = cell_p->y;
	path_p->node_p = node_p;
	path_p->len = 1;
	
	//add the node into the pointer array, need to initialize the first column, cell by cell
	*(pathPointerArray_pp + cell_p->x) = path_p;
}

/* Search the 3 neighbor cells for a specific cell, get the values and cell location of the 3 neighbors*/
neighborInfoS searchNeighbor(vec2 *cell_p, const Matrix *matrix)
{
	int nextRow[3], nextColumn;
	neighborInfoS neighborInfo;
	int row = cell_p->x;
	int column = cell_p->y;

    assert(column < matrix->size.x);  //should not search neighbor for cells in the last column

	nextColumn = column + 1;
	if (row == 0)
	{
		nextRow[0] = matrix->size.y - 1;
		nextRow[1] = 0;
		nextRow[2] = 1;
	} else if (row != matrix->size.y - 1)
	{
		nextRow[0] = row - 1;
		nextRow[1] = row;
		nextRow[2] = row + 1;
	} else
	{
		nextRow[0] = row - 1;
		nextRow[1] = row;
		nextRow[2] = 0;
	}
	
	neighborInfo.value[0] =  matrix->rows[nextRow[0]].values[nextColumn];
	neighborInfo.value[1] =  matrix->rows[nextRow[1]].values[nextColumn];
	neighborInfo.value[2] =  matrix->rows[nextRow[2]].values[nextColumn];
	
	for (unsigned i = 0; i < 3; i++)
	{
		neighborInfo.neighborCell[i].x = nextRow[i];
		neighborInfo.neighborCell[i].y = nextColumn;
	}

	return neighborInfo;
}

/* Return which position is the cell in the path.*/
int selectElem(pathS *path_p, vec2 *cell_p)
{
	int len = path_p->len;
	int i;

	for (i = 0; i < len; i++)
	{
		if ((path_p->node_p + i)->cell.x == cell_p->x && (path_p->node_p + i)->cell.y == cell_p->y)
		{
			return i + 1;
		}
	}

	return -1;
}

/* Insert a cell into a path.
   This means add a new node (cell info) and always add it to the the last position of the nodes list*/
void insertElem(pathS *path_p, vec2 *cell_p, int value)
{
	int len = path_p->len;
	path_p->node_p = (nodeS*)realloc(path_p->node_p, (len+1) * sizeof(nodeS));
    (path_p->node_p + len)->cell.x = cell_p->x;
	(path_p->node_p + len)->cell.y = cell_p->y;
	(path_p->node_p + len)->value = value;
	path_p->len = len + 1;
}

int getValueFromPath(pathS *path_p)
{
	int value = 0;
	int len = path_p->len;
	int i;
	for (i = 0; i < len; i++)
	{
		value = (path_p->node_p + i)->value + value;
	}

	return value;
}

/* Check all the existing paths from the path group to see if the cell is inside the path.
   If so, update all paths going through the cell according to the result, with the neighbor infomation got from searchNeighbor */
pathS **updatePathGroup(pathS **pathPointerArray_pp, int *len_p, vec2 *cell_p, neighborInfoS *neighborInfo_p)
{
	pathS *tempPath_p;
	int cellLocation = -1;
	int i;
	int n = *len_p;   //Nr of paths in the pathGroup
	int m = n;  //new length of the path group
	size_t nodesListSize = 0;

	pathS **tempPathPointerArray_pp = (pathS**)malloc(n * sizeof(tempPathPointerArray_pp));
	memcpy(tempPathPointerArray_pp, pathPointerArray_pp, n * sizeof(tempPathPointerArray_pp));

	for (i = 0; i < n; i++)
	{
		tempPath_p = *(tempPathPointerArray_pp + i);
		cellLocation = selectElem(tempPath_p, cell_p);

		if (cellLocation > 0)   //cell is in this path
		{
			m = m + 2;  //One original path will be extended to 3, keep to use the original one, so need to add 2 new ones. 
			pathPointerArray_pp = (pathS**)realloc(pathPointerArray_pp, m * sizeof(pathPointerArray_pp));

            //the cell is always at last position in the nodes list of the path, so cellLocation gives the length info.
			nodesListSize = cellLocation * sizeof(nodeS);
			*(pathPointerArray_pp + m - 2) = malloc(sizeof(pathS));   
			*(pathPointerArray_pp + m - 1) = malloc(sizeof(pathS));
			(*(pathPointerArray_pp + m - 2))->node_p = (nodeS*)malloc(nodesListSize);
			(*(pathPointerArray_pp + m - 1))->node_p = (nodeS*)malloc(nodesListSize);
			memcpy((void*)(*(pathPointerArray_pp + m - 2))->node_p, (void*)tempPath_p->node_p, nodesListSize);
			memcpy((void*)(*(pathPointerArray_pp + m - 1))->node_p, (void*)tempPath_p->node_p, nodesListSize);

			(*(pathPointerArray_pp + m - 2))->len = cellLocation;
			(*(pathPointerArray_pp + m - 1))->len = cellLocation;
			//insert the three neighors to three paths which include the original one and the two new ones.
			insertElem(*(pathPointerArray_pp + m - 2), &(neighborInfo_p->neighborCell[0]), neighborInfo_p->value[0]);
			insertElem(*(pathPointerArray_pp + m - 1), &(neighborInfo_p->neighborCell[1]), neighborInfo_p->value[1]);
			//update the origin path to a new extended path
			insertElem(*(pathPointerArray_pp + i), &(neighborInfo_p->neighborCell[2]), neighborInfo_p->value[2]);  
		}

		cellLocation = -1;
	}
	
	free(tempPathPointerArray_pp);
	*len_p = m;
	
	return pathPointerArray_pp;
}


/*Go through all cells, seaching each neighbor and update paths that passes through the specific cell */
pathS **goThroughCells(const Matrix *matrix, pathS **pathPointerArray_pp, int *len_p)
{
	vec2 cell;
	*len_p = matrix->size.y;
	neighborInfoS neighborInfo;

	// go through all cells and get all the paths
	/* Don't search the last column because cells there have no forward neighbors. 
	   Go through row to row firstly*/
	for (int j = 0; j < matrix->size.x -1; j++)
	{
		for (int k = 0; k < matrix->size.y; k++)
		{
			cell.x = k;
			cell.y = j;
			neighborInfo = searchNeighbor(&cell, matrix);

			pathPointerArray_pp = updatePathGroup(pathPointerArray_pp, len_p, &cell, &neighborInfo);
		}
	}
	
	return pathPointerArray_pp;
}

/* Free all nodes inside a particular path */
void freeNodes(pathS* path_p)
{
	nodeS *node_p = path_p->node_p;
	int len = path_p->len;
	int i;
	for (i =0; i < len; i++)
	{
		free((node_p + i));
	}
}

/* Free all paths from the pathGroup */
void freePaths(pathS **pathPointerArray_pp, int *len_p)
{
	int nrOfPaths = *len_p;
	int i, j;
	for (i = 0; i < nrOfPaths; i++)
	{
		freeNodes(*(pathPointerArray_pp + i));
		free(*(pathPointerArray_pp + i));
	}
}

/*Go through all the cells and then get all the paths.
  Compare the total value of each path and get the best one */
nodeS *getBestPath(const Matrix *matrix, int *minValue_p, int *pathLen_p)
{
	pathS **pathPointerArray_pp = malloc(matrix->size.y * sizeof(pathPointerArray_pp));
    vec2 cell;
	int value;
	int tempValue, minValue;
	int bestPathNr = 0;
	int pathLen = 0;
	int pathGroupLen = 0;
	int *len_p = &pathGroupLen;
	pathS *path_p;
	nodeS* nodes_p;

	//init the path for the cells of the first column
	for (int i = 0; i < matrix->size.y; i++)
	{
		value = matrix->rows[i].values[0];
		cell.x = i;
		cell.y = 0;   //The first column
		initPathGroup(pathPointerArray_pp, &cell, value);
	}

    // go through all cells, including the cells of the first column 
	pathPointerArray_pp = goThroughCells(matrix, pathPointerArray_pp, len_p);

	//compare value in the path group
	path_p = *(pathPointerArray_pp);
	minValue = getValueFromPath(path_p);
	for (int j = 1 ; j < *len_p; j++)
	{
		path_p = *(pathPointerArray_pp + j);
		tempValue = getValueFromPath(path_p);

		if (minValue > tempValue)
		{
			minValue = tempValue;
			bestPathNr = j;
		}
	}

    *minValue_p = minValue;   

	pathLen = (*(pathPointerArray_pp + bestPathNr))->len;
	nodes_p = (nodeS*)malloc(pathLen * sizeof(nodeS));
	memcpy(nodes_p, (*(pathPointerArray_pp + bestPathNr))->node_p, pathLen * sizeof(nodeS));
	freePaths(pathPointerArray_pp, len_p);

	*pathLen_p = pathLen;

	return nodes_p;
}

/*Implement the who algorithm */
void findBestPathAndPrint(Matrix *matrix_p)
{
	int minValue;
	int pathLen = 0;
	int *pathLen_p = &pathLen;
	int i;

	nodeS *nodes_p = getBestPath(matrix_p, &minValue, pathLen_p);
	pathLen = *pathLen_p;

	for (i = 0; i < pathLen; i++)
	{
		printf("cell: [%d %d]\n", (nodes_p + i)->cell.x, (nodes_p + i)->cell.y);
	}
	printf("\n");
	printf("The min value is %d\n", minValue);
	free(nodes_p);
}


// Usage:
// ./a.out path/to/mymatrixfile
int main(int argc, char *argv[])
{
    char *matrixFilename = argv[1];
    Matrix matrix = getMatrixFromFile(matrixFilename);
	// Don't run this printing if the test matrix is too big
    printMatrix(&matrix);
	printf("\n");

	printf("Now to get the best path as below: \n");
    findBestPathAndPrint(&matrix);

    freeMatrix(&matrix);
	
	return 0; 
}