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
#include "DeviceBase.h"
#include <fstream>
#include <time.h>
#include <process.h>

DeviceCb s_file_callback;

int swap32bitFromArray(const char *value, int offset){
    int v = 0;
    v |= value[offset] & 0xFF;
    v |= (value[(offset + 1)] & 0xFF) << 8;
    v |= (value[(offset + 2)] & 0xFF) << 16;
    v |= (value[(offset + 3)] & 0xFF) << 24;

    return v;
}

void swap32bitsToArray(int value, char *dest, int offset){
    dest[offset] = ((char)(value & 0xFF));
    dest[(offset + 1)] = ((char)((value & 0xFF00) >> 8));
    dest[(offset + 2)] = ((char)((value & 0xFF0000) >> 16));
    dest[(offset + 3)] = ((char)((value & 0xFF000000) >> 24));
}

long currentTimeMillis() {
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    return sys.wMilliseconds;
}


char* createReq(const char* command, int value) {
    char* buf = new char[8];
    memcpy(buf, command, strlen(command));
    swap32bitsToArray(value, buf, 4);
    return buf;
}

/**
 * RECV15/sdcard/dzx.txt
 */
char* createFileReq(const char* command, const char *path){
    int length = strlen(path); // maybe unicode error
    char* buf = new char[8 + length];

    memcpy(buf, command, strlen(command));
    swap32bitsToArray(length, buf, 4);
    memcpy(buf+8, path, length);
    return buf;
}

/**
 * SEND19/sdcard/dzx.txt,420
 */
char* createSendFileReq(const char* command, const char *path){
    int pathLen = strlen(path); // maybe unicode error
    int modeLen = 4;
    char* buf = new char[8 + pathLen + 4];
    //mode &= 0x1FF

    memcpy(buf, command, strlen(command));
    swap32bitsToArray(pathLen+modeLen, buf, 4);
    memcpy(buf, command, strlen(command));
    memcpy(buf+8, path, pathLen);
    memcpy(buf+8+pathLen, ",420", modeLen);
    return buf;
}

bool onPushLoop2() {
    AndroidFile *self = ((AndroidFile*)DeviceBase::mFileClient);
    FilerPanel *filer = (FilerPanel*)self->mLoopArgs;
    BYTE* reply = nullptr;
    
    tstring l = self->mLocalPath;
    std::ifstream fin(l, std::ios::binary);

    char *buffer = new char [65536+8];
    memcpy(buffer, "DATA", 4);

    fin.read(buffer+8, 65536);
    int count = fin.gcount();

    do {
        swap32bitsToArray(count, buffer, 4);
        int len = self->sendBytes(buffer, count+8);

        if (len != (count+8)) {
            popupMessage(L"push file error!");
            return false;
        }
        ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, (WPARAM)count, 0);

        fin.read(buffer+8, 65536);
        count = fin.gcount();
    } while (count > 0);

    fin.close();
    
    int tm = time(0);
    char* msg = createReq("DONE", tm);
    self->sendBytes(msg, 8);
    
    reply = self->getBytes(8);
    if (!(reply[0] == 'O' && reply[1] == 'K' && reply[2] == 'A' && reply[3] == 'Y')) {
        reply = self->getBytes(8);
        popupMessage(L"push file error?!");
    }
    freeif(reply);

    ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, 0, 0);
    delete[] buffer;
    return false;
}

bool AndroidFile::pullFile(const char* remote, const TCHAR* local, void* args) {
    if ( !connect() ) {
        return false;
    }

    mLoopArgs = args;
    mIsPull = true;
    mIsContinue = true;
    mSocket->SetTimeOut(0, 100);

    TCHAR* pLocal = SU::DupString(local);
    mLocalPath = pLocal;
    delete pLocal;

    BYTE* reply = nullptr;
    char* buf = nullptr;
    bool  ret = true;

    try {
        std::string msg = formAdbRequest("sync:");
        mSocket->SendLine(msg);
        if (!checkResult(mSocket, ID_OKAY)) {
            popupMessage(L"Start sync service failed!");
        }
        
        int len = strlen(remote) + 8;
        buf = createFileReq(ID_STAT, remote);

        mSocket->SendBytes(buf, len);
        freeif(buf);

        reply = getBytes(16);
        freeif(reply);
        
        buf = createFileReq(ID_RECV, remote);
        mSocket->SendBytes(buf, len);
        freeif(buf);

        startLoop();
    } catch (...) {
        ret = false;
        popupMessage(L"Unhandled exception!");
    }

    freeif(reply);
    freeif(buf);
    return ret;
}

bool AndroidFile::pushFile(DeviceCb* cb, const char* remote, const TCHAR* path, void* args) {
    if ( !connect() ) {
        return false;
    }

    _cb = *cb;
    mLoopArgs = args;
    mIsContinue = true;
    mSocket->SetTimeOut(5000, 1000);

    mLocalPath = SU::DupString(path);

    BYTE* reply = nullptr;
    char* buf = nullptr;

    try {
        std::string msg = formAdbRequest("sync:");
        mSocket->SendLine(msg);
        reply = getBytes(4);
        freeif(reply);
        
        int len = strlen(remote) + 8 + 4;
        buf = createSendFileReq("SEND", remote);
        mSocket->SendBytes(buf, len);
        freeif(buf);

        startLoop();
    } catch (...) {
        //::MessageBox(nullptr, L"Unhandled exception!", MSGBOX_TITLE, MB_OK);
        return false;
    }

    freeif(reply);
    freeif(buf);
    return true;
}

bool AndroidFile::pushFile(const char* remote, const TCHAR* local, void* args) {
    if ( !connect() ) {
        return false;
    }

    mLoopArgs = args;
    mIsPull = false;
    mIsContinue = true;
    mSocket->SetTimeOut(5000, 10000);
    
    TCHAR* pLocal = SU::DupString(local);
    mLocalPath = pLocal;
    delete pLocal;

    char* buf = nullptr;
    bool ret = true;

    try {
        std::string msg = formAdbRequest("sync:");
        mSocket->SendLine(msg);
        if (!checkResult(mSocket, ID_OKAY)) {
            popupMessage(L"Start sync service failed!");
        }

        int len = strlen(remote) + 8 + 4;
        buf = createSendFileReq(ID_SEND, remote);
        mSocket->SendBytes(buf, len);
        freeif(buf);

        startLoop();
    } catch (...) {
        popupMessage(L"Unhandled exception!");
        ret = false;
    }

    freeif(buf);
    return ret;
}

bool onFileStart(DeviceBase *device) {
    return true;
}

void onFileExit(DeviceBase *device) {
    device->release();
}

bool onFilePullLoop(DeviceBase *device) {
    AndroidFile *self = ((AndroidFile*)device);
    FilerPanel *filer = (FilerPanel*)self->mLoopArgs;
    BYTE* reply = nullptr;
    
    std::ofstream fout(self->mLocalPath, std::ios::binary);

    do {
        reply = self->getBytes(8);
        
        if (!memcmp(reply, ID_DONE, 4)) {
            break;
        }

        if (memcmp(reply, ID_DATA, 4)) {
            break;
        }

        int length = swap32bitFromArray((char*)reply, 4);
        if (length > 65536) {
            break;
        }
        delete reply;

        reply = self->getBytes(length);
        fout.write((char*)reply, length);
        delete reply;

        ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, (WPARAM)length, 0);
    } while (1);

    ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, 0, 0);
    fout.close();
    return false;
}

bool onFilePushLoop(DeviceBase *device) {
    AndroidFile *self = ((AndroidFile*)DeviceBase::mFileClient);
    FilerPanel *filer = (FilerPanel*)self->mLoopArgs;

    tstring l = self->mLocalPath;
    std::ifstream fin(self->mLocalPath, std::ios::binary);

    char *buffer = new char[65536+8];
    memcpy(buffer, ID_DATA, 4);

    fin.read(buffer+8, 65536);
    int count = fin.gcount();

    do {
        swap32bitsToArray(count, buffer, 4);
        int len = self->sendBytes(buffer, count+8);

        if (len != (count+8)) {
            popupMessage(L"push file error!");
            return false;
        }

        ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, (WPARAM)count, 1);

        fin.read(buffer+8, 65536);
        count = fin.gcount();
    } while (count > 0);

    fin.close();
    
    int tm = time(0);
    char* msg = createReq(ID_DONE, tm);
    self->sendBytes(msg, 8);
    
    if (!checkResult(self->mSocket, ID_OKAY)) {
        //reply = _file.getBytes(8);
        popupMessage(L"push file error!");
    }

    ::PostMessage(filer->GetHWND(), WM_FILE_PROGRESS, 0, 1);
    delete[] buffer;
    return false;
}

bool onFileLoop(DeviceBase *device) {
    AndroidFile *self = ((AndroidFile*)device);
    if (self->mIsPull) {
        return onFilePullLoop(device);
    }
    return onFilePushLoop(device);
}

bool onFilePullStart(DeviceBase *device) {
    return true;
}

void AndroidFile::release() {
    DeviceBase::release();
}

AndroidFile::AndroidFile():DeviceBase(){
    s_file_callback.device = this;
    s_file_callback.onExit = onFileExit;
    s_file_callback.onLoop = onFileLoop;
    s_file_callback.onStart= onFileStart;
    setDeviceCb(&s_file_callback);
}

AndroidFile::~AndroidFile() {
}