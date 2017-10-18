/*
  AndroidLogger: Android Logger plugin for Notepad++
  Copyright (C) 2015 Simbaba at Najing <zhaoxi.du@gmail.com>

  ******************************************************
  Thanks for GedcomLexer & NppFtp plugin source code.
  ******************************************************
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef _xx_debug_trace_h_
#define _xx_debug_trace_h_

extern void _xxDebugTrace(LPCTSTR lpszFormat, ...);

#ifdef _DEBUG
#define DebugTrace _xxDebugTrace
#else
#define DebugTrace
#endif

#endif /* _xx_debug_trace_h_ */

