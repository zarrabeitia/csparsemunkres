#ifndef MUNKRES_H
#define MUNKRES_H

#include "matrix.h"
#include <set>


class Munkres
{
public:
    Munkres(vector<entry> values);

    // Run the algorithm, return optimal matching
    vector<entry> munkres();

    // Munkres steps. Each step returns the index of the next step.
    short step_1();
    short step_2();
    short step_3();
    short step_4();
    short step_5();
    short step_6();
    ~Munkres();

//protected:
    Matrix *matrix;

    //uint *starred; // starred[i] == j if [i,j] is starred. starred[i] >= ncols+1 if there are no stars in this row.
    //bool *covered_rows; // covered_rows[i] iif the i-th column is covered.
    //bool *covered_columns; // covered_columns[j] iif the j-th column is covered.

    set<uint> starred;
    set<uint> primed;
    typedef set<uint>::iterator uint_set_iterator;

    uint last_primed_index;
};

vector<entry> munkres(vector<entry> &values);

#endif // MUNKRES_H
