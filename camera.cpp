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
 * \file camera.cpp
 * Camera implementation.
 */

#include <string>
#include <iostream>
#include <sstream>
#include "camera.h"
#include "matrix.h"
#include "debug.h"

using namespace std;

string Camera::stringify() const
{
   ostringstream representation;
   representation << "[ Camera " << center_x << " " << center_y << " " << center_z <<
                     " " << radius << " " << rotation_matrix.stringify() << " ]";
   return representation.str();
}

bool Camera::from_string(const vector<string> &parameters)
{
   if(parameters.size() != 23)
   {
      cout << "Error: Can't define a camera with " << parameters.size() <<
              " values (wants 23)" << endl;

      for(unsigned int i = 0; i < parameters.size(); i++)
         DBG(cout << parameters[i] << endl);

      return false;
   }

   if(parameters[0] != "Camera")
   {
      cout << "Error: Not a camera specification ('" << parameters[0] << "')" << endl;
      return false;
   }

   istringstream iss;

   iss.str(parameters[1]);
   if(!(iss >> center_x))
   {
      cout << "Error reading center_x: '" << parameters[1] << "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[2]);
   if(!(iss >> center_y))
   {
      cout << "Error reading center_y: '" << parameters[2] << "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[3]);
   if(!(iss >> center_z))
   {
      cout << "Error reading center_z: '" << parameters[3] << "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   iss.str(parameters[4]);
   if(!(iss >> radius))
   {
      cout << "Error reading radius: '" << parameters[4] << "' is not a GLfloat" << endl;
      return false;
   }
   iss.clear();

   if((parameters[5]  != "{") ||
      (parameters[22] != "}"))
   {
      cout << "Error: Can't find rotation matrix" << endl;
      return false;
   }

   vector<string>::const_iterator first = parameters.begin();
   vector<string>::const_iterator last = parameters.end();
   advance(first, 6);
   advance(last, -1);

   vector<string> rotation_data(first, last);

   if(!rotation_matrix.from_string(rotation_data))
   {
      cout << "Error reading rotation matrix" << endl;
      return false;
   }

   return true;
}
