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
 * \file renderer.cpp
 * Functions to render a CSG tree.
 */

#include <cstdlib>
#include "debug.h"
#ifdef DEBUG
#  include <iostream>
#endif
#include <GL/glut.h>
#include "renderer_interface.h"

using namespace std;

static RenderType render_type = RENDER_CSG;
static int render_partial = -1;
static int render_counter;

typedef GLuint ZValue;
const GLenum ZBUFFER_TYPE = GL_DEPTH_COMPONENT;
const GLenum ZBUFFER_FORMAT = GL_UNSIGNED_INT;

const int OPENGL_BUGGINESS = 30;
const unsigned int COLOR_STEP = 64;

static ZValue *splitscreen_zmerged = NULL;

#define FETDEBUG \
   DBG(cout << __LINE__ << ": render_counter = " << render_counter << endl)

//! Clears the Z-buffer to zfar where the stencil test passes.
//! (can't use glClear since it doesn't use the stencil test.)
void draw_zfar()
{
   GLfloat zfar = 1.0;
   GLint dims[4];
   glGetIntegerv(GL_VIEWPORT, dims);
   glPushAttrib(GL_PIXEL_MODE_BIT);
   {
      // Draw a single pixel scaled to the whole buffer.
      glPixelZoom(dims[2], dims[3]);
      glDrawPixels(1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zfar);
   }
   glPopAttrib();
}

//! Render a primitive to the Z-buffer.
bool render_primitive(CSG_Object *obj)
{
   obj->render();
   FETDEBUG;
   if (!--render_counter) return false;
   return true;
}

//! Draw all primitives in tree to the Z-buffer using current GL settings.
//! Tree must only contain intersected primitives.
//! Returns the number of primitives in the tree.
int render_intersected_primitives(const CSG_Node *tree)
{
   assert(tree->get_type() == CSG_Node::INTERSECTION ||
          tree->get_type() == CSG_Node::PRIMITIVE);
   if (tree->get_type() == CSG_Node::PRIMITIVE)
   {
      return render_primitive(tree->get_object());
   }
   else
   {
      int r1 = render_intersected_primitives(tree->get_left());
      if (!r1) return 0;
      int r2 = render_intersected_primitives(tree->get_right());
      if (!r2) return 0;
      return r1 + r2;
   }
}

//! Overwrite the Z-buffer with the image of an intersection.
//! Tree must only contain intersected primitives.
bool scs_intersect(const CSG_Node *tree)
{
   if (tree->get_type() == CSG_Node::PRIMITIVE)
   {
      // Intersection is a single object. No need to do anything complicated.
      DBG(cout << "scs_intersect: primitive" << endl);

      glDisable(GL_STENCIL_TEST);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_ALWAYS);
      glCullFace(GL_BACK);

      glClearDepth(1.0);
      glClear(GL_DEPTH_BUFFER_BIT);

      FETDEBUG;
      if (!--render_counter) return false;

      if (!render_primitive(tree->get_object()))
         return false;

      return true;
   }

   DBG(cout << "scs_intersect: intersection" << endl);

   // Draw a reversed Z-buffer. Store the z value for the furthest front face,
   // instead of the nearest, at each pixel.
   glDisable(GL_STENCIL_TEST);
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_GREATER);
   glCullFace(GL_BACK);

   // Clear Z-buffer to znear, stencil buffer to 0
   glClearDepth(0.0);
   glClearStencil(0);
   glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   FETDEBUG;
   if (!--render_counter) return false;
   
   int num_objects = render_intersected_primitives(tree);
   if (!num_objects) return false;
   DBG(cout << "num_objects == " << num_objects << endl);

   // Use the stencil buffer to count the number of back faces
   // behind the furthest front face.
   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS, 0, ~0);
   glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
   glDepthMask(GL_FALSE);
   glDepthFunc(GL_GREATER);
   glCullFace(GL_FRONT);

   if (!render_intersected_primitives(tree))
      return false;

   // Erase the parts where there are too few back faces, since not
   // all objects intersects there.
   glStencilFunc(GL_NOTEQUAL, num_objects, ~0);
   glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_ALWAYS);

   draw_zfar();
   FETDEBUG;
   if (!--render_counter) return false;
   return true;
}

//! Render a subtracted primitive.
bool scs_subtract_primitive(const CSG_Node *node)
{
   // When the current z is inside this subtraced object, move it back to
   // the back face of this object.

   // Set stencil to 1 where front face is in front of previous surface.
   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS, 1, ~0);
   glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_FALSE);
   glDepthFunc(GL_LESS);
   glCullFace(GL_BACK);

   if (!render_primitive(node->get_object()))
      return false;

   // Update z where stencil is 1 and back face is behind previous surface.
   glStencilFunc(GL_EQUAL, 1, ~0);
   glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_GREATER);
   glCullFace(GL_FRONT);

   if (!render_primitive(node->get_object()))
      return false;

   // Stencil buffer is all 0 when leaving
   return true;
}

//! Overwrite the Z-buffer with the image of a product.
bool scs_product(const CSG_Node *tree)
{
   DBG(cout << "scs_product" << endl);
   const CSG_Node *first_diff = tree;
   const CSG_Node *last_diff = tree;
   
   if (tree->get_type() == CSG_Node::DIFFERENCE)
   {
      // Count the number of subtracted objects.
      int num_subtracted = 1;
      while (last_diff->get_left()->get_type() == CSG_Node::DIFFERENCE)
      {
         ++num_subtracted;
         last_diff = last_diff->get_left();
      }

      // Overwrite z with the image of the intersected objects.
      CSG_Node *first_inter = last_diff->get_left();
      if (!scs_intersect(first_inter))
         return false;

      const CSG_Node *node = first_diff;
      if (!scs_subtract_primitive(node->get_right()))
         return false;

      // Render the subtracted objects. The correct order is unknown,
      // but unnecessary subtractions doesn't harm the result. Therefore,
      // it is enough that the correct order is embedded inside the
      // sequence of subtractions we perform. The sequence chosen here 
      // is one that embeds all possible orderings.

      // Example:
      // For four objects, we subtract (ABCDCBABCDCBA)
      // Any ordering of ABCD, e.g.    (  C   A  D B ),
      // is embedded inside this sequence.

      for(int pass = 0; pass < num_subtracted; ++pass)
      {
         if (node == first_diff)
            while (node != last_diff)
            {
               node = node->get_left();
               if (!scs_subtract_primitive(node->get_right()))
                  return false;
            }
         else // node == last_diff
            while (node != first_diff)
            {
               node = node->get_parent();
               if (!scs_subtract_primitive(node->get_right()))
                  return false;
            }
      }

      // Reset z to zfar in holes through the objects. Holes are where
      // the current z is further away than the back faces of the
      // intersected objects.
      
      // Stencil buffer is all 0 here. Set it to 1 where any back face
      // of an intersected object is in front of the current z.
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS, 1, ~0);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glDepthFunc(GL_LESS);
      glCullFace(GL_FRONT);

      if (!render_intersected_primitives(first_inter))
         return false;

      // Clear z to zfar where stencil is 1.
      glStencilFunc(GL_EQUAL, 1, ~0);
      glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_ALWAYS);
     
      draw_zfar();
      FETDEBUG;
      if (!--render_counter) return false;

      return true;
   }
   else
      // No subtractions in the product, saves work.
      return scs_intersect(tree);
}

//! Traverse tree and render the products in it.
bool scs_traverse_products(const CSG_Node *tree, bool first,
                           GLint *dims, ZValue *zmerged)
{
   if (tree->get_type() == CSG_Node::PRIMITIVE)
   {
      // No need to copy anything. Just render the primitive.
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      glCullFace(GL_BACK);

      if (first)
      {
         glClearDepth(1.0);
         glClear(GL_DEPTH_BUFFER_BIT);
      }

      if (!render_primitive(tree->get_object()))
         return false;

      return true;
   }
   else if (tree->get_type() != CSG_Node::UNION)
   {
      if (!first)
      {
         // Save the Z-buffer, containing all previously drawn products,
         // in zmerged.
         DBG(cout << "Saving zmerged" << endl);
         glReadPixels(dims[0], dims[1], dims[2], dims[3],
                      ZBUFFER_TYPE, ZBUFFER_FORMAT, zmerged);
         DBG(cout << "...done" << endl);
         FETDEBUG;
         if (!--render_counter) return false;
      }

      // Replace the Z-buffer contents with this new product.
      // For the first product encountered, this is all that is done.
      if (!scs_product(tree))
         return false;

      if (!first)
      {
         // Combine the new product with all the previous ones.
         // After this, the Z-buffer again contains all drawn products.
         DBG(cout << "Drawing zmerged" << endl);
         glDisable(GL_STENCIL_TEST);
         glEnable(GL_DEPTH_TEST);
         glDepthMask(GL_TRUE);
         glDepthFunc(GL_LESS);
         glDrawPixels(dims[2], dims[3], ZBUFFER_TYPE, ZBUFFER_FORMAT,
                      zmerged);
         DBG(cout << "...done" << endl);
         FETDEBUG;
         if (!--render_counter) return false;
      }

      return true;
   }
   else
   {
      if (!scs_traverse_products(tree->get_left(), first, dims, zmerged))
         return false;
      return scs_traverse_products(tree->get_right(), false, dims, zmerged);
   }
}

//! Render a normalized CSG tree to the Z-buffer.
void scs_render(const CSG_Node *tree)
{
  if (render_type == RENDER_NO_CSG ||
      render_type == RENDER_NO_CSG_Z)
     return;

   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

   // Allocate merged Z-buffer.
   GLint dims[4];
   glGetIntegerv(GL_VIEWPORT, dims);
   GLint halfDims[4] = { dims[0], dims[1], dims[2], dims[3] };
   GLint *usedDims = dims;

   DBG(cout << "Viewport: (" << dims[0] << ", " << dims[1] << "), "
            << dims[2] << " x " << dims[3] << endl);

   if (render_type == RENDER_CSG_SPLIT_SCREEN)
   {
      // Restrict viewport to 1/4 of screen.
      halfDims[2] /= 2;
      halfDims[3] /= 2;
      glViewport(halfDims[0], halfDims[1], halfDims[2], halfDims[3]);
      usedDims = halfDims;
      DBG(cout << "Rendering to: (" << halfDims[0] << ", " << halfDims[1]
               << "), " << halfDims[2] << " x " << halfDims[3] << endl);
   }

   DBG(cout << "Allocating zmerged" << endl);
   ZValue *zmerged = new ZValue[dims[2] * dims[3]];
   assert(zmerged);
   if (render_type == RENDER_CSG_SPLIT_SCREEN)
   {
      // Avoid drawing random stuff on screen
      memset(zmerged, 0xFF, dims[2] * dims[3] * sizeof(ZValue));
   }
   DBG(cout << "   ...done" << endl);

   scs_traverse_products(tree, true, usedDims, zmerged);

   if (render_type == RENDER_CSG_SPLIT_SCREEN)
   {
      // Restore viewport to full screen
      glViewport(dims[0], dims[1], dims[2], dims[3]);
      splitscreen_zmerged = zmerged;
   }
   else
   {
      delete[] zmerged;
   }
}

//! Render a primitive to the color buffer, using ugly hacks to work around
//! evil OpenGL implementations that round Z values in strange ways.
//! Set OPENGL_BUGGINESS to a suitable value.
void render_with_workaround(const CSG_Node *tree, bool csg)
{
   if (csg && OPENGL_BUGGINESS)
   {
      glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glEnable(GL_STENCIL_TEST);
      
      // Set stencil to 1 where forward-offset surface
      // isn't too far back.
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glStencilFunc(GL_ALWAYS, 1, ~0);
      glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
      glDepthFunc(GL_LEQUAL);
      glPolygonOffset(0, -OPENGL_BUGGINESS);
      tree->get_object()->render();
      
      // Draw color where stencil is 1 and backward-offset surface
      // isn't too close.
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glStencilFunc(GL_EQUAL, 1, ~0);
      glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
      glDepthFunc(GL_GEQUAL);
      glPolygonOffset(0, OPENGL_BUGGINESS);
      tree->get_object()->render();
      
      glPopAttrib();
   }
   else
      tree->get_object()->render();
}

//! Render a tree to the color buffer, using a unique color per object.
unsigned long render_picking_colors(const CSG_Node *tree, unsigned long color,
                                    bool csg)
{
   if(tree->get_type() == CSG_Node::PRIMITIVE)
   {
      GLubyte r =  color        & 0xFF;
      GLubyte g = (color >>  8) & 0xFF;
      GLubyte b = (color >> 16) & 0xFF;
      //cout << "RGB: " << (int)r << " " << (int)g << " " << (int)b << endl;
      glColor3ub(r, g, b);
      
      render_with_workaround(tree, csg);

      r += COLOR_STEP;
      if (!r)
      {
         g += COLOR_STEP;
         if (!g)
            b += COLOR_STEP;
      }

      return r | (g << 8) | (b << 16);
   }
   else
   {
      glCullFace(GL_BACK);
      color = render_picking_colors(tree->get_left(), color, csg);
      glCullFace(csg && tree->get_type() == CSG_Node::DIFFERENCE
                 ? GL_FRONT : GL_BACK);
      return render_picking_colors(tree->get_right(), color, csg);
   }
}

//! Returns the object drawn in a certain color.
const CSG_Node *object_with_picking_color(const CSG_Node *tree,
                                          unsigned long count =
                                          (unsigned long)-1)
{
   static unsigned long counter;
   if (count != (unsigned long)-1)
   {
      if (count)
         counter = count - 1;
      else
         return NULL; // Picked background color
   }
   if(tree->get_type() == CSG_Node::PRIMITIVE)
   {
      if (!counter) return tree;
      --counter;
      return NULL;
   }
   else
   {
      const CSG_Node *n;
      if ((n = object_with_picking_color(tree->get_left())))
         return n;
      if ((n = object_with_picking_color(tree->get_right())))
         return n;
      return NULL;
   }
}

unsigned long color_to_counter(GLubyte r, GLubyte g, GLubyte b)
{
   return (r + COLOR_STEP / 2) / COLOR_STEP +
      ((g + COLOR_STEP / 2) / COLOR_STEP * (256 / COLOR_STEP)) +
      ((b + COLOR_STEP / 2) / COLOR_STEP * (256 / COLOR_STEP)
       * (256 / COLOR_STEP));
}


// Interface function
// Renders a CSG tree to the Z buffer, and returns the object drawn at
// the specified image coordinate.
const CSG_Node *prerender(const CSG_Node *tree, int mouse_x, int mouse_y,
                          bool select_invisible)
{
   render_counter = render_partial + 1;

   GLint dims[4];
   glGetIntegerv(GL_VIEWPORT, dims);
   mouse_y = dims[3] - 1 - mouse_y;

   // For some incomprehensible reason lighting needs to be enabled in
   // the prerender pass too on some hardware. Disable the individual
   // lights instead.
   glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
   glEnable(GL_LIGHTING);
   glDisable(GL_LIGHT0);
   GLfloat flat_light[]  = { 1, 1, 1, 1 };
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, flat_light);

   const CSG_Node *result = NULL;

   if (render_partial != -1)
   {
      glClearStencil(0);
      glClear(GL_STENCIL_BUFFER_BIT);
   }

   if (render_type == RENDER_CSG_SPLIT_SCREEN)
   {
      scs_render(tree);
   }
   else if (select_invisible)
   {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      glCullFace(GL_BACK);
      
      glClearDepth(1.0);
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      render_picking_colors(tree, COLOR_STEP, false);

      GLubyte pixel[3];
      glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
      unsigned long count = color_to_counter(pixel[0], pixel[1], pixel[2]);
   
      result = object_with_picking_color(tree, count);

      scs_render(tree);
   }
   else
   {
      scs_render(tree);

      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glDepthFunc(GL_EQUAL);
      glCullFace(GL_BACK);
      
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      render_picking_colors(tree, COLOR_STEP, true);

      GLubyte pixel[3];
      glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
      unsigned long count = color_to_counter(pixel[0], pixel[1], pixel[2]);
      
      result = object_with_picking_color(tree, count);

      DBG(cout << "Mouse ( " << mouse_x << ", " << mouse_y << "): "
        << count << endl);
   }

   glPopAttrib();
   return result;
}

//! Render a CSG tree using current GL settings.
void traverse_and_render(const CSG_Node *tree, bool csg)
{
   if(tree->get_type() == CSG_Node::PRIMITIVE)
   {
      GLfloat r, g, b;
      tree->get_object()->get_color(r, g, b);
      glColor3f(r, g, b);
      render_with_workaround(tree, csg);
   }
   else
   {
      glCullFace(GL_BACK);
      traverse_and_render(tree->get_left(), csg);
      glCullFace(csg && tree->get_type() == CSG_Node::DIFFERENCE
                 ? GL_FRONT : GL_BACK);
      traverse_and_render(tree->get_right(), csg);
   }
}

//! Render CSG normally.
void render_csg(const CSG_Node *tree)
{
   // The Z-buffer contains the exact Z value for the surface to be drawn at
   // each pixel, so we can draw our surfaces in any order.
   glDepthMask(GL_FALSE);
   glDepthFunc(GL_EQUAL);
   glCullFace(GL_BACK);
   
   glClearStencil(0);
   glClear(GL_STENCIL_BUFFER_BIT);

   traverse_and_render(tree, true);
}

//! Render objects without CSG.
void render_trivial(const CSG_Node *tree)
{
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_LESS);
   glCullFace(GL_BACK);

   glClearDepth(1.0);
   glClear(GL_DEPTH_BUFFER_BIT);
   
   traverse_and_render(tree, false);
}

void invert_z_attribs()
{
   glPushAttrib(GL_PIXEL_MODE_BIT);
   glPixelTransferf(GL_RED_SCALE, -1);
   glPixelTransferf(GL_RED_BIAS, 1);
   glPixelTransferf(GL_GREEN_SCALE, -1);
   glPixelTransferf(GL_GREEN_BIAS, 1);
   glPixelTransferf(GL_BLUE_SCALE, -1);
   glPixelTransferf(GL_BLUE_BIAS, 1);
}

//! Copies the Z buffer to the color buffer as a grayscale image.
void display_zbuffer()
{
   // Display the Z-buffer for debugging
   GLint dims[4];
   glGetIntegerv(GL_VIEWPORT, dims);
   ZValue *buf = new ZValue[dims[2] * dims[3]];
   
   glReadPixels(dims[0], dims[1], dims[2], dims[3],
                ZBUFFER_TYPE, ZBUFFER_FORMAT, buf);
   
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);

   invert_z_attribs();
   glDrawPixels(dims[2], dims[3], GL_LUMINANCE, ZBUFFER_FORMAT, buf);
   glPopAttrib();
   
   delete[] buf;
}

void render_split_screen(const CSG_Node *tree)
{
   GLint dims[4];
   glGetIntegerv(GL_VIEWPORT, dims);

   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);

   GLfloat i2r[] = { 0, 1, 0, 0, 1, 0, 1, 1 };
   GLfloat i2g[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
   GLfloat i2b[] = { 0, 0, 0, 1, 1, 1, 0, 1 };

   glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 8, i2r);
   glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 8, i2g);
   glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 8, i2b);

   // Read out the stencil buffer.
   GLubyte *stencil = new GLubyte[(dims[2] / 2 + 3) / 4 * 4 * (dims[3] / 2)];
   glReadPixels(dims[0], dims[1], dims[2] / 2, dims[3] / 2,
                GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencil);

   // Read out the depth buffer.
   ZValue *zbuf = new ZValue[dims[2] * dims[3] / 4];
   glReadPixels(dims[0], dims[1], dims[2] / 2, dims[3] / 2,
                ZBUFFER_TYPE, ZBUFFER_FORMAT, zbuf);

   // Draw color image.
   glViewport(dims[0], dims[1], dims[2] / 2, dims[3] / 2);
   glEnable(GL_DEPTH_TEST);
   render_csg(tree);
   glViewport(dims[0], dims[1], dims[2], dims[3]);
   
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);

   // Copy color image to the upper left quadrant.
   DBG(cout << "Does this crash? ");
   glBitmap(0, 0, 0, 0, 0, dims[3] / 2, NULL);
   DBG(cout << "No, it doesn't." << endl);
   glCopyPixels(dims[0], dims[1], dims[2] / 2, dims[3] / 2, GL_COLOR);
   
   // Write depth image to the upper rigth quadrant.
   glBitmap(0, 0, 0, 0, dims[2] / 2, 0, NULL);
   invert_z_attribs();
   glDrawPixels(dims[2] / 2, dims[3] / 2, GL_LUMINANCE, ZBUFFER_FORMAT, zbuf);
   glPopAttrib();
   delete[] zbuf;

   // Write stencil image to the lower left quadrant.
   glBitmap(0, 0, 0, 0, -dims[2] / 2, -dims[3] / 2, NULL);
   glDrawPixels(dims[2] / 2, dims[3] / 2, GL_COLOR_INDEX, GL_UNSIGNED_BYTE,
                stencil);
   delete[] stencil;

   // Draw merged Z-buffer to the lower right quadrant.
   glBitmap(0, 0, 0, 0, dims[2] / 2, 0, NULL);
   invert_z_attribs();
   glDrawPixels(dims[2] / 2, dims[3] / 2, GL_LUMINANCE, ZBUFFER_FORMAT,
                splitscreen_zmerged);
   glPopAttrib();
   delete[] splitscreen_zmerged;
   splitscreen_zmerged = 0;

   // Restore raster position to origin.
   glBitmap(0, 0, 0, 0, -dims[2] / 2, 0, NULL);
}

// Interface function
void set_render_type(RenderType type)
{
   render_type = type;
}

void set_render_partial(int part)
{
   render_partial = part;
}

void set_render_whole()
{
   render_partial = -1;
}

// Interface function
// Draws the final image in the color buffer.
void render(const CSG_Node *tree)
{
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glClear(GL_COLOR_BUFFER_BIT);

   glDisable(GL_STENCIL_TEST);
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   switch (render_type)
   {
   case RENDER_CSG:
      render_csg(tree);
      break;
   case RENDER_CSG_Z:
      display_zbuffer();
      break;
   case RENDER_NO_CSG:
      render_trivial(tree);
      break;
   case RENDER_NO_CSG_Z:
      render_trivial(tree);
      display_zbuffer();
      break;
   case RENDER_CSG_SPLIT_SCREEN:
      render_split_screen(tree);
      break;
   default:
      assert(false);
   }
}
