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
 * \file dummy_renderer.cpp
 * Dummy version of the renderer, to test the modeler. Just draws all primitives.
 */

#include <GL/glut.h>
#include "renderer_interface.h"

using namespace std;

void traverse(const CSG_Node *tree);

const CSG_Node *prerender(const CSG_Node *tree, int mouse_x, int mouse_y,
                          bool select_invisible)
{
   return NULL;
}

void render(const CSG_Node *tree)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   traverse(tree);
}

void set_render_type(RenderType type)
{
}

void set_render_partial(int part)
{
}

void set_render_whole()
{
}

void traverse(const CSG_Node *tree)
{
   if(tree->get_type() == CSG_Node::PRIMITIVE)
   {
      GLfloat r, g, b;
      tree->get_object()->get_color(r, g, b);
      glColor3f(r, g, b);
      tree->get_object()->render();
   }
   else
   {
      traverse(tree->get_left());
      traverse(tree->get_right());
   }
}
