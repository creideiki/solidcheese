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
 * \file persistence.h
 * Interface to persistence functions.
 */

#include <string>
#include <list>

#include "csg_tree.h"
#include "camera.h"

/*!
 * Converts a CSG tree to a string in RPN.
 */
std::string stringify(const CSG_Node *tree);

/*!
 * Makes a string safe for exporting.
 */
std::string escape(const std::string &orig);

/*!
 * Make an escaped string readable again.
 */
std::string unescape(const std::string &orig);

/*!
 * Write a scene (tree and camera) to a file, overwriting its previous contents.
 *
 * \return 0 on success, >= 1 on failure.
 */
int save(const std::string &filename, const CSG_Node *tree, const Camera &camera);

/*!
 * Loads a saved CSG tree from a file.
 *
 * \param filename Name of file to load from.
 * \param objects The list holding all primitives in the scene. New primitives
 *                loaded from the file will be added to the front.
 * \param camera The camera object that will hold the camera information
 *               loaded from the file.
 * \return The root of the new tree, or NULL if there was an error.
 */
CSG_Node *load(const std::string &filename, std::list<CSG_Object *> &objects, Camera &camera);
