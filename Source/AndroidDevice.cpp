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
#include "DeviceBase.h"
#include "ImageUtil.h"
#include "Plugin.h"
#include "PluginInterface.h"
#include "FilerPanel.h"
#include <iomanip>


HANDLE g_hAsycHandle;
DeviceBase* DeviceBase::mCaptureClient = new AndroidCapture();
DeviceBase* DeviceBase::mFileClient = new AndroidFile();
DeviceBase* DeviceBase::mMonkeyClient = new AndroidMonkey();
DeviceBase* DeviceBase::mShellClient = new AndroidShell();
DeviceBase* DeviceBase::mFilerClient = new AndroidFiler();

void popupMessage(tstring message) {
    message += L"\n\nPlease make sure: \n 1. Device connected to PC! \n 2. adb server started!";
    message += L"\n\nNotice: adb server can start by \"adb start-server\" or \"adb shell\" etc.";
    //TCHAR tmessage[200] = {0};
    //MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, tmessage, 200);
    ::MessageBox(nullptr, message.c_str(), MSGBOX_TITLE, MB_OK);
}

void AndroidDevice::closeAllClients(){
    delete DeviceBase::mShellClient;
    delete DeviceBase::mCaptureClient;
    delete DeviceBase::mFileClient;
    delete DeviceBase::mFilerClient;
    delete DeviceBase::mMonkeyClient;
}

AndroidDevice::AndroidDevice(){
}

AndroidDevice::~AndroidDevice() {
    closeAllClients();
}

/**
 * static method exported to user
 */
bool AndroidDevice::shellCmd(std::vector<std::string> &cmds) {
    return static_cast<AndroidShell*>(DeviceBase::mShellClient)->shellCmd(cmds);
}

/**
 * static method exported to user
 */
bool AndroidDevice::shellCmd(std::vector<std::string> &cmds, HWND hwnd) {
	DeviceBase::mShellClient->mListeners.push_back(hwnd);
    return static_cast<AndroidShell*>(DeviceBase::mShellClient)->shellCmd(cmds);
}

/**
 * static method exported to user
 */
bool AndroidDevice::capture() {
    return static_cast<AndroidCapture*>(DeviceBase::mCaptureClient)->capture();
}

/**
 * static method exported to user
 */
bool AndroidDevice::lsDir(const char* dir, void *args) {
    return static_cast<AndroidFiler*>(DeviceBase::mFilerClient)->listDir(dir, args);
}

/**
 * static method exported to user
 */
bool AndroidDevice::pushFile(const char* remote, const TCHAR* path, void* args) {
    return static_cast<AndroidFile*>(DeviceBase::mFileClient)->pushFile(remote, path, args);
}

/**
 * static method exported to user
 */
bool AndroidDevice::pullFile(const char* remote, const TCHAR* path, void* args) {
    return static_cast<AndroidFile*>(DeviceBase::mFileClient)->pullFile(remote, path, args);
}

bool AndroidDevice::monkey(std::vector<std::string> &cmds) {
    return static_cast<AndroidMonkey*>(DeviceBase::mMonkeyClient)->monkey(cmds);
}
