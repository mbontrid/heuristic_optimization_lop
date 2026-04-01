/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _LO_H_
#define _LO_H_

#include "instance.h"
extern long int **CostMat;

long long int computeCost(long int *lo);
void createRandomSolution(long int *s);

int pivot_first(long int **matrix);
int pivot_best(long int **matrix);

int neighborhood_tranpose(int a, int b, long int **matrix);
int neighborhood_exchange(int a, int b, long int **matrix);
int neighborhood_insert(int a, int b, long int **matrix);

int sol_start_random(int, int);
int sol_start_c_and_w(int, int);

#endif
