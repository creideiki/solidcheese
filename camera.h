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
 * \file camera.h
 * Interface to the camera.
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <vector>
#include <string>
#include <GL/gl.h>
#include "matrix.h"
#include "modeler_constants.h"

/*!
 * Holds all information about the camera.
 */
class Camera
{
public:
   Camera() : center_x(0), center_y(0), center_z(0), radius(DEFAULT_RADIUS)
   {
   }

   /*!
    * Exports a string representation.
    */
   std::string stringify() const;

   /*!
    * Sets parameters from a tokenized string.
    *
    * \return true if successful, false if there were any errors
    *         (in which case the state of the object is undefined).
    */
   bool from_string(const std::vector<std::string> &parameters);

   /*!
    * The coordinates of the point the camera is looking at currently.
    */
   GLfloat center_x, center_y, center_z;
   GLfloat radius;         //!< The distance from the look-at point to the camera.
   Matrix rotation_matrix; //!< The rotation of the camera around the look-at point.
};

#endif
