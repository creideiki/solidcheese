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

#include <GL/glut.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
   glutCreateWindow("glinfo");

   string vendor   = (const char *)glGetString(GL_VENDOR);
   string renderer = (const char *)glGetString(GL_RENDERER);
   string version  = (const char *)glGetString(GL_VERSION);
   string extstr   = (const char *)glGetString(GL_EXTENSIONS);

   vector<string> extensions;
   unsigned int p, q = 0;
   do
   {
      p = extstr.find(' ', q);
      extensions.push_back(extstr.substr(q, p - q));
      q = p + 1;
   } while (p != string::npos);

   cout << "Vendor: " << vendor << endl
        << "Renderer: " << renderer << endl
        << "Version: " << version << endl << endl;

   GLint r, g, b, depth, stencil;

   glGetIntegerv(GL_RED_BITS,     &r);
   glGetIntegerv(GL_GREEN_BITS,   &g);
   glGetIntegerv(GL_BLUE_BITS,    &b);
   glGetIntegerv(GL_DEPTH_BITS,   &depth);
   glGetIntegerv(GL_STENCIL_BITS, &stencil);

   cout << "R/G/B/Depth/Stencil depth: " << r << " " << g << " "
        << b << " " << depth << " " << stencil << endl << endl;

   cout << "Extensions:" << endl;
   for(unsigned int i = 0; i < extensions.size(); ++i)
   {
      cout << "   " << extensions[i] << endl;
   }

   return 0;
}
