//
//   Solid Cheese - a simple CSG-SCS modeler and renderer
//
//   Copyright (C) 2003
//   Simon Elén, Marcus Eriksson, Karl-Johan Karlsson, Nils Öster
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

//
// $Id$
//

#include <cassert>
#include "matrix.h"

using namespace std;

int main()
{
   Matrix m;
   Matrix n;
   assert(m * n == m);

   m += n;

   assert(m != n);
   assert(m + n == n + n + n);

   GLfloat foo[16] = {1, 2, 3, 4,
                      5, 6, 7, 8,
                      9, 0, 1, 2,
                      3, 4, 5, 6};//T

   GLfloat footrans[16] = {1, 5, 9, 3,
                           2, 6, 0, 4,
                           3, 7, 1, 5,
                           4, 8, 2, 6};//T

   n = foo;

   GLfloat matlab_says[16] = {  50,  30,  40,  50,
                               122,  78, 104, 130,
                                24,  26,  38,  50,
                                86,  54,  72,  90};//T

   assert(n * Matrix(foo) == Matrix(matlab_says));

   assert(n.transpose() == Matrix(footrans));

   n = n.transpose();

   GLfloat matlab_says2[16] = { 116, 44,  62,  80,
                                 44, 56,  68,  80,
                                 62, 68,  84, 100,
                                 80, 80, 100, 120 };//T

   assert(Matrix(foo) * n == Matrix(matlab_says2));
   Matrix bar(foo);
   bar *= n;
   assert(bar == Matrix(matlab_says2));

   GLfloat unit[4] = {1, 0, 1, 0};
   Vector v(unit);
   Matrix identity;
   assert(identity * v == v);

   GLfloat rotation[16] = {-1,  0,  0, 0,
                            0,  1,  0, 0,
                            0,  0, -1, 0,
                            0,  0,  0, 1};
   assert(Matrix(rotation) * v == Vector(-1, 0, -1, 0));

   GLfloat origin[4] = {0, 0, 0, 1};
   GLfloat translation[16] = {1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              1, 2, 3, 1};
   assert(Matrix(translation) * Vector(origin) != Vector(0, 0, 0, 1));
   assert(Matrix(translation) * Vector(origin) == Vector(1, 2, 3, 1));

   return 0;
}
