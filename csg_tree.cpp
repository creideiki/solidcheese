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
 * \file csg_tree.cpp
 * Implementation of a CSG tree
 */

#include <cassert>
#include <cstdlib>
#ifdef DEBUG
#  include <iostream>
#  include <iomanip>
#endif
#include "debug.h"
#include "csg_tree.h"

using namespace std;

CSG_Node::CSG_Node(CSG_Object *object) : 
   _type(PRIMITIVE), _object(object), _left(NULL), _right(NULL), _parent(NULL)
{
}

CSG_Node::CSG_Node(const CSG_Node &node) :
   _type(node._type), _object(node._object), _parent(NULL)
{
   if (node._left)
   {
      _left = new CSG_Node(*node._left);
      _left->_parent = this;
   }
   else
      _left = NULL;

   if (node._right)
   {
      _right = new CSG_Node(*node._right);
      _right->_parent = this;
   }
   else 
      _right = NULL;
}

CSG_Node::CSG_Node(CSG_Type type, CSG_Node *left, CSG_Node *right) :
   _type(type), _object(NULL), _left(left), _right(right)
{
   assert(!(_left->_parent && _right->_parent));

   if (_left->_parent)
      _parent = _left->_parent;
   else
      _parent = _right->_parent;

   _left->_parent = this;
   _right->_parent = this;
}

CSG_Node::~CSG_Node()
{
   //If you get crashes here, you probably haven't read the docunentation för CSG_Node,
   //especially the part where it says that you're not supposed to allocate stack objects
   //of this class.

   //DBG(cout << "Deleting node 0x" << hex << (unsigned int)this <<
   //    ", left = 0x" << hex << (unsigned int)_left <<
   //    ", right = 0x" << hex << (unsigned int)_right << endl);
   //DBG(cout << "Deleting left = 0x" << hex << (unsigned int)_left << endl);
   delete _left;
   //DBG(cout << "Deleting right = 0x" << hex << (unsigned int)_right << endl);
   delete _right;
   //DBG(cout << "Deleting this = 0x" << hex << (unsigned int)this << endl);
}

CSG_Node *CSG_Node::create_and_insert(CSG_Type type,
                                      CSG_Node *left,
                                      CSG_Node *right)
{
   return new CSG_Node(type, left, right);
}

CSG_Node *CSG_Node::detach_left()
{
   assert(_type != PRIMITIVE);
   assert(_parent);
   assert(_parent->_left == this || _parent->_right == this);
   (_parent->_left == this ? _parent->_left : _parent->_right) = _right;
   _right->_parent = _parent;
   CSG_Node *ret = _left;
   ret->_parent = NULL;
   _left = NULL; _right = NULL;
   delete this;
   return ret;
}

CSG_Node *CSG_Node::detach_right()
{
   assert(_type != PRIMITIVE);
   assert(_parent);
   assert(_parent->_left == this || _parent->_right == this);
   (_parent->_left == this ? _parent->_left : _parent->_right) = _left;
   _left->_parent = _parent;
   CSG_Node *ret = _right;
   ret->_parent = NULL;
   _left = NULL; _right = NULL;
   delete this;
   return ret;
}

CSG_Node *CSG_Node::delete_left_detach_right()
{
   assert(_type != PRIMITIVE);
   assert(!_parent);
   delete _left;
   CSG_Node *ret = _right;
   ret->_parent = NULL;
   _left = NULL; _right = NULL;
   delete this;
   return ret;
}

CSG_Node *CSG_Node::delete_right_detach_left()
{
   assert(_type != PRIMITIVE);
   assert(!_parent);
   delete _right;
   CSG_Node *ret = _left;
   ret->_parent = NULL;
   _left = NULL; _right = NULL;
   delete this;
   return ret;
}

void CSG_Node::delete_and_replace_left(CSG_Node *tree)
{
   assert(_type != PRIMITIVE);
   assert(tree);
   assert(!tree->_parent);
   delete _left;
   _left = tree;  
   tree->_parent = this;
}

void CSG_Node::delete_and_replace_right(CSG_Node *tree)
{
   assert(_type != PRIMITIVE);
   assert(tree);
   assert(!tree->_parent);
   delete _right;
   _right = tree;
   tree->_parent = this;
}

string CSG_Node::stringify() const
{
   switch(_type)
   {
      case UNION:
         return _left->stringify() + " " +
                _right->stringify() + " + ";
         break;
      case DIFFERENCE:
         return _left->stringify() + " " +
                _right->stringify() + " - ";
         break;
      case INTERSECTION:
         return _left->stringify() + " " +
                _right->stringify() + " * ";
         break;
      case PRIMITIVE:
         return "[ " + _object->stringify() + " ]";
         break;
      default:
         return "I don't think this tree is in Kansas anymore...";
         break;
   }
}

CSG_Node *CSG_Node::get_left() const
{
   assert(_left);
   return _left;
}

CSG_Node *CSG_Node::get_right() const
{
   assert(_right);
   return _right;
}

CSG_Node *CSG_Node::get_parent() const
{
   return _parent;
}

CSG_Object *CSG_Node::get_object() const
{
   assert(_object);
   return _object;
}

CSG_Node::CSG_Type CSG_Node::get_type() const
{
   return _type;
}

void CSG_Node::set_type(CSG_Node::CSG_Type type)
{
   assert(_type != PRIMITIVE);
   assert(type != PRIMITIVE);
   _type = type;
}
