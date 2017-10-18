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

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "PluginInterface.h"
#include <string>

#define PLUGIN_NAME          "AndroidLogger"
#define PLUGIN_NAME_UNICODE  L"AndroidLogger"
#define LEXER_NAME           "AndroidLog Lexer"
#define MSGBOX_TITLE         L"AndroidLogger v1.2.6"

#define MAX_MENU_ITEMS       9
#define MAX_LOG_LINES        200000

extern bool appendLogToNpp(std::string &log);
extern NppData g_NppData;
extern FuncItem g_menuFuncs[MAX_MENU_ITEMS];

struct PluginSettings {
	TCHAR workingDirectory[MAX_PATH]; // default = D:\AndroidLogger
	TCHAR logcatLevel[2]; // default = V
	TCHAR logcatTag[64]; // default = *
	int autoShowCapture; // default = 1
};

void saveSettings(PluginSettings *settings);
void loadSettings(PluginSettings *settings);

const TCHAR* getWorkingDirectory();
bool isAutoShowCapture();

