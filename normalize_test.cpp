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

#include <iostream>
#include <string>

#include "csg_tree.h"
#include "csg_object.h"

using namespace std;

extern string get_desc(const CSG_Node *tree);
extern CSG_Node *get_tree(string str);
extern CSG_Node *normalize(const CSG_Node *tree);

extern int normalize_counter;

string binary_string(int bits)
{
   string result;
   while (bits)
   {
      result += '0' + (bits & 1);
      bits >>= 1;
   }
   return result;
}

int count_leaves(const CSG_Node *tree)
{
   if (tree->get_type() == CSG_Node::PRIMITIVE)
      return 1;
   else
      return count_leaves(tree->get_left()) + count_leaves(tree->get_right());
}

bool tree_value(const CSG_Node *tree, int value)
{
   switch (tree->get_type())
   {
   case CSG_Node::PRIMITIVE:
   {
      int index = tree->get_object()->get_name()[0] - 'a';
      assert(index >= 0 && index < 30);
      return (value >> index) & 1;
   }
   case CSG_Node::UNION:
      return tree_value(tree->get_left(), value) || tree_value(tree->get_right(), value);
   case CSG_Node::DIFFERENCE:
      return tree_value(tree->get_left(), value) && !tree_value(tree->get_right(), value);
   case CSG_Node::INTERSECTION:
      return tree_value(tree->get_left(), value) && tree_value(tree->get_right(), value);
   default:
      assert(!"Händer inte");
      return false;
   }
}

void compare_trees(const CSG_Node *tree1, const CSG_Node *tree2)
{
   int count = count_leaves(tree1);
   assert(count < 30);

   for(int values = 0; values < (1 << count); ++values)
   {
      bool v1 = tree_value(tree1, values);
      bool v2 = tree_value(tree2, values);
      
      if (v1 != v2)
         cout << "Fel för tilldelning " << binary_string(values) << "!" << endl;
   }
   cout << "Klar med sanningstabelltest." << endl;
}

bool is_simon_normal(const CSG_Node *tree)
{
   switch(tree->get_type())
   {
   case CSG_Node::PRIMITIVE:
      return true;
   case CSG_Node::UNION:
      return is_simon_normal(tree->get_left()) && is_simon_normal(tree->get_right());
   case CSG_Node::DIFFERENCE:
      return ((tree->get_right()->get_type() == CSG_Node::PRIMITIVE) &&
              (tree->get_left()->get_type() != CSG_Node::UNION) &&
              is_simon_normal(tree->get_left()));
   case CSG_Node::INTERSECTION:
      return ((tree->get_right()->get_type() == CSG_Node::PRIMITIVE) &&
              ((tree->get_left()->get_type() == CSG_Node::INTERSECTION) ||
               (tree->get_left()->get_type() == CSG_Node::PRIMITIVE)) &&
              is_simon_normal(tree->get_left()));
   default:
      return false;
   }
}

int main()
{
   string before;

   cout << "Testa (omvänd polsk notation, avsluta med \".\")" << endl;

   while((cin >> before, before)!=".")
   {
      normalize_counter = 0;
      CSG_Node *tree=get_tree(before);
      CSG_Node *ntree = normalize(tree);
      cout << normalize_counter << endl << get_desc(ntree) << endl << endl;

      cout << "Testar..." << endl;
      compare_trees(tree, ntree);
      cout << "Simon är " << (is_simon_normal(ntree)?"":"inte ") << "normal!" << endl;

      delete tree;
      delete ntree;
   }
}
