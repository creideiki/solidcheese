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
 * \file matrix.h
 * A 4x4 GLfloat matrix and a 4-dimensional vector.
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <string>
#include <GL/gl.h>

extern const GLfloat identity_matrix[16];
extern const GLfloat zero_vector[4];

/*!
 * A 4x4 GLfloat matrix class.
 */
class Matrix
{
public:
   explicit inline Matrix(const GLfloat *data = identity_matrix);

   inline Matrix &operator=(const Matrix &m);
   inline Matrix &operator=(const GLfloat *data);

   inline Matrix &operator+=(const Matrix &m);
   inline Matrix &operator*=(const Matrix &m);

   inline bool operator==(const Matrix &m);
   inline bool operator!=(const Matrix &m);

   inline Matrix transpose();

   /*!
    * Exports a string representation.
    */
   std::string stringify() const;

   /*!
    * Sets parameters from a tokenized string.
    *
    * \return true if successful, false if there were any errors.
    *         (in which case the state of the object is undefined).
    */
   bool from_string(const std::vector<std::string> &parameters);

   GLfloat data[16];

private:
   friend inline Matrix operator*(const Matrix &a, const Matrix &b);
   friend inline Matrix operator+(const Matrix &a, const Matrix &b);
};

Matrix translate(GLfloat x, GLfloat y, GLfloat z);
Matrix rotate_x(GLfloat angle); //!< Radians!
Matrix rotate_y(GLfloat angle); //!< Radians!
Matrix rotate_z(GLfloat angle); //!< Radians!
Matrix scale(GLfloat x, GLfloat y, GLfloat z);

/*!
 * Rotation around arbitrary axis, like glRotate.
 *
 * \param angle Note that the angle is specified in radians.
 */
Matrix rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

// Implementation

Matrix::Matrix(const GLfloat *d)
{
   for(int i = 0; i < 16; ++i)
      data[i] = d[i];
}

inline Matrix &Matrix::operator=(const Matrix &m)
{
   for(int i = 0; i < 16; ++i)
      data[i] = m.data[i];
   return *this;
}

inline Matrix &Matrix::operator=(const GLfloat *d)
{
   for(int i = 0; i < 16; ++i)
      data[i] = d[i];
   return *this;
}

Matrix &Matrix::operator+=(const Matrix &m)
{
   for(int i = 0; i < 16; ++i)
      data[i] += m.data[i];
   return *this;
}

Matrix &Matrix::operator*=(const Matrix &m)
{
   Matrix m2 = *this * m;
   for(int i = 0; i < 16; ++i)
      data[i] = m2.data[i];
   return *this;
}

inline bool Matrix::operator==(const Matrix &m)
{
   for(int i = 0; i < 16; ++i)
      if (data[i] != m.data[i]) return false;
   return true;
}

inline bool Matrix::operator!=(const Matrix &m)
{
   return !(*this == m);
}

inline Matrix Matrix::transpose()
{
   GLfloat m[16];
   for(int c = 0; c < 4; ++c)
   {
      for(int r = 0; r < 4; ++r)
      {
         m[c*4+r] = data[r*4+c];
      }
   }
   return Matrix(m);
}

inline Matrix operator*(const Matrix &a, const Matrix &b)
{
   GLfloat m[16];
   for(int c4 = 0; c4 < 16; c4 += 4)
   {
      for(int r = 0; r < 4; ++r)
      {
         m[c4+r] = a.data[r+ 0] * b.data[0+c4] + 
                   a.data[r+ 4] * b.data[1+c4] +
                   a.data[r+ 8] * b.data[2+c4] +
                   a.data[r+12] * b.data[3+c4];
      }
   }
   return Matrix(m);
}

inline Matrix operator+(const Matrix &a, const Matrix &b)
{
   Matrix m(a);
   m += b;
   return m;
}

/*!
 * A 4-dimensional vector class.
 */
class Vector
{
public:
   explicit inline Vector(const GLfloat *data = zero_vector);
   explicit inline Vector(GLfloat x, GLfloat y, GLfloat z, GLfloat h);

   inline Vector &operator=(const Vector &m);
   inline Vector &operator=(const GLfloat *data);

   inline Vector &operator+=(const Vector &m);
   inline Vector &operator-=(const Vector &m);
   inline Vector &operator*=(GLfloat s);

   inline bool operator==(const Vector &m);
   inline bool operator!=(const Vector &m);

   GLfloat data[4];

private:
   friend inline GLfloat dot_product(const Vector &a, const Vector &b);

   /*!
    * \return The cross product of a and b with h = 0.
    */
   friend inline Vector cross_product(const Vector &a, const Vector &b);

   friend inline Vector operator+(const Vector &a, const Vector &b);
   friend inline Vector operator-(const Vector &a, const Vector &b);
   friend inline Vector operator*(GLfloat s, const Vector &v);
   friend inline Vector operator*(const Vector &v, GLfloat s);

   friend inline Vector operator*(const Matrix &m, const Vector &v);
};

// Implementation

Vector::Vector(const GLfloat *d)
{
   for(int i = 0; i < 4; ++i)
      data[i] = d[i];
}

Vector::Vector(GLfloat x, GLfloat y, GLfloat z, GLfloat h)
{
   data[0] = x;
   data[1] = y;
   data[2] = z;
   data[3] = h;
}

inline Vector &Vector::operator=(const Vector &v)
{
   for(int i = 0; i < 4; ++i)
      data[i] = v.data[i];
   return *this;
}

inline Vector &Vector::operator=(const GLfloat *d)
{
   for(int i = 0; i < 4; ++i)
      data[i] = d[i];
   return *this;
}

Vector &Vector::operator+=(const Vector &v)
{
   for(int i = 0; i < 4; ++i)
      data[i] += v.data[i];
   return *this;
}

Vector &Vector::operator-=(const Vector &v)
{
   for(int i = 0; i < 4; ++i)
      data[i] -= v.data[i];
   return *this;
}

Vector &Vector::operator*=(GLfloat s)
{
   for(int i = 0; i < 4; ++i)
      data[i] *= s;
   return *this;
}

inline bool Vector::operator==(const Vector &v)
{
   for(int i = 0; i < 4; ++i)
      if (data[i] != v.data[i]) return false;
   return true;
}

inline bool Vector::operator!=(const Vector &v)
{
   return !(*this == v);
}

inline GLfloat dot_product(const Vector &a, const Vector &b)
{
   return a.data[0] * b.data[0] +
          a.data[1] * b.data[1] +
          a.data[2] * b.data[2];
}

inline Vector cross_product(const Vector &a, const Vector &b)
{
   Vector v;
   v.data[0] = a.data[1] * b.data[2] - a.data[2] * b.data[1];
   v.data[1] = a.data[2] * b.data[0] - a.data[0] * b.data[2];
   v.data[2] = a.data[0] * b.data[1] - a.data[1] * b.data[0];
   v.data[3] = 0;
   return v;
}

inline Vector operator+(const Vector &a, const Vector &b)
{
   Vector v(a);
   v += b;
   return v;
}

inline Vector operator-(const Vector &a, const Vector &b)
{
   Vector v(a);
   v -= b;
   return v;
}

inline Vector operator*(const Vector &v, GLfloat s)
{
   Vector a(v);
   a *= s;
   return a;
}

inline Vector operator*(GLfloat s, const Vector &v)
{
   Vector a(v);
   a *= s;
   return a;
}

inline Vector operator*(const Matrix &m, const Vector &v)
{
   GLfloat n[4] = {0, 0, 0, 0};

   for(int c = 0; c < 4; c++)
   {
      for(int r = 0; r < 4; r++)
      {
         n[c] += v.data[r] * m.data[c + 4 * r];
      }
   }
   return Vector(n);
}

#endif
