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
 * \file csg_tree.h
 * Class representing a (normalizeable) CSG tree
 */

#ifndef __CSG_TREE_H__
#define __CSG_TREE_H__

#include "csg_object.h"

//#include <random_compiler_errors.h>
#undef DIFFERENCE
//#include <rants/ms_windows/stupid_defines.h>

/*!
 * Represents a node in a CSG tree.
 *
 * All objects of this class should be allocated with new! Using global
 * or stack objects in a tree will cause bugs when the tree is destroyed!
 */
class CSG_Node
{
public:

   /*!
    * PRIMITIVE is a primitive geometry node. It has no children, but a
    * pointer to a primitive geometric object to render.
    * The other kinds are operation nodes, which always have a left
    * and a right child.
    */
   enum CSG_Type
   {
      PRIMITIVE = 1,
      UNION,
      INTERSECTION,
      DIFFERENCE
   };

   /*!
    * Recursively destroys this subtree (but not any CSG_Object:s
    * pointed to by the leafs).
    */
   ~CSG_Node();

   //! Creates a new primitive node.
   explicit CSG_Node(CSG_Object *object);

   /*!
    * Recursively copies the subtree rooted in node.
    * Note that this sets parent of the copy to NULL.
    */
   explicit CSG_Node(const CSG_Node &node);

   /*!
    * Creates a new operation node with given left and right children.
    * If neither of the children had a parent before, the new node will
    * have no parent.
    * If exactly one of the children had a parent, that parent will
    * become the parent of the new node.
    * It is an error for both the left and right child to already have
    * a parent.
    * \param type What kind of operation the new node is.
    * \param left Left child.
    * \param right Right child.
    * \return The newly created operation node.
    */
   static CSG_Node *create_and_insert(CSG_Type type,
                                      CSG_Node *left,
                                      CSG_Node *right);

   /*!
    * Destroys this operation node, replacing it in the tree with its
    * right child, and returns its left child. The operation node
    * must have a parent.
    */
   CSG_Node *detach_left();

   /*!
    * Destroys this operation node, replacing it in the tree with its
    * left child, and returns its right child. The operation node
    * must have a parent.
    */
   CSG_Node *detach_right();

   /*!
    * Destroys this operation node and its right child, and returns
    * its left child  The operation node must not have a parent.
    */
   CSG_Node *delete_right_detach_left();

   /*!
    * Destroys this operation node and its left child, and returns
    * its right child  The operation node must not have a parent.
    */
   CSG_Node *delete_left_detach_right();

   /*!
    * Replaces the left child with the given tree. Deletes the previous
    * left subtree.
    */
   void delete_and_replace_left(CSG_Node *tree);

   /*!
    * Replaces the right child with the given tree. Deletes the previous
    * right subtree.
    */
   void delete_and_replace_right(CSG_Node *tree);

   /*!
    * Returns an RPN string representation of the subtree rooted at this node,
    * for use in persistence.
    */
   std::string stringify() const;

   //! Get the left child. Can only be used in an operation node.
   CSG_Node *get_left() const;
   //! Get the right child. Can only be used in an operation node.
   CSG_Node *get_right() const;
   //! Get the parent. Will be NULL in the root node.
   CSG_Node *get_parent() const;
   //! Get the CSG_Object. Can only be used in a primitive node.
   CSG_Object *get_object() const;
   //! Get the type of the object.
   CSG_Type get_type() const;

   //! Set the type of the object. Can only be used in an operation node.
   void set_type(CSG_Type type);

private:
   CSG_Node(CSG_Type type, CSG_Node *left, CSG_Node *right);
   void operator=(const CSG_Node &);

   CSG_Type _type;
   CSG_Object *_object; //!< Will be NULL in an operation node.

   CSG_Node *_left;     //!< Will be NULL in a primitive node.
   CSG_Node *_right;    //!< Will be NULL in a primitive node.

   CSG_Node *_parent;
};

#endif
