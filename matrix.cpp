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

/*!
 * \file matrix.cpp
 * Implementation of 4x4 GLfloat matrix class.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <cmath>
#include "matrix.h"
#include "debug.h"

using namespace std;

const GLfloat identity_matrix[16] = {1, 0, 0, 0,
                                     0, 1, 0, 0,
                                     0, 0, 1, 0,
                                     0, 0, 0, 1};

const GLfloat zero_vector[4] = {0, 0, 0, 0};

string Matrix::stringify() const
{
   ostringstream representation;

   representation << "{ ";
   for(int i = 0; i < 16; i++)
      representation << data[i] << " ";
   representation << "}";

   return representation.str();
}

bool Matrix::from_string(const vector<string> &parameters)
{
   if(parameters.size() != 16)
   {
      cout << "Trying to set the contents of a 4x4 matrix with " <<
              parameters.size() << " values (wants 16)" << endl;

      for(unsigned int i = 0; i < parameters.size(); i++)
         DBG(cout << parameters[i] << endl);

      return false;
   }

   istringstream iss;

   for(int i = 0; i < 16; ++i)
   {
      iss.str(parameters[i]);
      if(!(iss >> data[i]))
      {
         cout << "Error reading matrix data: '" << parameters[i] << "' is not a GLfloat" << endl;
         return false;
      }
      iss.clear();
   }

   return true;
}

Matrix translate(GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat m[16] = { 1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     x, y, z, 1 };//T
   return Matrix(m);
}

Matrix rotate_x(GLfloat angle)
{
   GLfloat sin = std::sin(angle);
   GLfloat cos = std::cos(angle);
   GLfloat m[16] = { 1,    0,    0, 0,
                     0,  cos,  sin, 0,
                     0, -sin,  cos, 0,
                     0,    0,    0, 1 };//T
   return Matrix(m);
}

Matrix rotate_y(GLfloat angle)
{
   GLfloat sin = std::sin(angle);
   GLfloat cos = std::cos(angle);
   GLfloat m[16] = { cos, 0, -sin, 0,
                       0, 1,    0, 0,
                     sin, 0,  cos, 0,
                       0, 0,    0, 1 };//T
   return Matrix(m);
}

Matrix rotate_z(GLfloat angle)
{
   GLfloat sin = std::sin(angle);
   GLfloat cos = std::cos(angle);
   GLfloat m[16] = {  cos, sin, 0, 0,
                     -sin, cos, 0, 0,
                        0,   0, 1, 0,
                        0,   0, 0, 1 };//T
   return Matrix(m);
}

Matrix scale(GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat m[16] = {x, 0, 0, 0,
                    0, y, 0, 0,
                    0, 0, z, 0,
                    0, 0, 0, 1 };//T
   return Matrix(m);
}

Matrix rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat length = sqrt(x * x + y * y + z * z);

   GLfloat ux = x / length;
   GLfloat uy = y / length;
   GLfloat uz = z / length;

   GLfloat d = sqrt(uy * uy + uz * uz);

   GLfloat rx_data[16] = {1,       0,      0, 0,
                          0,  uz / d, uy / d, 0,
                          0, -uy / d, uz / d, 0,
                          0,       0,      0, 1}; //T
   Matrix rx(rx_data);

   GLfloat ry_data[16] = {  d, 0, ux, 0,
                            0, 1,  0, 0,
                          -ux, 0,  d, 0,
                            0, 0,  0, 1}; //T
   Matrix ry(ry_data);

   Matrix rz = rotate_z(angle);

   return rx.transpose() * ry.transpose() * rz * ry * rx;
}
