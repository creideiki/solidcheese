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
 * \file csg_object.cpp
 * Implementation of primitive objects.
 */

#include <cmath>
#include <iostream>
#include <sstream>
#include <assert.h>

#include <GL/glut.h>

#include "csg_object.h"
#include "debug.h"
#include "persistence.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

CSG_Object::CSG_Object(string name):
   _name(name), _precision(10),
   _red(1.0), _green(1.0), _blue(1.0)
{
}

CSG_Object::~CSG_Object()
{
}

void CSG_Object::set_precision(int precision)
{
   _precision = (precision < 3) ? 3 : precision;
}

int CSG_Object::get_precision()
{
   return _precision;
}

string CSG_Object::stringify()
{
   ostringstream representation;
   representation << type_name() << " " << escape(_name) << " " << _precision << " "
                  << _red << " " << _green << " " << _blue << " " << _trans.stringify();
   return representation.str();
}

string CSG_Object::type_name()
{
   return "Undefined";
}

bool CSG_Object::from_string(const std::vector<std::string> &parameters)
{
   if(parameters.size() != 24)
   {
      cout << "Error: Can't define an object with " << parameters.size() <<
              " values (wants 24)" << endl;

      for(unsigned int i = 0; i < parameters.size(); i++)
         DBG(cout << parameters[i] << endl);

      return false;
   }

   assert(parameters[0] == type_name());

   _name = unescape(parameters[1]);

   istringstream iss;

   iss.str(parameters[2]);
   if(!(iss >> _precision))
   {
      cout << "Error reading precision: '" << parameters[2] << "' is not an int" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[3]);
   if(!(iss >> _red))
   {
      cout << "Error reading red color component: '" << parameters[3] <<
              "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[4]);
   if(!(iss >> _green))
   {
      cout << "Error reading green color component: '" << parameters[4] <<
              "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[5]);
   if(!(iss >> _blue))
   {
      cout << "Error reading blue color component: '" << parameters[5] <<
              "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   if((parameters[6]  != "{") ||
      (parameters[23] != "}"))
   {
      cout << "Error: Can't find transformation matrix" << endl;
      return false;
   }

   vector<string>::const_iterator first = parameters.begin();
   vector<string>::const_iterator last = parameters.end();
   advance(first, 7);
   advance(last, -1);

   vector<string> transformation_data(first, last);

   if(!_trans.from_string(transformation_data))
   {
      cout << "Error reading transformation matrix" << endl;
      return false;
   }

   DBG(cout << "Loaded " << _name << endl);

   return true;
}

void CSG_Object::set_transform(const Matrix &m)
{
   _trans = m;
}

Matrix CSG_Object::get_transform() const
{
   return _trans;
}

void CSG_Object::set_name(string name)
{
   _name = name;
}

string CSG_Object::get_name() const
{
   return _name;
}

void CSG_Object::set_color(GLfloat red, GLfloat green, GLfloat blue)
{
   _red = red; _green = green; _blue = blue;
}

void CSG_Object::get_color(GLfloat &red, GLfloat &green, GLfloat &blue)
{
   red = _red; green = _green; blue = _blue;
}

void prepare_highlight(GLfloat red, GLfloat green, GLfloat blue)
{
   glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT |
                GL_CURRENT_BIT | GL_ENABLE_BIT);
   //glDisable(GL_LIGHTING);
   glDisable(GL_LIGHT0);
   GLfloat flat_light[]  = { 1, 1, 1, 1 };
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, flat_light);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_LEQUAL);
   glColor3f(red, green, blue);
}

string CSG_Object_Cube::type_name()
{
   return "Cube";
}

void CSG_Object_Cube::render()
{
   DBG(cout << "Rendering " << get_name() << "   ");
   glPushMatrix();
   glMultMatrixf(get_transform().data);
   glutSolidCube(1);
   glPopMatrix();
   DBG(cout << endl);
}

void CSG_Object_Cube::render_highlight(GLfloat red, GLfloat green, GLfloat blue)
{
   DBG(cout << "Rendering highlight for " << get_name() << "   ");
   prepare_highlight(red, green, blue);
   glPushMatrix();
   glMultMatrixf(get_transform().data);
   glutWireCube(1);
   glPopMatrix();
   glPopAttrib();
   DBG(cout << endl);  
}


CSG_Object_Cylinder::CSG_Object_Cylinder(string name) :
   CSG_Object(name),
   dirty(true),
   num_vertices(0), num_indices(0),
   vertices(NULL), indices(NULL)
{
}

CSG_Object_Cylinder::~CSG_Object_Cylinder()
{
   delete[] vertices;
   delete[] indices;
}

string CSG_Object_Cylinder::type_name()
{
   return "Cylinder";
}

void CSG_Object_Cylinder::set_precision(int precision)
{
   CSG_Object::set_precision(precision);
   dirty = true;
}

void CSG_Object_Cylinder::rebuild_vertices()
{
   const int num_faces = get_precision();
   
   DBG(cout << "Recalculating cylinder vertices" << endl);

   delete[] vertices;
   delete[] indices;
   
   num_vertices = num_faces * 2;
   num_indices = num_faces * 4 + 2;
   
   vertices = new GLfloat[num_vertices * 3];
   indices = new GLuint[num_indices];
   
   //Do main hull
   //DBG(cout << "Calculating vertices of main hull of cylinder" << endl);
   GLuint vertex_num = 0;
   for(int i = 0; i < num_faces; ++i)
   {
      GLfloat angle = 360.0 - 360.0 * i / num_faces;
      
      GLfloat sine = 0.5 * cos((M_PI / 180.0) * angle);
      GLfloat cosine = 0.5 * sin((M_PI / 180.0) * angle);
      
      vertices[vertex_num + 0] = sine;
      vertices[vertex_num + 1] = 0.5;
      vertices[vertex_num + 2] = cosine;
      vertex_num += 3;
      vertices[vertex_num + 0] = sine;
      vertices[vertex_num + 1] = -0.5;
      vertices[vertex_num + 2] = cosine;
      vertex_num += 3;
      
   }
   
   //for(int i = 0; i < num_vertices; i++)
   //   DBG(cout << "Vertex: " << i << ": (" <<
   //       vertices[i * 3 + 0] << ", " <<
   //       vertices[i * 3 + 1] << ", " <<
   //       vertices[i * 3 + 2] << ")" << endl);
   
   //DBG(cout << "Calculating indices of main hull of cylinder" << endl);
   for(int index_num = 0; index_num < num_indices - 2 * num_faces; index_num++)
      indices[index_num] = index_num % num_vertices;
   
   //Do end bits
   //DBG(cout << "Calculating indices of end bits of cylinder" << endl);
   int index_num = num_indices - 2 * num_faces;
   for(int corner = 0; corner < num_faces * 2; corner += 2)
      indices[index_num++] = corner;
   for(int corner = num_faces * 2 - 1; corner > 0; corner -= 2)
      indices[index_num++] = corner;
   
   //for(int i = 0; i < num_indices; i++)
   //   DBG(cout << "Index " << i << ": vertex " << (int)indices[i] << endl);
   
   dirty = false;
   
   DBG(cout << "Done recalculating vertices" << endl);
}

void CSG_Object_Cylinder::render()
{
   if(dirty) rebuild_vertices();

   DBG(cout << "Rendering " << get_name() << "   ");

   const int num_faces = get_precision();

   glPushAttrib(GL_ENABLE_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   glEnable(GL_NORMALIZE);
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);

   glPushMatrix();
   glMultMatrixf(get_transform().data);

   //Draw main hull
   //DBG(cout << "Drawing main hull of cylinder" << endl);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glNormalPointer(GL_FLOAT, 0, vertices);
   glDrawElements(GL_QUAD_STRIP, num_indices - 2 * num_faces, GL_UNSIGNED_INT, indices);

   //Draw end bits
   //DBG(cout << "Drawing end bits of cylinder" << endl);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glNormalPointer(GL_FLOAT, 0, vertices);
   glDrawElements(GL_POLYGON, num_faces, GL_UNSIGNED_INT, indices + num_indices - 2 * num_faces);
   glDrawElements(GL_POLYGON, num_faces, GL_UNSIGNED_INT, indices + num_indices - num_faces);

   glPopMatrix();

   glPopClientAttrib();
   glPopAttrib();

   DBG(cout << endl);
}

void CSG_Object_Cylinder::render_highlight(GLfloat red, GLfloat green, GLfloat blue)
{
   if(dirty) rebuild_vertices();

   DBG(cout << "Rendering highlight for " << get_name() << "   ");

   const int num_faces = get_precision();

   glPushAttrib(GL_ENABLE_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
   prepare_highlight(red, green, blue);

   glEnable(GL_NORMALIZE);
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   
   glPushMatrix();
   glMultMatrixf(get_transform().data);

   //Draw main hull
   //DBG(cout << "Drawing main hull of cylinder" << endl);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glNormalPointer(GL_FLOAT, 0, vertices);
   glDrawElements(GL_LINES, num_indices - 2 * num_faces, GL_UNSIGNED_INT, indices);
   
   //Draw end bits
   //DBG(cout << "Drawing end bits of cylinder" << endl);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glNormalPointer(GL_FLOAT, 0, vertices);
   glDrawElements(GL_LINE_LOOP, num_faces, GL_UNSIGNED_INT, indices + num_indices - 2 * num_faces);
   glDrawElements(GL_LINE_LOOP, num_faces, GL_UNSIGNED_INT, indices + num_indices - num_faces);
   
   glPopMatrix();

   glPopAttrib(); // from prepare_highlight
   glPopClientAttrib();
   glPopAttrib();

   DBG(cout << endl);
}

string CSG_Object_Sphere::type_name()
{
   return "Sphere";
}

void CSG_Object_Sphere::render()
{
   DBG(cout << "Rendering " << get_name() << "   ");
   glPushMatrix();
   glMultMatrixf(get_transform().data);
   glutSolidSphere(0.5, get_precision(), get_precision());
   glPopMatrix();
   DBG(cout << endl);
}

void CSG_Object_Sphere::render_highlight(GLfloat red, GLfloat green, GLfloat blue)
{
   DBG(cout << "Rendering highlight for " << get_name() << "   ");
   prepare_highlight(red, green, blue);
   glPushMatrix();
   glMultMatrixf(get_transform().data);
   glutWireSphere(0.5, get_precision(), get_precision());
   glPopMatrix();
   glPopAttrib();
   DBG(cout << endl);
}


