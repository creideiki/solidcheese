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
 * \file modeler.cpp
 * A simple modeler.
 */

#include <list>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>

#include <GL/glut.h>

#include "csg_tree.h"
#include "csg_object.h"
#include "renderer_interface.h"
#include "debug.h"
#include "matrix.h"
#include "modeler_constants.h"
#include "camera.h"
#include "persistence.h"
#include "normalize.h"

using namespace std;
using std::list;

int render_partial = -1;

/*!
 * Holds the state of the mouse (pointer coords and button states).
 */
class Mouse
{
public:
   Mouse() : x(-1), y(-1), last_node(NULL), selected_node(NULL), current_node(NULL)
   {
      for(int i = 0; i < 4; i++)
         buttons[i] = GLUT_UP;
   }
   
   int x, y;
   bool buttons[4];   
  CSG_Node *last_node;     //!< The last selected node
  CSG_Node *selected_node; //!< The selected node
  CSG_Node *current_node; //!< The node that the mouse_pointer currently points at
   int click_x, click_y;
};



/*!
 * The types of primitives we allow the user to create.
 */
enum Primitive {CUBE, SPHERE, CYLINDER};

int x_resolution = 640; //!< Current window width.
int y_resolution = 480; //!< Current window height.

GLfloat clip_hither = 1;   //!< Hither clipping plane.
GLfloat clip_yon    = 100; //!< Yon clipping plane.

GLfloat gl_max_x = 1; //!< The X-extents of the current window, in NDC.
GLfloat gl_max_y = 640.0 / 480; //!< The Y-extents of the current window, in NDC.

Mouse mouse;
Camera camera;
CSG_Node::CSG_Type operation_type = CSG_Node::UNION;
bool negative_visibility=true;
bool affect_camera=true;  //!< Do we want to rotate the camera or the object.

list<CSG_Object *> objects;   //!< All objects in the scene.
CSG_Node *root = NULL;        //!< The root of our all-encompassing CSG tree.
CSG_Node *normal_root = NULL; //!< The root of the normalized CSG tree.

void translate_along_screen(CSG_Object *o, int mouse_x, int mouse_y, int center_x, int center_y);
void put_along_screen(CSG_Object *o, int mouse_x, int mouse_y, int center_x, int center_y);

void rebuild_normal_tree()
{
   DBG(cout << "rebuild_normal_tree" << endl);
   delete normal_root;
   normal_root = normalize(root);
   DBG(cout << "rebuild_normal_tree done" << endl);
}

CSG_Node *find_primitive(CSG_Node *tree, CSG_Object *object)
{
   if (tree->get_type() == CSG_Node::PRIMITIVE)
   {
      if (tree->get_object() == object)
         return tree;
      else
         return NULL;
   }
   else
   {
      CSG_Node *n = find_primitive(tree->get_left(), object);
      if (n) return n;
      return find_primitive(tree->get_right(), object);
   }
}

CSG_Node *delete_primitive(CSG_Node *tree, CSG_Object *object)
{
   DBG(cout << "delete_primitive" << endl);
   assert(!tree->get_parent());
   CSG_Node *p = find_primitive(tree, object);
   assert(p);

   objects.remove(object);
   delete object;

   if (!p->get_parent())
   {
      delete p;
      return NULL;
   }
   else
   {
      CSG_Node *pp = p->get_parent();
      bool delete_left = pp->get_left() == p;
      
      if (pp->get_parent())
      {
         if (delete_left)
            delete pp->detach_left();
         else
            delete pp->detach_right();
         return tree;
      }
      else
      {
         if (delete_left)
            return pp->delete_left_detach_right();
         else
            return pp->delete_right_detach_left();
      }
   }
}

/*!
 * GLUT display callback.
 */
void display()
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   //gluLookAt(camera.center_x, camera.center_y, camera.center_z + camera.radius,
   //          camera.center_x, camera.center_y, camera.center_z,
   //          0, 1, 0);
   
   gluLookAt(0, 0, camera.radius,
             0, 0, 0,
             0, 1, 0);
   
   glMultMatrixf(camera.rotation_matrix.data);

   glTranslatef(-camera.center_x, -camera.center_y, -camera.center_z);
   
   mouse.current_node=NULL;

   if(normal_root)
   {
      mouse.current_node = const_cast<CSG_Node *>
         (prerender(normal_root, mouse.x, mouse.y, negative_visibility)); 
      render(normal_root);
      if (mouse.selected_node)
         mouse.selected_node->get_object()->render_highlight(1, 0, 0);
      if (mouse.current_node && !mouse.selected_node)
         mouse.current_node->get_object()->render_highlight(1, 1, 0);
   }
   else
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   glutSwapBuffers();
}

/*!
 * GLUT window reshape callback. Updates the program's notion of the window's size
 * and aspect ratio.
 */
void reshape(GLsizei w, GLsizei h)
{
   x_resolution = w;
   y_resolution = h;

   glViewport(0, 0, w, h);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   gl_max_x = x_resolution > y_resolution ? 1 : (float)x_resolution / y_resolution;
   gl_max_y = x_resolution > y_resolution ? (float)y_resolution / x_resolution : 1;

   glFrustum(-gl_max_x, gl_max_x,
             -gl_max_y, gl_max_y,
             clip_hither, clip_yon);

   glMatrixMode(GL_MODELVIEW);

   glutPostRedisplay();
}

/*!
 * To do cleanup after glutMainLoop() exits, you have to register an atexit()
 * function, since glutMainLoop() doesn't return. Insert flameage here.
 */
void cleanup()
{
   DBG(cout << "cleanup() called, program is dying" << endl);

   DBG(cout << "   Destructing the tree recursively from the root" << endl);
   delete root;
   DBG(cout << "   Destructing the normal tree recursively from the root" << endl);
   delete normal_root;
   DBG(cout << "   Desctructing primitives" << endl);

   CSG_Object *ob;
   while(!objects.empty())
   {
      ob = objects.front();
      objects.pop_front();
      DBG(cout << "Deleting " << ob->get_name() << endl);
      delete ob;
   }

   DBG(cout << endl);
   DBG(cout << "All done, bye bye" << endl);
}

/*!
 * Translates a mouse x coordinate in viewport space to world space.
 */
GLfloat gl_coord_x(int x, int center)
{
   return (x - center) * 2.0 * gl_max_x / x_resolution;
}

/*!
 * Translates a mouse y coordinate in viewport space to world space.
 */
GLfloat gl_coord_y(int y, int center)
{
   return (center - y) * 2.0 * gl_max_y / y_resolution;
}

/*!
 * Adds an object to the current scene.
 *
 * \param attach_to The existing node that will be the left branch of the created operation.
 * \param operation_type The type of operation that relates the old and the new node.
 * \param primitive Type of object to insert.
 * \param mouse_x The screen X coordinate of the position to insert at.
 * \param mouse_y The screen Y coordinate of the position to insert at.
 */
void add_object(CSG_Node* attach_to, CSG_Node::CSG_Type operation_type,
                Primitive primitive, int mouse_x, int mouse_y)
{
   DBG(cout << "Placing new object at coordinates:" << endl);
   DBG(cout << "Mouse: (" << mouse_x << ", " << mouse_y << ")" << endl);

   assert(primitive == CUBE ||
          primitive == SPHERE ||
          primitive == CYLINDER);

   CSG_Object *object;

   switch(primitive)
   {
      case CUBE:
         object = new CSG_Object_Cube();
         break;
      case SPHERE:
         object = new CSG_Object_Sphere();
         break;
      case CYLINDER:
         object = new CSG_Object_Cylinder();
         break;
      default:
         cout << "No such primitive (" << primitive << ") - drawing a cube instead." << endl;
         object = new CSG_Object_Cube();
         break;
   }
   
   put_along_screen(object, mouse_x, mouse_y, x_resolution/2, y_resolution/2);
   objects.push_front(object);

   CSG_Node *node = new CSG_Node(object);

   if(!attach_to)
   {
      root = node;
   }
   else
   {
      root = CSG_Node::create_and_insert(operation_type, attach_to, node);
   }

   rebuild_normal_tree();

   glutPostRedisplay();

   DBG(cout << endl);
}

/*!
 * Move an object along the plane defined by the screen
 */
void translate_along_screen(CSG_Object *object, int mouse_x, int mouse_y, int center_x, int center_y)
{
   GLfloat dx = gl_coord_x(mouse_x, center_x) * camera.radius;
   GLfloat dy = gl_coord_y(mouse_y, center_y) * camera.radius;
   
   //To find the world coordinates of the point where the mouse pointer is,
   //multiply with the inverse (i.e. transpose, since it's a rotation matrix)
   //of the camera transform.
   Matrix form  = object->get_transform();
   Matrix cam   = camera.rotation_matrix;
   Matrix pose  = cam.transpose();
   Matrix late  = translate(dx, dy, 0);
   object->set_transform(pose * late * cam * form);
}

/*!
 * Put an object into the plane defined by the screen
 */
void put_along_screen(CSG_Object *object, int mouse_x, int mouse_y, int center_x, int center_y)
{
   GLfloat dx = gl_coord_x(mouse_x, center_x) * camera.radius;
   GLfloat dy = gl_coord_y(mouse_y, center_y) * camera.radius;

   //To find the world coordinates of the point where the mouse pointer is,
   //multiply with the inverse (i.e. transpose, since it's a rotation matrix)
   //of the camera transform.
   Matrix pose = camera.rotation_matrix.transpose();
   Matrix m    = translate(camera.center_x, camera.center_y, camera.center_z);
   Matrix late = translate(dx, dy, 0);
   object->set_transform(m * pose * late);
}

/*!
 * Centers the scene around an object.
 */
void center_on(CSG_Object *object)
{
   Vector u = object->get_transform() *  Vector(0, 0, 0, 1);
   //camera.rotation_matrix *= translate(-u.data[0], -u.data[1], -u.data[2]);
   camera.center_x = u.data[0];
   camera.center_y = u.data[1];
   camera.center_z = u.data[2];
}

void scale_object(GLfloat s)
{
  CSG_Object *object;
  if (mouse.current_node) object=mouse.current_node->get_object();
  else if (mouse.last_node) object=mouse.last_node->get_object();
  else return;
    
  object->set_transform(object->get_transform() * scale(s, s, s));
}

/*!
 * GLUT keyboard callback.
 */
void keypress(unsigned char key, int mouse_x, int mouse_y)
{
   switch(key)
   {
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
      case SAVE_KEY:
      {
         string filename;
         cout << "Filename: ";
         getline(cin, filename);
         save(filename, root, camera);
         break;
      }
      case LOAD_KEY:
      {
         delete root;
         CSG_Object *ob;
         while(!objects.empty())
         {
            ob = objects.front();
            objects.pop_front();
            DBG(cout << "Deleting " << ob->get_name() << endl);
            delete ob;
         }

         string filename;
         cout << "Filename: ";
         getline(cin, filename);
         root = load(filename, objects, camera);
         if(!root)
            cout << "Load failed!" << endl;
         rebuild_normal_tree();
         break;
      }
      case '/':
         set_render_partial(render_partial = 0);
         glutPostRedisplay();
         break;
      case '*':
         render_partial = -1;
         set_render_whole();
         glutPostRedisplay();
         break;
      case '+':
         if(render_partial != -1)
            set_render_partial(++render_partial);
         glutPostRedisplay();
         break;
      case '-':
         if(render_partial > 0)
            set_render_partial(--render_partial);
         glutPostRedisplay();
         break;
      case 27: //Escape
         exit(0);
         break;
      case ZOOM_KEY:
         if (affect_camera) {
	   camera.radius -= SPEED;
	   if(camera.radius < 0) camera.radius = 0;
	 }
	 else scale_object(1.25);
         glutPostRedisplay();
         break;
      case UNZOOM_KEY:
	if (affect_camera)
	  camera.radius += SPEED;
	else scale_object(0.8);
	 glutPostRedisplay();
         break;
      case INTERSECTION_KEY:
         operation_type = CSG_Node::INTERSECTION;
         break;
      case UNION_KEY:
         operation_type = CSG_Node::UNION;
         break;
      case SUBTRACTION_KEY:
         operation_type = CSG_Node::DIFFERENCE;
         break;
      case CUBE_KEY:
         add_object(root, operation_type, CUBE, mouse_x, mouse_y);
         break;
      case SPHERE_KEY:
	 add_object(root, operation_type, SPHERE, mouse_x, mouse_y);
	 break;
      case CYLINDER_KEY:
	 add_object(root, operation_type, CYLINDER, mouse_x, mouse_y);
	 break;
      case DELETE_OBJECT_KEY:
	 if(mouse.current_node)
         {
            root = delete_primitive(root, mouse.current_node->get_object());
            rebuild_normal_tree();
            glutPostRedisplay();
	 }
	 break;
      case CENTER_KEY:
         if (mouse.current_node)
         {
            center_on(mouse.current_node->get_object());
            glutPostRedisplay();
         }
         break;
   case TOGGLE_NEGATIVE_VISIBILITY:
     negative_visibility = !negative_visibility;
     glutPostRedisplay();
     break;
   case TOGGLE_CAMERA_OR_OBJECT:
     affect_camera = !affect_camera;
     //no need to redisplay
     break;
   default:
         break;
   }
}

/*!
 * GLUT mouse move and passive move callback. Updates the global mouse state.
 */
void mouse_moved(int x, int y)
{
   mouse.x = x;
   mouse.y = y;
   if(mouse.selected_node != NULL)
   {
      translate_along_screen(mouse.selected_node->get_object(), mouse.x, mouse.y,
                             mouse.click_x, mouse.click_y);
      mouse.click_x = mouse.x;
      mouse.click_y = mouse.y;
   }

   glutPostRedisplay();
}

/*!
 * GLUT mouse button callback. Updates the global mouse state.
 */
void mouse_pressed(int button, int state, int x, int y)
{
   mouse.buttons[button] = state;
   
   if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
   {
      mouse.click_x=x;
      mouse.click_y=y;
      mouse.selected_node=mouse.current_node;
      if (mouse.current_node) mouse.last_node=mouse.current_node;
   }
   else if(button==GLUT_LEFT_BUTTON && state==GLUT_UP)
   {
      mouse.selected_node=NULL;
   }
   
   glutPostRedisplay();
}


/*!
 * Updates a matrix with respect to mouse's position. 
 */
void update_matrix(Matrix &m)
{
   m = rotate_y(TURNSPEED * (mouse.x - x_resolution / 2)) * m;
   m = rotate_x(TURNSPEED * (mouse.y - y_resolution / 2)) * m;
}

/*!
 * Continuously checks if right (höger) button is pressed.
 */
void poll_mouse_button(int )
{
  if(mouse.buttons[GLUT_RIGHT_BUTTON] == GLUT_DOWN)
    {
      if (affect_camera)
	{
	  update_matrix(camera.rotation_matrix);
	  glutPostRedisplay();  
	}
      else
	if (mouse.last_node) {
	  Matrix m = mouse.last_node->get_object()->get_transform();
	  Vector v= m * Vector(0,0,0,1);
	  m = translate(-v.data[0], -v.data[1], -v.data[2]) * m;
	  update_matrix(m);
	  m = translate(v.data[0], v.data[1], v.data[2]) * m;
	  mouse.last_node->get_object()->set_transform(m);
	  glutPostRedisplay(); 
	}
    }	
  
  glutTimerFunc(CHECKMOUSE_INTERVAL, poll_mouse_button, 1);
  
}

int main(int argc, char **argv)
{
   atexit(cleanup);

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(x_resolution, y_resolution);
   glutInitWindowPosition(0, 0);
   glutCreateWindow("Solid Cheese - Modeler");

   if(argc > 1 && (string(argv[1]) == "--fullscreen" ||
                   string(argv[1]) == "-f"))
      glutFullScreen();

   glutDisplayFunc(display);
   glutTimerFunc(CHECKMOUSE_INTERVAL, poll_mouse_button, 1);
   glutKeyboardFunc(keypress);
   glutMotionFunc(mouse_moved);
   glutPassiveMotionFunc(mouse_moved);
   glutMouseFunc(mouse_pressed);
   glutReshapeFunc(reshape);
   
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glShadeModel(GL_SMOOTH);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);

   glCullFace(GL_BACK);

   glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();

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

   glutMainLoop();

   //Just to make GCC happy. We'll never get here, since glutMainLoop()
   //doesn't return.
   return 0;
}
