#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <string.h> 
#include <assert.h> 
#include <limits.h> 
  
typedef enum { false, true } bool; 
  
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
  
typedef struct Path 
{ 
    int *row; 
} Path; 
  
typedef struct TrackingInfo 
{ 
    bool leftPathUsed; 
    bool forwardPathUsed; 
    bool rightPathUsed; 
    int  rowNo; 
} TrackingInfo; 
  
  
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
  
    for (i = 0; i < lineSize; i++) 
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
    int i; 
  
    for (i = 0; i < matrix->size.y; i++) 
    { 
        free(matrix->rows[i].values); 
    } 
    free(matrix->rows); 
} 
  
void printMatrix(const Matrix *matrix) 
{ 
    int i,j; 
  
    for (i = 0; i < matrix->size.y; i++) 
    { 
        for (j = 0; j < matrix->size.x; j++) 
        { 
            printf("%d ", matrix->rows[i].values[j]); 
        } 
        printf("\n"); 
    } 
} 
  
void updatePath(Path   *path,
                int    *outputResult,
                const  Matrix *matrix, 
                int    rowNo, 
                int    colNo) 
{ 
    path->row[colNo] = matrix->rows[rowNo].values[colNo];
    outputResult[colNo] = rowNo + 1; 
    
} 
  
void initTrackingInfo(TrackingInfo *trackingInfo, int colNo ) 
{ 
    // Init tracking info for a certain column 
     
    trackingInfo[colNo].leftPathUsed = false; 
    trackingInfo[colNo].forwardPathUsed = false; 
    trackingInfo[colNo].rightPathUsed = false; 
    trackingInfo[colNo].rowNo = -1; 
} 
  
bool getNextRowNo(TrackingInfo *trackingInfo, 
                  int           noOfRows, 
                  int           rowNo, 
                  int           colNo, 
                  int          *nextRowNo) 
{ 
    // Get next row number in the path. 
    // Search in order "left - forward - right". 
    // If all directions have been used, return false. 
    
    bool pathExists=true; 
  
    if (!trackingInfo[colNo].leftPathUsed) { 
        trackingInfo[colNo].leftPathUsed=true; 
        *nextRowNo = (rowNo-1 < 0) ? noOfRows-1 : rowNo-1; 
        trackingInfo[colNo].rowNo = rowNo; 
    } 
    else if (!trackingInfo[colNo].forwardPathUsed) { 
        trackingInfo[colNo].forwardPathUsed=true; 
        *nextRowNo = rowNo; 
    } 
    else if (!trackingInfo[colNo].rightPathUsed) { 
        trackingInfo[colNo].rightPathUsed=true; 
        *nextRowNo = (rowNo+1 == noOfRows) ? 0 : rowNo+1; 
    } 
    else { 
        pathExists=false; 
    } 
  
    return pathExists; 
} 
  
void findSmallestPath(const Matrix *matrix) 
{ 
    int noOfRows = matrix->size.y; 
    int noOfCols = matrix->size.x; 
    int rowNo,colNo, i; 
    int nextRowNo;  
    bool pathExists, isSmallest, stopSearch; 
    Path path, smallestPath;
    int  weight;
    int  smallestWeight = INT_MAX; 
    TrackingInfo *trackingInfo = malloc(sizeof(TrackingInfo)*noOfCols);
    int *outputResult = malloc(sizeof(int)*noOfCols); 
    int *smallestOutputResult = malloc(sizeof(int)*noOfCols);
  
    // Search for the smallest path 
  
    smallestPath.row = malloc(sizeof(Row)*noOfCols); 
    path.row = malloc(sizeof(Row)*noOfCols);
 
    for (rowNo=0; rowNo < noOfRows; rowNo++ ) { 
        for (i = 0; i < noOfCols; i++) { 
           initTrackingInfo(trackingInfo, i); 
        }
             
        colNo=0;         
        while (true) { 
            updatePath(&path, outputResult, matrix, rowNo, colNo); 
            if (colNo == noOfCols-1) { 
  
                // A complete path is found. 
                // Check if this is the path with smallest weight so far. 
  
                weight = 0; 
                for (i = 0; i < noOfCols; i++) { 
                    weight += path.row[i]; 
                } 
  
                if (weight == smallestWeight) { 
                     
                    // Check if the path is lexicographical smallest 
                    
                    isSmallest = false; 
                    stopSearch = false; 

                    for (i = 0; i < noOfCols && !stopSearch; i++) { 
                        if (smallestOutputResult[i] < outputResult[i]) { 
                            isSmallest = false; 
                            stopSearch=true; 
                        } 
                        else if (outputResult[i] < smallestOutputResult[i]) { 
                            isSmallest = true; 
                            stopSearch=true; 
                        } 
                    } 
 
                }    
                else { 
                    isSmallest = weight < smallestWeight; 
                } 
  
                if (isSmallest) { 
                    for (i = 0; i < noOfCols; i++) { 
                        smallestPath.row[i] = path.row[i];
                        smallestOutputResult[i] = outputResult[i];   
                    } 
                    smallestWeight = weight; 
                } 
                             
                // Go back on step and continue 
  
                colNo--; 
                rowNo = trackingInfo[colNo].rowNo; 
            } 
            else { 
  
                // Not a complete path yet 
  
                if (pathExists=getNextRowNo(trackingInfo, noOfRows, rowNo,  
                                            colNo, &nextRowNo)) { 
  
                    // Go forward one step 
  
                    rowNo = nextRowNo; 
                    colNo++; 
                } 
                else { 
                    if (colNo == 0) { 
                        // All paths have been found for this starting row 
                        break; 
                    } 
                    else { 
                        // Go back one step and continue the search in a new direction 
                        initTrackingInfo(trackingInfo, colNo); 
                        colNo--; 
                        rowNo = trackingInfo[colNo].rowNo; 
                    } 
                }          
            } 
        } 
    }  
  
    for (i = 0; i < noOfCols; i++) { 
        printf("%d ", smallestOutputResult[i]);
    }
    printf("\n%d\n", smallestWeight);

    free(trackingInfo);
    free(outputResult);
    free(smallestOutputResult);
} 
  
  
// Usage: 
// ./a.out path/to/mymatrixfile 
int main(int argc, char *argv[]) 
{ 
    char *matrixFilename = argv[1]; 
    // char *matrixFilename = "testmatrix"; 
    Matrix matrix = getMatrixFromFile(matrixFilename); 
      
    //printMatrix(&matrix); 
    findSmallestPath(&matrix); 
    freeMatrix(&matrix); 
}

