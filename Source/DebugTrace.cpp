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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "tchar.h"

#define DT_BUFFER_SIZE 512

/**
 *  @brief Sends printf-like formatted message to debug console
 *  
 *  @param [in] lpszFormat Printf-like format string
 *  @param [in] ...        Parameters to format string
 *  @return None
 *  
 *  @details A message longer than DT_BUFFER_SIZE-1 bytes will be truncated.
 */
void _xxDebugTrace(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    TCHAR szBuffer[DT_BUFFER_SIZE];
    int nBuf = _vsntprintf_s(szBuffer, DT_BUFFER_SIZE-1, _TRUNCATE, lpszFormat, args);
    ::OutputDebugString(szBuffer);
    va_end(args);
}
