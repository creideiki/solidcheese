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
 * \file csg_object.h
 * Various classes representing primitive CSG objects.
 */

#ifndef __CSG_OBJECT_H__
#define __CSG_OBJECT_H__

#include <string>
#include <GL/gl.h>
#include "matrix.h"

/*!
 * Abstract base class representing a primitive object.
 */
class CSG_Object
{
public:

   CSG_Object(std::string name = "Untitled");
   virtual ~CSG_Object();

   /*!
    * Renders this primitive, using the transformation chosen with set_transform().
    * Does NOT draw the highlight anymore.
    * Only draw vertices in here. Do not change any OpenGL state.
    */
   virtual void render() = 0;

   /*!
    * Renders this primitive as a wireframe, using the transformation chosen with
    * set_transform().
    * Only draw vertices in here. Do not change any OpenGL state.
    */
   virtual void render_highlight(GLfloat red, GLfloat green, GLfloat blue) = 0;

   /*!
    * The precision used to draw this object. Higher values give more
    * subdivisions on cylinders and spheres. This setting lacks meaning for
    * cubes. Values lower than 3 are clamped to 3.
    */
   void set_precision(int precision);
   int get_precision();

   /*!
    * Output a representation of this object as a string.
    */
   virtual std::string stringify();

   /*!
    * Returns the type of the primitive as a string.
    */
   virtual std::string type_name();

   /*!
    * Sets parameters from a tokenized string.
    *
    * \return true if successful, false if there were any errors.
    *         (in which case the state of the object is undefined).
    */
   bool from_string(const std::vector<std::string> &parameters);

   /*!
    * The transformation that is applied before render() is called.
    */
   Matrix get_transform() const;
   void set_transform(const Matrix &m);

   //! Name of the object. Only used for debugging.
   void set_name(std::string name);
   std::string get_name() const;

   void set_color(GLfloat red, GLfloat green, GLfloat blue);
   void get_color(GLfloat &red, GLfloat &green, GLfloat &blue);

private:
   std::string _name;
   Matrix _trans;
   int _precision;
   GLfloat _red, _green, _blue;
};

class CSG_Object_Cube : public CSG_Object
{
public:
   std::string type_name();
   void render();
   void render_highlight(GLfloat red, GLfloat green, GLfloat blue);
};

class CSG_Object_Cylinder : public CSG_Object
{
public:
   CSG_Object_Cylinder(std::string name = "Untitled");
   ~CSG_Object_Cylinder();

   std::string type_name();
   void set_precision(int precision);
   void render();
   void render_highlight(GLfloat red, GLfloat green, GLfloat blue);


private:
   bool dirty;  //!< True if we need to recalculate vertices at next call to render().
   void rebuild_vertices();

   int num_vertices;  //!< Number of stored vertices (precision * 2).
   int num_indices;   //!< Number of storde indices (precision * 4 + 2).

   GLfloat *vertices; //!< Stored vertices, ready to be sent to OpenGL.
   GLuint *indices;   //!< Stored indices, ready to be sent to OpenGL.
};

class CSG_Object_Sphere : public CSG_Object
{
public:
   std::string type_name();
   void render();
   void render_highlight(GLfloat red, GLfloat green, GLfloat blue);
};

#endif
