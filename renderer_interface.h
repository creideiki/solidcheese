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
 * \file renderer_interface.h
 * Exposes the renderer to the modeler.
 */

#ifndef __RENDERER_INTERFACE_H__
#define __RENDERER_INTERFACE_H__

#include "csg_tree.h"

/*!
 * Does all the magic stuff for CSG rendering, but doesn't draw to the color
 * buffer. Calculates Z-buffer values for later drawing of colors after the
 * modeler has decided what to highlight. Call render() to do that.
 * 
 * \param tree The root of the normalized CSG tree to render.
 * \param mouse_x The X coordinate of the mouse pointer, for calculating which
 *                object is selected. Given in pixels in window coordinates.
 * \param mouse_y The Y coordinate of the mouse pointer, for calculating which
 *                object is selected. Given in pixels in window coordinates.
 * \param select_invisible
 *                If false, the function returns the object belonging to the
 *                visible surface under the mouse pointer.
 *                If true, the function returns the closest object, even if
 *                the part under the cursor is not visible.
 * \return The object under the mouse pointer.
 * \sa render()
 */
const CSG_Node *prerender(const CSG_Node *tree, int mouse_x, int mouse_y,
                          bool select_invisible = false);

/*!
 * Uses the Z-buffer values calculated by prerender() to draw the final image
 * into the color buffer. Horrible things will happen if you don't call
 * prerender() first. This is not checked.
 *
 * \param tree The root of the normalized CSG tree to render.
 * \sa prerender()
 */
void render(const CSG_Node *tree);

enum RenderType
{
   RENDER_CSG,              //!< Show CSG result
   RENDER_CSG_Z,            //!< Show Z buffer of CSG result
   RENDER_NO_CSG,           //!< Show all primitives, as if all operations
                            //!< were unions.
   RENDER_NO_CSG_Z,         //!< Show Z buffer of union of primitives
   RENDER_CSG_SPLIT_SCREEN  //!< Show 4 smaller images containing color,
                            //!< Z, stencil and zmerged.
};

/*!
 * Sets the kind of rendering performed by prerender() and render().
 */
void set_render_type(RenderType type);

/*! Selects that prerender() and render() should only perform part of
 *  the rendering process, for pedagogical purposes.
 *
 *  \param part The number of steps in the rendering process performed.
 *  \sa set_render_whole
 */
void set_render_partial(int part);

/*! Selects that prerender() and render() should perform the whole
 *  rendering process.
 *
 *  \sa set_render_partial
 */
void set_render_whole();

#endif
