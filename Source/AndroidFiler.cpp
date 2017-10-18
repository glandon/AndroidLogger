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
#include "AndroidDevice.h"
#include "FilerPanel.h"

DeviceCb s_filer_callback;

bool onFilerStart(DeviceBase *device) {
    return true;
}

bool onFilerLoop(DeviceBase *device) {
    AndroidFiler *self = ((AndroidFiler*)device);
    FilerPanel *filer = (FilerPanel*)(self->mLoopArgs);
    std::string lines;
    std::string line;

    self->getLine(line);

    while(!line.empty()) {
        lines += line;
        self->getLine(line);
    }

    int size = lines.size();
	FileList *files = new FileList(lines, filer->currentDir());

    ::SendMessage(filer->GetHWND(), WM_UPDATE_FILER, (WPARAM)files, (LPARAM)size);
    return false;
}

void onFilerExit(DeviceBase *device) {
    AndroidFiler *self = ((AndroidFiler*)device);
    self->release();
}

bool AndroidFiler::listDir(const char* dir, void *args) {
    std::string cmd("shell:ls -l ");
    cmd += dir;
    mLoopArgs = args;

    if ( !connect() ) {
        return false;
    }

    mIsContinue = true;
    mSocket->SetTimeOut(0, 10);

    std::string msg = formAdbRequest(cmd.c_str());
    mSocket->SendLine(msg);
        
    if (checkResult(mSocket, ID_OKAY)) {
        startLoop();
        return true;
    }

    return false;
}

void AndroidFiler::release() {
    DeviceBase::release();
}

AndroidFiler::AndroidFiler():DeviceBase(){
    s_filer_callback.device = this;
    s_filer_callback.onExit = onFilerExit;
    s_filer_callback.onLoop = onFilerLoop;
    s_filer_callback.onStart= onFilerStart;
    setDeviceCb(&s_filer_callback);
}

AndroidFiler::~AndroidFiler() {
}