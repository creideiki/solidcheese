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
 * \file normalize.cpp
 * Implementation of a normalizer.
 */

#include "csg_tree.h"
#include "normalize.h"

#include <cassert>
#include <string>
#include <stack>
#include "debug.h"
#ifdef DEBUG
#  include <iostream>
#endif

using namespace std;

int normalize_counter = 0;

CSG_Node *get_tree(string str)
{
   stack<CSG_Node*> treeStack;
   CSG_Node *op1, *op2;
   CSG_Object_Sphere *ob;

   for(int i = 0; i < (int)str.size(); i++)
   {
      switch(str[i])
      {
         case '+':
            op2 = treeStack.top();
            treeStack.pop();
            op1 = treeStack.top();
            treeStack.pop();
            treeStack.push(CSG_Node::create_and_insert(CSG_Node::UNION, op1, op2));
            break;
         case '-':
            op2 = treeStack.top();
            treeStack.pop();
            op1 = treeStack.top();
            treeStack.pop();
            treeStack.push(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE, op1, op2));
            break;
         case '*':
            op2 = treeStack.top();
            treeStack.pop();
            op1 = treeStack.top();
            treeStack.pop();
            treeStack.push(CSG_Node::create_and_insert(CSG_Node::INTERSECTION, op1, op2));
            break;
         default:
            ob = new CSG_Object_Sphere();
            ob->set_name(str.substr(i,1));
            treeStack.push(new CSG_Node(ob));
            break;
      }
   }

   return treeStack.top();
}

string get_desc(const CSG_Node *tree)
{
   switch(tree->get_type())
   {
      case CSG_Node::UNION:
         return get_desc(tree->get_left())+get_desc(tree->get_right())+"+";
         break;
      case CSG_Node::DIFFERENCE:
         return get_desc(tree->get_left())+get_desc(tree->get_right())+"-";
         break;
      case CSG_Node::INTERSECTION:
         return get_desc(tree->get_left())+get_desc(tree->get_right())+"*";
         break;
      case CSG_Node::PRIMITIVE:
         return tree->get_object()->get_name();
         break;
      default:
         return "The compiler settings are a little too pedantic!";
         break;
   }
}

static void do_stuff(CSG_Node *tree, bool is_left)
{
   DBG(cout << "do_stuff" << endl);

   // How to write unmaintainable code
   CSG_Node *child = is_left ? tree->get_left() : tree->get_right();
   CSG_Node *other_child = is_left ?
      new CSG_Node(*tree->get_right())
      : new CSG_Node(*tree->get_left());

   CSG_Node::CSG_Type root_type = tree->get_type(),
      child_type = child->get_type();
   
   assert(root_type != CSG_Node::PRIMITIVE);
   if(child_type == CSG_Node::PRIMITIVE)
   {
      delete other_child;
      return;
   }

   CSG_Node *childs_left = new CSG_Node(*child->get_left()),
      *childs_right = new CSG_Node(*child->get_right());
   
   // A-(B+C) => (A-B)-C
   if(root_type==CSG_Node::DIFFERENCE && child_type==CSG_Node::UNION && !is_left)
   {
      DBG(cout << "Operation 2" << endl);

      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                other_child,
                                                                childs_left));
      tree->delete_and_replace_right(childs_right);
   }
   // (A-B)*C => (A*C)-B
   else if(root_type==CSG_Node::INTERSECTION && child_type==CSG_Node::DIFFERENCE)
   {
      DBG(cout << "Operation 3" << endl);

      tree->set_type(CSG_Node::DIFFERENCE);
      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::INTERSECTION,
                                                                childs_left,
                                                                other_child));
      tree->delete_and_replace_right(childs_right);
   }
   // A*(B*C) => (A*B)*C
   else if(root_type==CSG_Node::INTERSECTION && child_type==CSG_Node::INTERSECTION && !is_left)
   {
      DBG(cout << "Operation 7" << endl);

      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::INTERSECTION,
                                                                other_child,
                                                                childs_left));
      tree->delete_and_replace_right(childs_right);
   }
   // (A+B)*C => (A*C)+(B*C)
   else if(root_type==CSG_Node::INTERSECTION && child_type==CSG_Node::UNION)
   {
      DBG(cout << "Operation 1" << endl);

      tree->set_type(CSG_Node::UNION);
      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::INTERSECTION,
                                                                childs_left,
                                                                other_child));
      tree->delete_and_replace_right(CSG_Node::create_and_insert(CSG_Node::INTERSECTION,
                                                                 childs_right,
                                                                 new CSG_Node(*other_child)));
   }
   // (A+B)-C => (A-C)+(B-C)
   else if(root_type==CSG_Node::DIFFERENCE && child_type==CSG_Node::UNION && is_left)
   {
      DBG(cout << "Operation 4" << endl);

      tree->set_type(CSG_Node::UNION);
      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                childs_left,
                                                                other_child));
      tree->delete_and_replace_right(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                 childs_right,
                                                                 new CSG_Node(*other_child)));
   }
   // A-(B-C) => (A-B)+(A*C)
   else if(root_type==CSG_Node::DIFFERENCE && child_type==CSG_Node::DIFFERENCE && !is_left)
   {
      DBG(cout << "Operation 5" << endl);

      tree->set_type(CSG_Node::UNION);
      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                other_child,
                                                                childs_left));
      tree->delete_and_replace_right(CSG_Node::create_and_insert(CSG_Node::INTERSECTION,
                                                                 new CSG_Node(*other_child),
                                                                 childs_right));
   }
   // A-(B*C) => (A-B)+(A-C)
   else if(root_type==CSG_Node::DIFFERENCE && child_type==CSG_Node::INTERSECTION && !is_left)
   {
      DBG(cout << "Operation 6" << endl);

      tree->set_type(CSG_Node::UNION);
      tree->delete_and_replace_left(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                other_child,
                                                                childs_left));
      tree->delete_and_replace_right(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE,
                                                                 new CSG_Node(*other_child),
                                                                 childs_right));
   }
   else
   {
      delete childs_left;
      delete childs_right;
      delete other_child;
      return; // Prevent unnecessary normalization below
   }

   DBG(cout << "do_stuff done" << endl);
   //cout << "Efter: " << get_desc(tree) << endl;
}

static int is_left_fixable(const CSG_Node::CSG_Type root_type,
                           const CSG_Node::CSG_Type left_type)
{
   return ((root_type == CSG_Node::INTERSECTION && (left_type == CSG_Node::DIFFERENCE ||
                                                    left_type == CSG_Node::UNION)) ||
           root_type == CSG_Node::DIFFERENCE && left_type == CSG_Node::UNION);
}

static int is_right_fixable(const CSG_Node::CSG_Type root_type,
                            const CSG_Node::CSG_Type right_type)
{
   return ((root_type == CSG_Node::DIFFERENCE || root_type == CSG_Node::INTERSECTION) &&
           right_type != CSG_Node::PRIMITIVE);
}

static int get_fixable(const CSG_Node::CSG_Type root_type,
                       const CSG_Node::CSG_Type left_type,
                       const CSG_Node::CSG_Type right_type)
{
   if(is_left_fixable(root_type, left_type))
      return -1;

   if(is_right_fixable(root_type, right_type))
      return 1;

   return 0;
}

CSG_Node *normalize(const CSG_Node *tree)
{
   if(!tree)
      return NULL;

   normalize_counter++;

   DBG(cout << "normalize" << endl);

   CSG_Node *result_tree = new CSG_Node(*tree);

   if(result_tree->get_type() == CSG_Node::PRIMITIVE)
      return result_tree;

   int side = 0;

   do
   {
      while(result_tree->get_type() != CSG_Node::PRIMITIVE &&
            (side = get_fixable(result_tree->get_type(), result_tree->get_left()->get_type(),
                                result_tree->get_right()->get_type())))
      {
         do_stuff(result_tree, (side == -1));
      }
      result_tree->delete_and_replace_left(normalize(result_tree->get_left()));
   }
   while(get_fixable(result_tree->get_type(), result_tree->get_left()->get_type(),
                     result_tree->get_right()->get_type()) != 0);

   result_tree->delete_and_replace_right(normalize(result_tree->get_right()));

   DBG(cout << "normalize done" << endl);
   
   return result_tree;
}
