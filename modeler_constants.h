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
 * \file modeler_constants.h
 * Constants for the modeler.
 */

#ifndef __MODELER_CONSTANTS_H__
#define __MODELER_CONSTANTS_H__

#include <GL/gl.h>

const char SAVE_KEY                   = 'x';
const char LOAD_KEY                   = 'l';
const char INTERSECTION_KEY           = 'i';
const char SUBTRACTION_KEY            = 's';
const char UNION_KEY                  = 'u';
const char CUBE_KEY                   = 'k';
const char CYLINDER_KEY               = 'c';
const char SPHERE_KEY                 = 'p';
const char ZOOM_KEY                   = 'e';
const char UNZOOM_KEY                 = 'r';
const char DELETE_OBJECT_KEY          = 'z';
const char CENTER_KEY                 = ' ';
const char TOGGLE_NEGATIVE_VISIBILITY = 'n';
const char TOGGLE_CAMERA_OR_OBJECT    = 'h';

const unsigned int CHECKMOUSE_INTERVAL = 50;
const GLfloat TURNSPEED                = 0.001314;
const GLfloat SPEED                    = 0.5;
const GLfloat DEFAULT_RADIUS           = 4;

extern int x_resolution;
extern int y_resolution;

extern GLfloat clip_hither;
extern GLfloat clip_yon;

#endif
