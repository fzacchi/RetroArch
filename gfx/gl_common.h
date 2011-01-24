/*  SSNES - A Super Nintendo Entertainment System (SNES) Emulator frontend for libsnes.
 *  Copyright (C) 2010-2011 - Hans-Kristian Arntzen
 *
 *  Some code herein may be based on code found in BSNES.
 * 
 *  SSNES is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  SSNES is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with SSNES.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GL_COMMON_H
#define __GL_COMMON_H

#include "general.h"

static inline bool gl_check_error(void)
{
   int error = glGetError();
   switch (error)
   {
      case GL_INVALID_ENUM:
         SSNES_ERR("GL: Invalid enum.\n");
         break;
      case GL_INVALID_VALUE:
         SSNES_ERR("GL: Invalid value.\n");
         break;
      case GL_INVALID_OPERATION:
         SSNES_ERR("GL: Invalid operation.\n");
         break;
      case GL_STACK_OVERFLOW:
         SSNES_ERR("GL: Stack overflow. (wtf)\n");
         break;
      case GL_STACK_UNDERFLOW:
         SSNES_ERR("GL: Stack underflow. (:v)\n");
         break;
      case GL_OUT_OF_MEMORY:
         SSNES_ERR("GL: Out of memory. Harhar.\n");
         break;
      case GL_TABLE_TOO_LARGE:
         SSNES_ERR("GL: Table too large. Big tables scare you! :(\n");
         break;
      case GL_NO_ERROR:
         return true;
         break;
      default:
         SSNES_ERR("Non specified error :v\n");
   }

   return false;
}

#endif
