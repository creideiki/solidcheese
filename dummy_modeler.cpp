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
 * \file dummy_modeler.cpp
 * A simple dummy modeler, used only to test the renderer. Gives out a static,
 * normalized, CSG tree.
 */

#include <GL/glut.h>
#include <iostream>
#include <cstdlib>

#include "csg_tree.h"
#include "csg_object.h"
#include "renderer_interface.h"
#include "debug.h"
#include "matrix.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

CSG_Node *i_node;
CSG_Node *d_node;
CSG_Node *u_node;

GLfloat angle = 0;

int render_partial = -1;

/*!
 * GLUT needs a display callback to run happily.
 */
void display()
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   gluLookAt( 1.5, 1.5, 1.5,
                0,   0,   0,
                0,   1,   0);

   glRotatef(angle, 0, 1, 0);

   DBG(cout << "Doing prerender()" << endl);
   prerender(i_node, 0, 0);
   DBG(cout << "Doing render()" << endl << endl);
   render(i_node);
   glutSwapBuffers();
   DBG(cout << "Done rendering. Hope it looks nice." << endl);
   DBG(cout << endl);
}

/*!
 * To do cleanup after glutMainLoop() exits, you have to register an atexit()
 * function, since glutMainLoop() doesn't return. Insert flameage here.
 */
void cleanup()
{
   DBG(cout << "cleanup() called, program is dying" << endl);

   DBG(cout << "   Destructing the tree recursively from the root" << endl);
   delete u_node;
   DBG(cout << endl);
   DBG(cout << "All done, bye bye" << endl);
}

void keypress(unsigned char key, int mousex, int mousey)
{
   switch (key)
   {
   case 27: // Escape
      exit(0);
      break; 
   case '1':
      set_render_type(RENDER_CSG);
      glutPostRedisplay();
      break;
   case '2':
      set_render_type(RENDER_CSG_Z);
      glutPostRedisplay();
      break;
   case '3':
      set_render_type(RENDER_NO_CSG);
      glutPostRedisplay();
      break;
   case '4':
      set_render_type(RENDER_NO_CSG_Z);
      glutPostRedisplay();
      break;
   case '5':
      set_render_type(RENDER_CSG_SPLIT_SCREEN);
      glutPostRedisplay();
      break;
   case '/':
      render_partial = 0;
      set_render_partial(render_partial);
      glutPostRedisplay();
      break;
   case '*':
      render_partial = -1;
      set_render_whole();
      glutPostRedisplay();
      break;
   case '+':
      if (render_partial != -1)
      {
         ++render_partial;
         set_render_partial(render_partial);
      }
      glutPostRedisplay();
      break;
   case '-':
      if (render_partial > 0)
      {
         --render_partial;
         set_render_partial(render_partial);
      }
      glutPostRedisplay();
      break;
   case ' ':
      angle += 3;
      glutPostRedisplay();
      break;
   }
}

void reshape(GLsizei w, GLsizei h)
{
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(90, (GLfloat)w/h, 1, 10);
   glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
   atexit(cleanup);

   //Some OpenGL setup code, stolen from the labs.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(200, 200);
   glutCreateWindow("Solid Cheese - Dummy modeler");

   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keypress);

   glClearColor(0.0, 0.0, 0.0, 1.0);
   glShadeModel(GL_SMOOTH);
   //glShadeModel(GL_FLAT);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);

   glCullFace(GL_BACK);

   GLfloat light_position[] = { 1, 0, 3, 1 };
   GLfloat zero_light[]  = { 0, 0, 0, 1 };
   GLfloat ambient_light[]  = { 0.2, 0.2, 0.2, 1 };
   GLfloat diffuse_light[]  = { 0.8, 0.8, 0.8, 1 };

   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
   glLightfv(GL_LIGHT0, GL_AMBIENT, zero_light);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

   GLfloat mat_specular[] = { 1, 1, 1, 1 };

   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
   
   //glEnable(GL_LINE_SMOOTH);
   glLineWidth(2);
   //glEnable(GL_POLYGON_OFFSET_LINE);
   //glPolygonOffset(10, 10);

   /*We'll create the following normalized tree and send it to the renderer:
    *
    *                       union
    *                        / \
    *              difference   u_cube
    *                 / \
    *     intersection   sphere
    *         / \
    * cylinder   i_cube
    */

   //Create primitives
   DBG(cout << "Creating primitives" << endl);
   Matrix m;

   DBG(cout << "   Creating cylinder primitive" << endl);
   CSG_Object_Cylinder cylinder;
   cylinder.set_name("cylinder");

   DBG(cout << "   Translating cylinder to (0.5, 0, 0)" << endl);
   m = cylinder.get_transform();
   m *= translate(0.5, 0, 0);
   DBG(cout << "   Rotating cylinder -pi/8 around the (-1, 1, -1) axis" << endl);
   m *= rotate(-M_PI / (float)8,
               -1, 1, -1);
   cylinder.set_transform(m);
   cylinder.set_precision(7);
   cylinder.set_color(0, 1, 0);

   DBG(cout << "   Creating intersection cube primitive" << endl);
   CSG_Object_Cube i_cube;
   i_cube.set_name("intersection cube");

   DBG(cout << "   Translating intersection cube to (0, 0, 0.5)" << endl);
   m = i_cube.get_transform();
   m *= translate(0, 0, 0.5);
   //m *= rotate(delta, 0.75, 0.5, 0.25);
   i_cube.set_transform(m);
   i_cube.set_color(1, 0, 0);

   DBG(cout << "   Creating sphere primitive" << endl);
   CSG_Object_Sphere sphere;
   sphere.set_name("sphere");

   DBG(cout << "   Translating sphere to (0, 0, -0.5)" << endl);
   m = sphere.get_transform();
   m *= translate(0, 0, -0.5);
   sphere.set_transform(m);

   DBG(cout << "   Creating union cube primitive" << endl);
   CSG_Object_Cube u_cube;
   u_cube.set_name("union cube");

   DBG(cout << "   Translating union cube to (-0.5, 0, 0)" << endl);
   m = u_cube.get_transform();
   m *= translate(-0.5, 0, 0);
   u_cube.set_transform(m);
   DBG(cout << endl);

   //Create object nodes
   DBG(cout << "Creating object nodes" << endl);
   DBG(cout << "   Creating cylinder node" << endl);
   CSG_Node *cylinder_node = new CSG_Node(&cylinder);
   DBG(cout << "   Creating intersection cube node" << endl);
   CSG_Node *i_cube_node = new CSG_Node(&i_cube);
   DBG(cout << "   Creating sphere node" << endl);
   CSG_Node *sphere_node = new CSG_Node(&sphere);
   DBG(cout << "   Creating union cube node" << endl);
   CSG_Node *u_cube_node = new CSG_Node(&u_cube);
   DBG(cout << endl);

   //Create operation nodes and build tree
   DBG(cout << "Creating operation nodes and building tree" << endl);
   DBG(cout << "   Creating and inserting intersection node" << endl);
   i_node = CSG_Node::create_and_insert(CSG_Node::DIFFERENCE, // Jaja...
                                        cylinder_node, i_cube_node);
   DBG(cout << "   Creating and inserting difference node" << endl);
   d_node = CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                        i_node, sphere_node);
   DBG(cout << "   Creating and inserting union node" << endl);
   u_node = CSG_Node::create_and_insert(CSG_Node::UNION,
                                        d_node, u_cube_node);
   DBG(cout << endl);

   glutMainLoop();

   //Just to make GCC happy. We'll never get here, since glutMainLoop()
   //doesn't return.
   return 0;
}
