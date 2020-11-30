#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#define WRAP(row) ((row) + matrix->size.y) % matrix->size.y
#define NODE_V(i, j)  matrix->rows[(i)].nodes[(j)].value
#define NODE_U(i, j)  matrix->rows[(i)].nodes[(j)].up
#define NODE_S(i, j)  matrix->rows[(i)].nodes[(j)].straight
#define NODE_D(i, j)  matrix->rows[(i)].nodes[(j)].down

typedef struct vec2
{
  int x;
  int y;
} vec2;

typedef struct Node
{
  int value;
  int up;
  int straight;
  int down;
} Node;

typedef struct Row
{
  Node* nodes;
} Row;

typedef struct Matrix
{
  Row *rows;
  vec2 size;
} Matrix;

typedef struct Outcome
{
  int* minRow;
  int* tmpRow;
  int* minPath;
  int* tmpPath;
  int  minWeight;
  int  tmpWeight;
  int  len;
} Outcome;

Outcome outcome = {0};

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

void getNumbersFromLine(const char *line, const ssize_t lineSize, Node outBuff[], int *outBuffSize)
{
  int i = 0;
  //worst case scenario, whole line is a number. +1 for null termination
  char *currNumberBuff = malloc(sizeof(char) * lineSize + 1);
  int currNumberSize = 0;
  for (i = 0; i < lineSize; i++)
  {
    const char currChar = line[i];
    if (currChar == ' ' || currChar == '\n')
    {
      if (currNumberSize > 0)
      {
        currNumberBuff[currNumberSize] = 0; // null terminate so atoi works
        outBuff[*outBuffSize].value = atoi(currNumberBuff);
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
  Matrix matrix = {0};
  FILE *file = fopen(filename, "r");
  assert(file != NULL);
  {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    read = getline(&line, &len, file);
    Node *nodes = malloc(sizeof(Node) * 2);
    int numberOfNumbers = 0;
    getNumbersFromLine(line, read, nodes, &numberOfNumbers);
    free(line);
    assert(numberOfNumbers == 2);
    matrix.size.y = nodes[0].value;
    matrix.size.x = nodes[1].value;
    free(nodes);
    outcome.len = matrix.size.x;
    outcome.minPath = malloc(sizeof(int) * outcome.len);
    outcome.tmpPath = malloc(sizeof(int) * outcome.len);
    outcome.minRow = malloc(sizeof(int) * outcome.len);
    outcome.tmpRow = malloc(sizeof(int) * outcome.len);
    outcome.minWeight = 0x3FFFFFFF;
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
      matrix.rows[currentRow].nodes = malloc(sizeof(Node) * matrix.size.x);
      memset(matrix.rows[currentRow].nodes, 0, sizeof(Node) * matrix.size.x);
      int unusedBecauseIAmABadBoiWhoDoesNoChecks = 0;
      getNumbersFromLine(line, read, matrix.rows[currentRow].nodes, &unusedBecauseIAmABadBoiWhoDoesNoChecks);
      currentRow++;
    }
  }
  free(line);
  fclose(file);

  return matrix;
}

void freeMatrix(Matrix *matrix)
{
  int i = 0;

  for (i = 0; i < matrix->size.y; i++)
  {
    free(matrix->rows[i].nodes);
  }
  free(matrix->rows);
}

void printMatrix(const Matrix *matrix)
{
  int i = 0;
  int j = 0;

  for (i = 0; i < matrix->size.y; i++)
  {
    for (j = 0; j < matrix->size.x; j++)
    {
      printf("%d ", matrix->rows[i].nodes[j].value);
    }
    printf("\n");
  }
}

int lexiCompare(const int* seq1, const int* seq2)
{
  int i = 0;

  for(i = 0; seq1[i] == seq2[i]; i++)
  {
    if(i == outcome.len)
    {
      return 0;
    }
  }

  return (seq1[i] < seq2[i]) ? 1 : 0;
}

void updatePath()
{
  int i = 0;

  for(i = 0; i < outcome.len; i++)
  {
    outcome.minPath[i] = outcome.tmpPath[i];
    outcome.minRow[i] = outcome.tmpRow[i];
  }
}

int calculateWeight(const int* path)
{
  int w = 0;
  int i = 0;

  for(i = 0; i < outcome.len; i++)
  {
    w += path[i];
  }

  return w;
}

void printPath()
{
  int w = 0;
  int i = 0;

  for (i = 0; i < outcome.len; i++)
  {
    w += outcome.minPath[i];
    printf("%d ", outcome.minRow[i]);

    if(i == outcome.len -1)
    {
      printf("\n%d\n", w);
    }
  }
}

void updateOutcome(void)
{
  int tmpWeight = calculateWeight(outcome.tmpPath);

  if(tmpWeight < outcome.minWeight)
  {
    outcome.minWeight = tmpWeight;
    updatePath();
  }
  else if(tmpWeight == outcome.minWeight)
  {
    if(lexiCompare(outcome.tmpRow, outcome.minRow))
    {
      outcome.minWeight = tmpWeight;
      updatePath();
    }
  }
}

void dfs(const Matrix *matrix, const int row, const int col)
{
  outcome.tmpPath[col] = NODE_V(row, col);
  outcome.tmpRow[col] = row + 1;

  if(col == matrix->size.x - 1)
  {
    updateOutcome();
  }
  else
  {
    if(!NODE_U(row, col))
    {
      dfs(matrix, WRAP(row - 1), col + 1);
      NODE_U(row, col) = 1;
    }

    if(!NODE_S(row, col))
    {
      dfs(matrix, row, col + 1);
      NODE_S(row, col) = 1;
    }

    if(!NODE_D(row, col))
    {
      dfs(matrix, WRAP(row + 1), col + 1);
      NODE_D(row, col) = 1;
    }

    NODE_U(row, col) = 0;
    NODE_S(row, col) = 0;
    NODE_D(row, col) = 0;
  }
}

void findPath(const Matrix *matrix)
{
  int i = 0;

  for(i = 0; i < matrix->size.y; i++)
  {
    dfs(matrix, i, 0);
  }
}

// Usage:
// ./a.out path/to/mymatrixfile
int main(int argc, char *argv[])
{
  char *matrixFilename = argv[1];
  Matrix matrix = getMatrixFromFile(matrixFilename);

  findPath(&matrix);
  printPath();
  freeMatrix(&matrix);
  free(outcome.tmpPath);
  free(outcome.minPath);
  free(outcome.tmpRow);
  free(outcome.minRow);

  return 0;
}