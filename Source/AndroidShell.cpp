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
#include "DeviceBase.h"
#include "AndroidDevice.h"
#include "FilerPanel.h"
#include "Plugin.h"
#include <time.h>

DeviceCb s_shell_callback;

void AndroidShell::sendShellCmds() {
    int count = mShellCmds.size();

    for ( int i = 0; i < count; i++ ) {
        if(!mShellCmds[i].find("logcat")) {
            _hasLogcat = true;
        }
        mSocket->SendLine(mShellCmds[i]);
    }

    mSocket->SendLine("\0");

    /** just drop **/
    for ( int i = 0; i < count; i++ ) {
        mSocket->ReceiveLine();
    }

    //appendLogToNpp(std::string("\n"));
    Sleep(100);
}

bool AndroidShell::shellCmd(const char* cmd) {
    if ( !connect() ) {
		notifyListeners(WM_FILE_SHELLCMD, 0, 0);
        return false;
    }

	mLinesCount = 0;
    mIsContinue = true;
    mSocket->SetTimeOut(0, 10);

    std::string msg = formAdbRequest(cmd);
    mSocket->SendLine(msg);
        
    if (checkResult(mSocket, ID_OKAY)) {
        startLoop();
        return true;
    }
    return false;
}

bool AndroidShell::shellCmd(std::vector<std::string> &cmds) {
    mShellCmds = cmds;
    return shellCmd("shell:");
}

/**
 * release socket
 */
void AndroidShell::release() {
    DeviceBase::release();
    freeif(mSocket);
    mShellCmds.clear();
    mLogLines.clear();
    mLinesCount = 0;
    mLastClock = 0;
    mIsContinue = false;
}

bool onShellStart(DeviceBase *device) {
    AndroidShell *self = ((AndroidShell*)device);
    self->sendShellCmds();
    self->mLastClock = clock();
    return true;
}

void onShellExit(DeviceBase *device) {
    AndroidShell *self = ((AndroidShell*)device);
    appendLogToNpp(std::string("\n---- exit ----"));
    self->release();
    ::PostMessage(g_NppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[0]._cmdID, (LPARAM)false);
    ::PostMessage(g_NppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[1]._cmdID, (LPARAM)false);
    ::PostMessage(g_NppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)g_menuFuncs[2]._cmdID, (LPARAM)false);

	device->notifyListeners(WM_FILE_SHELLCMD, 0, 0);
}

//shell@android:/sdcard $
bool onShellLoop(DeviceBase *device) {
    AndroidShell *self = ((AndroidShell*)device);
    double end, cost;
    std::string line;
    int size = self->getLine(line);

    //shell@andoroid: /\s$\s\r\r\n
	if (size > 4 && (line[size-5] == '$' || line[size-5] == '#') && (line[size-3] == '\r' || line[size-3] == '\n')){
        self->mLogLines += line;
        appendLogToNpp(self->mLogLines);
        return false;
    }

    end = clock();
    cost = end - self->mLastClock;

    /** timeout, device lost?! **/
    size += self->mLogLines.size();
    if ( size == 0 ) {

        /** try again **/
        if (cost < 200) {
            return true;
        }

        /** device lost ? **/
        if ( !self->isDeviceOn() ) { 
            return false;
        }
    }

    self->mLogLines += line;
    self->mLinesCount += 1;

    /** interval too short **/
    if ( cost < 20 ) {
        return self->mIsContinue;
    }

    /** current buffer maybe hide! **/
    if ( !appendLogToNpp(self->mLogLines) ){
        return false;
    }

    /** too many lines ! **/
    if ( self->mLinesCount > 200000 ) {
        return false;
    }

    self->mLogLines.clear();
    self->mLastClock = clock();

    return self->mIsContinue;
}

AndroidShell::AndroidShell():DeviceBase(){
    s_shell_callback.device = this;
    s_shell_callback.onExit = onShellExit;
    s_shell_callback.onLoop = onShellLoop;
    s_shell_callback.onStart= onShellStart;
    setDeviceCb(&s_shell_callback);
}

AndroidShell::~AndroidShell() {
}