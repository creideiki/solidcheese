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
 * \file persistence.cpp
 * Persistence functions.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <list>

#include "persistence.h"
#include "csg_tree.h"
#include "csg_object.h"
#include "debug.h"
#include "camera.h"

using namespace std;

//The escaped form is Base64. See RFC 2045, section 6.8, for details.
//Code borrowed from HTTP Request Mangler 1.6 by Karl-Johan Karlsson,
//which lives at http://creideiki.cjb.net/computers/computers.shtml

/*!
 * Table describing the mapping from 6-bit bit vectors to Base64 ASCII characters.
 */
const unsigned char base64_encoding[66] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/*!
 * Table describing the mapping from Base64 ASCII characters to 6-bit bit vectors. Positions
 * that do not correspond to a legal Base64 ASCII character have the value 255.
 */
const unsigned char reverse_base64_encoding[256] = { 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,
    63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255,   0, 255, 255, 255,   0,
     1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, 255,  26,  27,  28,  29,  30,  31,  32,
    33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

string escape(const string &orig)
{
   string output;
   int length = orig.size();
   int processed_length = 0;
   unsigned char bitgroup1, bitgroup2, bitgroup3, bitgroup4;

   while(processed_length < length)
   {
      switch(length - processed_length)
      {
         case 1:
            bitgroup1 =  orig[processed_length] >> 2;
            bitgroup2 = (orig[processed_length] << 4) & 0x30;
            bitgroup3 = 64;
            bitgroup4 = 64;
            break;

         case 2:
            bitgroup1  =  orig[processed_length] >> 2;
            bitgroup2  = (orig[processed_length] << 4) & 0x30;
            bitgroup2 |= (orig[++processed_length] >> 4) & 0x0F;
            bitgroup3  = (orig[processed_length] << 2) & 0x3C;
            bitgroup4  = 64;
            break;

         default:
            bitgroup1  =  orig[processed_length] >> 2;
            bitgroup2  = (orig[processed_length] << 4) & 0x30;
            bitgroup2 |= (orig[++processed_length] >> 4) & 0x0F;
            bitgroup3  = (orig[processed_length] << 2) & 0x3C;
            bitgroup3 |= (orig[++processed_length] >> 6) & 0x03;
            bitgroup4  =  orig[processed_length] & 0x3F;
            break;
      }
      processed_length++;

      output.push_back(base64_encoding[bitgroup1]);
      output.push_back(base64_encoding[bitgroup2]);
      output.push_back(base64_encoding[bitgroup3]);
      output.push_back(base64_encoding[bitgroup4]);
   }

   return output;
}

string unescape(const string &orig)
{
   string output;
   int length = orig.size();
   int processed_length = 0;
   unsigned char byte1, byte2, byte3;
   unsigned char bitgroups[4];
   int part_length = 0;
   int padding = 0;

   while(processed_length < length)
   {
      while(part_length < 4)
      {
         bitgroups[part_length] = orig[processed_length++];
         if(reverse_base64_encoding[bitgroups[part_length]] != (unsigned char)255)
         {
            if(bitgroups[part_length] == '=')
               padding++;

            part_length++;
         }

         //If we reach the end of the data while not having read
         //a full 4-byte group, something is wrong. Abort.
         if(processed_length > length)
         {
            cout << "\"" << orig << "\" is not a valid escaped string" << endl;
            return "Error";
         }
      }

      part_length = 0;

      bitgroups[0] = reverse_base64_encoding[bitgroups[0]];
      bitgroups[1] = reverse_base64_encoding[bitgroups[1]];
      bitgroups[2] = reverse_base64_encoding[bitgroups[2]];
      bitgroups[3] = reverse_base64_encoding[bitgroups[3]];

      byte1 =  (bitgroups[0] << 2) |
              ((bitgroups[1] >> 4) & 0x03);
      byte2 = ((bitgroups[1] << 4) & 0xF0) |
              ((bitgroups[2] >> 2) & 0x0F);
      byte3 = ((bitgroups[2] << 6) & 0xC0) |
               (bitgroups[3] & 0x3F);

      output.push_back(byte1);
      output.push_back(byte2);
      output.push_back(byte3);
   }

   return output;
}

int save(const string &filename, const CSG_Node *tree, const Camera &camera)
{
   DBG(cout << "Trying to save scene to \"" << filename << "\"" << endl);

   ofstream file(filename.c_str(), ios::trunc | ios::out);

   if(!file)
   {
      cout << "Unable to open \"" << filename << "\" for writing" << endl;
      return 1;
   }

   file << "Solid Cheese scene file" << endl;
   file << "Version 3" << endl << endl;

   string camera_repr = camera.stringify();
   DBG(cout << "Camera representation is: " << camera_repr << endl);
   file << camera_repr << endl << endl;

   string tree_repr = tree->stringify();
   DBG(cout << "Tree representation is:" << endl << tree_repr << endl);
   file << tree_repr << endl;
   DBG(cout << "Done saving" << endl);

   return 0;
}

CSG_Node *load(const string &filename, list<CSG_Object *> &objects, Camera &camera)
{
   DBG(cout << "Trying to load scene from \"" << filename << "\"" << endl);

   ifstream file(filename.c_str(), ios::in);

   if(!file)
   {
      cout << "Unable to open \"" << filename << "\" for reading" << endl;
      return NULL;
   }

   //Check header
   string s;
   getline(file, s);
   if(s != "Solid Cheese scene file")
   {
      cout << "\"" << filename << "\" is not a Solid Cheese scene file" << endl;
      return NULL;
   }

   //Check version number
   int version;
   file >> s >> version;
   if(s != "Version")
   {
      cout << "Could not find version information" << endl;
      return NULL;
   }
   if(version != 3)
   {
      cout << "File is version " << version << ", but we only support version " << 3 << endl;
      return NULL;
   }

   int num_tokens = 0;

   DBG(cout << "Everything OK so far, reading the camera" << endl);

   vector<string> camera_info;

   file >> s;
   if(s != "[")
   {
      cout << "Parse error: expected '[' (start of camera information), got '" << s << "'" << endl;
      return NULL;
   }
   num_tokens++;

   file >> s;
   if(s != "Camera")
   {
      cout << "Error in camera specification: expected 'Camera', got '" << s << "'" << endl;
      return NULL;
   }
   num_tokens++;

   DBG(cout << "Populating camera_info" << endl);

   while(s != "]")
   {
      camera_info.push_back(s);
      file >> s;
      num_tokens++;
   }

   DBG(cout << "Setting camera parameters" << endl);

   if(!camera.from_string(camera_info))
   {
      cout << "Error setting camera parameters" << endl;
      return NULL;
   }

   DBG(cout << "Reading the tree" << endl);
   int num_nodes = 0;
   stack<CSG_Node *> node_stack;
   CSG_Node *operand1;
   CSG_Node *operand2;

   //FIXME: If we get any parse errors after this point, the nodes on the stack are leaked.

   //The representation is a simple RPN, so all we need is a quick-and-dirty push-down parser.

   do
   {
      if(!(file >> s))
         break;

      num_tokens++;

      if(s == "[")
      {
         //New primitive
         DBG(cout << "Loading a primitive" << endl);

         vector<string> object_info;
         CSG_Object *new_object;
         string str;
         file >> str;
         num_tokens++;

         if(str == "Cube")
         {
            new_object = new CSG_Object_Cube;
         }
         else if(str == "Cylinder")
         {
            new_object = new CSG_Object_Cylinder;
         }
         else if(str == "Sphere")
         {
            new_object = new CSG_Object_Sphere;
         }
         else
         {
            cout << "\"" << s << "\" is not an object type" << endl;
            return NULL;
         }

         DBG(cout << "Populating object_info" << endl);

         while(str != "]")
         {
            object_info.push_back(str);
            file >> str;
            num_tokens++;
         }

         DBG(cout << "Setting object parameters" << endl);

         if(!new_object->from_string(object_info))
         {
            cout << "Error setting object parameters" << endl;
            return NULL;
         }

         node_stack.push(new CSG_Node(new_object));
      }
      else if(s == "*")
      {
         //Intersection
         DBG(cout << "Loading an intersection" << endl);

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get first operand of an intersection)" << endl;
            return NULL;
         }

         operand2 = node_stack.top();
         node_stack.pop();

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get first operand of an intersection)" << endl;
            delete operand2;
            return NULL;
         }

         operand1 = node_stack.top();
         node_stack.pop();

         node_stack.push(CSG_Node::create_and_insert(CSG_Node::INTERSECTION, operand1, operand2));
      }
      else if(s == "-")
      {
         //Difference
         DBG(cout << "Loading a difference" << endl);

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get first operand of a difference)" << endl;
            return NULL;
         }

         operand2 = node_stack.top();
         node_stack.pop();

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get second operand of a difference)" << endl;
            delete operand2;
            return NULL;
         }

         operand1 = node_stack.top();
         node_stack.pop();

         node_stack.push(CSG_Node::create_and_insert(CSG_Node::DIFFERENCE, operand1, operand2));
      }
      else if(s == "+")
      {
         //Union
         DBG(cout << "Loading a union" << endl);

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get first operand of a union)" << endl;
            return NULL;
         }
         operand2 = node_stack.top();
         node_stack.pop();

         if(node_stack.empty())
         {
            cout << "Parse error: Empty stack after " << num_tokens << " tokens " <<
               "(trying to get second operand of a union)" << endl;
            delete operand2;
            return NULL;
         }

         operand1 = node_stack.top();
         node_stack.pop();

         node_stack.push(CSG_Node::create_and_insert(CSG_Node::UNION, operand1, operand2));
      }
      else
      {
         cout << "Parse error: expected '[', '*', '-' or '+', got '" << s << "' (after " <<
            num_tokens << " tokens)" << endl;
         return NULL;
      }

      num_nodes++;

      operand1 = NULL;
      operand2 = NULL;
   }
   while(!file.eof());

   if(node_stack.size() != 1)
   {
      cout << "Parse error: Stack depth " << node_stack.size() <<
              " at end of input (should be 1)" << endl;
      return NULL;
   }

   DBG(cout << "Read " << num_tokens << " tokens, " << num_nodes << " nodes from file" << endl);

   operand1 = node_stack.top();
   node_stack.pop();
   return operand1;
}
