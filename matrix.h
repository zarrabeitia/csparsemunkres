#ifndef MATRIX_H
#define MATRIX_H
#include "limits.h"
#include <vector>
using namespace std;

#define BIGVALUE 1e10
#define INF 1e1000
#define EPS 1e-6
#define NOTFOUND UINT_MAX

typedef unsigned int uint;

typedef struct _coords {
    uint i, j;
} coords;

typedef struct _entry {
    coords pos;
    double cost;
} entry;

/*
   Auxiliary class.
   Stores the sparse representation of the munkres matrix.
   The user should remap the rows and columns to avoid having emtpy rows and columns.
*/
class Matrix
{
public:
    Matrix(vector<entry> entries);

    // Returns the non-empty entries in row rowindex
    vector<entry> row(uint rowindex);
    // Returns the current values of the matrix.
    vector<entry> get_values();
    // Adds value to all the elements of column colindex.
    void add_to_column(uint colindex, double value);
    // Adds value to all the elements of row rowindex.
    void add_to_row(uint rowindex, double value);
    // Returns all zero (but non-empty) entries in the matrix
    // An element is considered to be zero if abs(value) <= EPS.
    vector<uint> zeros();

    // Returns the index of an uncovered zero.
    // If no uncovered zero is found, return NOTFOUND
    uint find_uncovered_zero();

    ~Matrix();

    // Auxilliary function: adds the corresponding row_adds and column_adds to the entry.
    // Needed because internally, we never update the original values.
    entry current_value(uint index);

    uint nrows, ncols, real_columns;
    uint nentries();
    double min_uncovered_cost();
    bool* covered_rows;
    bool* covered_columns;

    vector<entry> entries;

protected:

    // this probably wont be needed.
    // entry current_value(entry &e);
    double *column_adds;
    double *row_adds;
};

#endif // MATRIX_H
