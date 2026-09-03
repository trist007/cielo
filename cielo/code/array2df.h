#ifndef ARRAY2DF_H
#define ARRAY2DF_H

#include <stdlib.h>
#include <stdio.h>

typedef struct Array2Df Array2Df;
struct Array2Df {
    float* data;
    int    cols;
    int    rows;
};

static inline size_t array2Df_calcIndex(const Array2Df* a, int col, int row)
{
#ifndef NDEBUG
    if (col < 0 || col >= a->cols) {
        printf("%s:%d - column overflow (%d vs %d)\n", __FILE__, __LINE__, col, a->cols);
        exit(0);
    }
    if (row < 0 || row >= a->rows) {
        printf("%s:%d - row overflow (%d vs %d)\n", __FILE__, __LINE__, row, a->rows);
        exit(0);
    }
#endif
    return (size_t)row * a->cols + col;
}

/* equivalent to InitArray2D(Cols, Rows) */
inline void array2Df_init(Array2Df* a, int cols, int rows)
{
    a->cols = cols;
    a->rows = rows;

    if (a->data) {
        free(a->data);
    }

    a->data = (float*)malloc((size_t)cols * rows * sizeof(float));
}

/* equivalent to InitArray2D(Cols, Rows, InitVal) */
static inline void array2Df_initFill(Array2Df* a, int cols, int rows, float initVal)
{
    array2Df_init(a, cols, rows);

    for (int i = 0; i < cols * rows; i++) {
        a->data[i] = initVal;
    }
}

/* equivalent to InitArray2D(Cols, Rows, void* pData) */
static inline void array2Df_initFromData(Array2Df* a, int cols, int rows, float* pData)
{
    a->cols = cols;
    a->rows = rows;

    if (a->data) {
        free(a->data);
    }

    a->data = pData;
}

/* equivalent to Destroy() / destructor */
static inline void array2Df_destroy(Array2Df* a)
{
    if (a->data) {
        free(a->data);
        a->data = NULL;
    }
}

static inline float* array2Df_getAddr(const Array2Df* a, int col, int row)
{
    size_t index = array2Df_calcIndex(a, col, row);
    return &a->data[index];
}

static inline float* array2Df_getBaseAddr(const Array2Df* a)
{
    return a->data;
}

static inline int array2Df_getSize(const Array2Df* a)
{
    return a->rows * a->cols;
}

static inline int array2Df_getSizeInBytes(const Array2Df* a)
{
    return array2Df_getSize(a) * (int)sizeof(float);
}

/* Get(Col, Row) */
inline float array2Df_get(const Array2Df* a, int col, int row)
{
    return *array2Df_getAddr(a, col, row);
}

/* Get(Index) */
static inline float array2Df_getIndex(const Array2Df* a, int index)
{
#ifndef NDEBUG
    if (index >= a->rows * a->cols) {
        printf("%s:%d - index %d is out of bounds (max size %d)\n",
               __FILE__, __LINE__, index, a->rows * a->cols);
        exit(0);
    }
#endif
    return a->data[index];
}

/* At(Col, Row) - returns a pointer since C has no reference return */
static inline float* array2Df_at(Array2Df* a, int col, int row)
{
    size_t index = array2Df_calcIndex(a, col, row);
    return &a->data[index];
}

/* Set(Col, Row, Val) */
static inline void array2Df_set(Array2Df* a, int col, int row, float val)
{
    *array2Df_getAddr(a, col, row) = val;
}

/* Set(Index, Val) */
static inline void array2Df_setIndex(Array2Df* a, int index, float val)
{
#ifndef NDEBUG
    if (index >= a->rows * a->cols) {
        printf("%s:%d - index %d is out of bounds (max size %d)\n",
               __FILE__, __LINE__, index, a->rows * a->cols);
        exit(0);
    }
#endif
    a->data[index] = val;
}

static inline void array2Df_getMinMax(const Array2Df* a, float* min, float* max)
{
    *max = *min = a->data[0];

    for (int i = 1; i < a->rows * a->cols; i++) {
        if (a->data[i] < *min) {
            *min = a->data[i];
        }
        if (a->data[i] > *max) {
            *max = a->data[i];
        }
    }
}

inline void array2Df_normalize(Array2Df* a, float minRange, float maxRange)
{
    float min, max;

    array2Df_getMinMax(a, &min, &max);

    if (max <= min) {
        return;
    }

    float minMaxDelta = max - min;
    float minMaxRange = maxRange - minRange;

    for (int i = 0; i < a->rows * a->cols; i++) {
        a->data[i] = ((a->data[i] - min) / minMaxDelta) * minMaxRange + minRange;
    }
}

static inline void array2Df_printFloat(const Array2Df* a)
{
    for (int y = 0; y < a->rows; y++) {
        printf("%d: ", y);
        for (int x = 0; x < a->cols; x++) {
            printf("%.6f ", a->data[y * a->cols + x]);
        }
        printf("\n");
    }
}

static inline int array2Df_getWidth(const Array2Df* a)  { return a->cols; }
static inline int array2Df_getHeight(const Array2Df* a) { return a->rows; }

#endif /* ARRAY2DF_H */
