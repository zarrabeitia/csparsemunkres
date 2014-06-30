#include "matrix.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::sort
#include "assert.h"

bool sort_by_i(entry e1, entry e2) {
    return e1.pos.i < e2.pos.i;
}

bool sort_by_j(entry e1, entry e2) {
    return e1.pos.j < e2.pos.j;
}

Matrix::Matrix(vector<entry> entries)
{
    this->entries = entries;
    std::sort(this->entries.begin(), this->entries.end(), sort_by_i);
    // the matrix has the same height as the original, but the columns are extended
    // by the number of rows. This is necessary for the sparse algorithm.
    this->nrows = std::max_element(entries.begin(), entries.end(), sort_by_i)->pos.i+1;
    this->real_columns = std::max_element(entries.begin(), entries.end(), sort_by_j)->pos.j+1;
    this->ncols = this->real_columns + this->nrows;
    // Ensure there is a feasible but very undesireable solution
    // covering the rows
    for (uint i = 0; i < nrows; i++) {
        entry e;
        e.pos.i = i; e.pos.j = real_columns+i; e.cost = BIGVALUE;
        this->entries.push_back(e);
    }
    this->row_adds = new double[nrows];
    std::fill_n(row_adds,nrows,0);

    this->column_adds = new double[ncols];
    std::fill_n(column_adds,ncols,0);

    this->covered_rows = new bool[nrows];
    std::fill_n(covered_rows,nrows,false);

    this->covered_columns = new bool[ncols];
    std::fill_n(covered_columns,ncols,false);
}

Matrix::~Matrix() {
    delete[] row_adds;
    delete[] column_adds;
    delete[] covered_rows;
    delete[] covered_columns;
}

vector<entry> Matrix::row(uint rowindex)
{
    vector<entry> res;
    for (uint index=0; index<entries.size(); index++) {
        entry e = current_value(index);
        if (e.pos.i == rowindex) {
            res.push_back(e);
        }
    }
    return res;
}

vector<entry> Matrix::get_values()
{
    vector<entry> res;
    for (uint index=0; index<entries.size(); index++) {
        entry e = current_value(index);
        res.push_back(e);
    }
    return res;
}

void Matrix::add_to_column(uint colindex, double value) {
    this->column_adds[colindex] += value;
}

void Matrix::add_to_row(uint rowindex, double value) {
    this->row_adds[rowindex] += value;
}

vector<uint> Matrix::zeros() {
    vector<uint> res;
    for (uint index=0; index<entries.size(); index++) {
        entry e = current_value(index);
        if (e.cost < EPS && e.cost > -EPS) {
            res.push_back(index);
        }
    }
    return res;
}

uint Matrix::find_uncovered_zero() {
    //res.pos.i = -1; res.pos.j = -1; res.cost = -1;
    for (uint index=0; index<entries.size(); index++) {
        entry e = current_value(index);
        if (e.cost < EPS && e.cost > -EPS && !covered_rows[e.pos.i] && !covered_columns[e.pos.j]) {
            return index;
        }
    }
    return NOTFOUND;
}


//entry Matrix::current_value(entry &e) {
//    entry res = e;
//    res.cost += row_adds[e.pos.i] + column_adds[e.pos.j];
//    return res;
//}

entry Matrix::current_value(uint index) {
    entry res = entries[index];
    res.cost += row_adds[res.pos.i] + column_adds[res.pos.j];
    return res;
}

uint Matrix::nentries() {
    return entries.size();
}

double Matrix::min_uncovered_cost() {
    double minval = INF;
    for (uint index = 0; index < entries.size(); index++) {
        entry e = current_value(index);
        bool covered = covered_rows[e.pos.i] || covered_columns[e.pos.j];
        if (!covered && minval > e.cost) {
            minval = e.cost;
        }
    }
    assert(EPS < minval && minval < INF);
    return minval;
}
