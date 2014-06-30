#include "munkres.h"
#include "limits.h"
#include "assert.h"

#include "iostream"
#include <map>

ostream& operator <<(ostream &s, Munkres *m) {
    for (uint i = 0; i < m->matrix->nentries(); i++) {
        entry e = m->matrix->current_value(i);
        s << " (" << e.pos.i << "," << e.pos.j <<")->" << e.cost;
        if (m->starred.find(i)!= m->starred.end()) { s << "*"; }
        if (m->primed.find(i) != m->primed.end()) { s << "p"; }
        s << endl;
    }
    cout << "covered_rows: ";
    for (uint i = 0; i < m->matrix->nrows; i++) cout << m->matrix->covered_rows[i] <<" ";
    cout << endl;
    cout << "covered_cols: ";
    for (uint i = 0; i < m->matrix->ncols; i++) cout << m->matrix->covered_columns[i] <<" ";
    cout << endl;
    return s;
}

Munkres::Munkres(vector<entry> values)
{
    this->matrix = new Matrix(values);
}

Munkres::~Munkres() {
    delete this->matrix;
}

typedef short(Munkres::*step_pointer)();
vector<entry> Munkres::munkres() {
    vector<entry> res;
    step_pointer steps[] = {&Munkres::step_1, &Munkres::step_2, &Munkres::step_3, &Munkres::step_4, &Munkres::step_5, &Munkres::step_6};
    short next_step = 1;
//    cout << "Extended matrix:" << endl << this << endl;
    int count = 0;
    while (next_step >= 1) {
//        cout << "Next step is " << next_step << " at iteration " << count << endl;
        next_step = (this->*steps[next_step-1])();
//        cout << this << endl;
        count++;
    }
    for (set<uint>::iterator it=starred.begin(); it!=starred.end(); ++it) {
        uint starred_index = *it;
        entry &starred = matrix->entries[starred_index];
        if (starred.pos.j < this->matrix->real_columns) {
            res.push_back(starred);
        }
    }
    return res;
}

/*
 * For each row of the matrix, find the smallest element and subtract it
 * from every element in its row. Go to Step 2.
 */
short Munkres::step_1() {
    double rowmins[matrix->nrows];
    std::fill_n(rowmins,matrix->nrows, INF);
    for (uint index = 0; index < matrix->nentries(); index++) {
        entry e = matrix->current_value(index);
        if (e.cost < rowmins[e.pos.i]) {
            rowmins[e.pos.i] = e.cost;
        }
//        rowmins[e.pos.i] = min(rowmins[e.pos.i], e.cost);
    }
    for (uint i = 0; i < matrix->nrows; i++) {
        matrix->add_to_row(i, -rowmins[i]);
    }
    return 2;
}

/*
 * Find a zero (Z) in the resulting matrix. If there is no starred zero
 * in its row or column, star Z.
 * Repeat for each zero in the matrix.
 * Go to Step 3.
 */
short Munkres::step_2() {
    vector<uint> zeros = matrix->zeros();
    for (uint iz = 0; iz < zeros.size(); iz++) {
        uint zero_index = zeros[iz];
        entry zero = matrix->current_value(zero_index);
        bool starred_found = false;
        for (set<uint>::iterator it=starred.begin(); it!=starred.end(); ++it) {
            uint starred_index = *it;
            entry starred = matrix->current_value(starred_index);
            if (zero.pos.i == starred.pos.i || zero.pos.j == starred.pos.j) {
                starred_found = true;
                break; // There is a starred element in its row or column.
            }
        }
        if (!starred_found) {
            starred.insert(zero_index);
        }
    }
    assert (this->starred.size() <= this->matrix->nrows);
    return 3;
}

/*
 * Cover each column containing a starred zero. If K columns are covered,
 * the starred zeros describe a complete set of unique assignments.
 * In this case, go to DONE, otherwise, go to step 4.
 */
short Munkres::step_3() {
    for (set<uint>::iterator it=starred.begin(); it!=starred.end(); ++it) {
        uint starred_index = *it;
        entry starred = matrix->current_value(starred_index);
        matrix->covered_columns[starred.pos.j] = true;
    }
    uint count = 0;
    for (uint i = 0; i<matrix->ncols; i++) {
        count += matrix->covered_columns[i];
    }
    if (count == matrix->nrows) {
        return -1;
    } else {
        return 4;
    }
}

/*
 * Find a noncovered zero and prime it. If there is no starred zero in
 * the row containing this primed zero, go to step 5. Otherwise, cover
 * this row and uncover the column containing the starred zero.
 * Continue in this manner until there are no uncovered zeros left.
 * Save the smallest uncovered value and go to step 6.
 * (We'll find the smallest uncovered value in step 6.)
 */
short Munkres::step_4() {
    bool done = false;
    while (!done) {
        uint zero_index = matrix->find_uncovered_zero();
        if (zero_index < NOTFOUND) {
            entry zero = matrix->current_value(zero_index);
            primed.insert(zero_index);
            last_primed_index = zero_index;
            entry starred_zero;
            // see if there is a starred zero in the row zero.pos.i
            bool found_star = false;
            for (set<uint>::iterator it=starred.begin(); it!=starred.end(); ++it) {
                uint starred_index = *it;
                starred_zero = matrix->current_value(starred_index);
                if (starred_zero.pos.i == zero.pos.i) {
                    found_star = true;
                    break;
                }
            }
            if (!found_star) { // There was no star in this row, go to step 5.
                return 5;
            }
            matrix->covered_rows[zero.pos.i] = true; // cover this row.
            matrix->covered_columns[starred_zero.pos.j] = false; // uncover the starred column.

        } else {
            done = true;
        }
    }
    return 6;
}

/* Construct a series of alternating primed and starred zeros as follows:
 * Let Z0 represent the uncovered primed zero found in step 4. Let Z1
 * denote the starred zero in the column of Z0 (if any). Let Z2 denote
 * the primed zero in the row of Z1 (there will always be one). Continue
 * until the series terminates at a primed zero that has no starred zero
 * in its column. Unstar each starred zero of the series, star each
 * primed zero of the series, erase all primes and uncover every line in
 * the matrix. Return to step 3.
 */
short Munkres::step_5() {
    uint last_starred_index;
    uint last_primed_index = this->last_primed_index;
    vector<uint> seq_primed;
    seq_primed.push_back(last_primed_index);
    vector<uint> seq_starred;
    while(true) {
        entry last_primed = matrix->current_value(last_primed_index);
        entry last_starred;
        // find the starred zero in the same column of last_primed
        last_starred_index = NOTFOUND;
        for (uint_set_iterator it=starred.begin(); it!=starred.end(); ++it) {
            uint si = *it;
            entry e = matrix->current_value(si);
            if (e.pos.j == last_primed.pos.j) {
                assert (e.pos.j == last_primed.pos.j);
                last_starred_index = si;
                last_starred = e;
                break;
            }
        }
        if (last_starred_index==NOTFOUND) {
            break; // Couldn't find a starred zero, the sequence is complete.
        }

        seq_starred.push_back(last_starred_index);

        assert (last_starred.pos.j == last_primed.pos.j); // They must be in the same column.

        // Find the primed zero in the row of last_starred
        last_primed_index = NOTFOUND;
        for (uint_set_iterator it=primed.begin(); it!=primed.end(); ++it) {
            uint pi = *it;
            entry e = matrix->current_value(pi);
            if (e.pos.i == last_starred.pos.i) {
                last_primed_index = pi;
                break;
            }
        }
        assert(last_primed_index != NOTFOUND); // it should always exist.
        seq_primed.push_back(last_primed_index);
    }

    // Sequence is complete. Unstar seq_starred
    for (uint si = 0; si < seq_starred.size(); si++) {
        uint index = seq_starred[si];
        starred.erase(index);
    }
    // Star all the from seq_primed
    for (uint pi = 0; pi < seq_primed.size(); pi++) {
        uint index = seq_primed[pi];
        starred.insert(index);
    }
    assert (this->starred.size() <= this->matrix->nrows);
    // Clear all primes and uncover all rows
    primed.clear();
    fill_n(matrix->covered_rows, matrix->nrows, false);
    return 3;
}

/* Add the smallest uncovered value to every element of each covered row,
 * and subtract it from every element of each uncovered column. Return to
 * step 4 without altering any stars, primes or uncovered lines.
 */

short Munkres::step_6() {
    // find the smallest uncovered value
    double minval = matrix->min_uncovered_cost();
    for (uint index = 0; index < matrix->nrows; index++) {
        if (matrix->covered_rows[index]) {
            matrix->add_to_row(index, minval);
        }
    }
    for (uint index = 0; index < matrix->ncols; index++) {
        if (!matrix->covered_columns[index]) {
            matrix->add_to_column(index, -minval);
        }
    }
    return 4;
}

// Remap the indices in entries to 0..n, to eliminate empty rows and columns.
vector<entry> munkres(vector<entry> &entries) {
    vector<entry> optimal, remapped;
    map<uint, uint> i_rmap;
    map<uint, uint> j_rmap;
    map<uint, uint>::iterator it;

    vector<uint> i_map, j_map;

    uint mi, mj;
    for (uint index = 0; index < entries.size(); index++) {
        entry &e = entries[index];
        // remap i
        it = i_rmap.find(e.pos.i);
        if (it == i_rmap.end()) {
            mi = i_map.size();
            i_map.push_back(e.pos.i); // i_map[mi] == e.pos.i
            i_rmap[e.pos.i] = mi; // i_rmap[e.pos.i] = mi;
        } else {
            mi = it->second;
        }

        // remap j
        it = j_rmap.find(e.pos.j);
        if (it == j_rmap.end()) {
            mj = j_map.size();
            j_map.push_back(e.pos.j); // i_map[mi] == e.pos.i
            j_rmap[e.pos.j] = mj; // i_rmap[e.pos.i] = mi;
        } else {
            mj = it->second;
        }
        entry n = e;
        n.pos.i = mi; n.pos.j = mj;
        remapped.push_back(n);
    }

    Munkres m(remapped);
    optimal = m.munkres();
    for (uint index = 0; index<optimal.size(); index++) {
        entry &e = optimal[index];
        e.pos.i = i_map[e.pos.i];
        e.pos.j = j_map[e.pos.j];
    }
    return optimal;
}
