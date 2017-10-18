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

DeviceCb s_monkey_callback;

int getMonkeyCmd(std::string &cmd){
    int type = 0;
    std::string monkey;
    std::string args;
    int pos1,pos2;

    if(SU::strStartWith(cmd, "PRESS")){
        monkey = "press ";
        pos1 = cmd.find("name") + 7;
        pos2 = cmd.find("'", pos1);
        args += cmd.substr(pos1, pos2-pos1);
        args += " ";
        
        pos1 = cmd.find("type", pos2) + 7;
        pos2 = cmd.find("'", pos1);
        std::string type = cmd.substr(pos1, pos2-pos1);
        if ( type == "downAndUp") {
            type = "";
        } else {
            monkey = "key";
            args += type;
        }
         
    } else if(SU::strStartWith(cmd, "TOUCH")){
        monkey = "touch ";
        type =1;
        pos1 = cmd.find("TOUCH") + 7;
        pos2 = cmd.find("'", pos1);
        args += cmd.substr(pos1, pos2-pos1);
        args += " ";

        pos1 = cmd.find("x") + 3;
        pos2 = cmd.find(",", pos1);
        args += cmd.substr(pos1, pos2-pos1);
        args += " ";

        pos1 = cmd.find("y", pos2) + 3;
        pos2 = cmd.find(",", pos1);
        args += cmd.substr(pos1, pos2-pos1);

        pos1 = cmd.find("type", pos2) + 7;
        pos2 = cmd.find("'", pos1);
        std::string type = cmd.substr(pos1, pos2-pos1);
        if ( type == "downAndUp") {
            monkey = "tap";
            type = "";
        }
    } else if(SU::strStartWith(cmd, "TYPE")){
        monkey = "type ";
        type =2;
        pos1 = cmd.find("message") + 10;
        pos2 = cmd.find("'", pos1);
        args += cmd.substr(pos1, pos2-pos1);
    } else if(SU::strStartWith(cmd, "WAIT")){
        monkey = "sleep ";
        pos1 = cmd.find("seconds") + 9;
        pos2 = cmd.find(",", pos1);
        int ms = atof(cmd.substr(pos1, pos2-pos1).c_str())*1000;
        Sleep(ms);
        //char buf[11] = {0};
        //itoa(ms, buf, 10);
        //args += buf;
        return -1;
    } else if(SU::strStartWith(cmd, "DRAG")){
        return -1;
    } else {
        return -1;
    }

    monkey += args;
    cmd = monkey;
    return type;
}

//press KEYCODE_HOME\n
bool AndroidMonkey::sendMonkeyCmd(const char* monkey) {
    mSocket->SendLine(monkey);
    int max = 100;
    std::string line = mSocket->ReceiveLine(); // debug
    return true;
}

bool AndroidMonkey::monkey(std::vector<std::string> &cmds) {
    if ( !connectMonkey() ) {
        return false;
    }

    mMonkeyCmds  = cmds;
    startLoop();
    return true;
}

void AndroidMonkey::quitMonkey() {
    mSocket->SendLine("done");
}

bool onMonkeyStart(DeviceBase *device) {
    Sleep(1000);
    //sendMonkeyCmd("press HOME");
    return true;
}

void onMonkeyExit(DeviceBase *device) {
    AndroidMonkey *self = ((AndroidMonkey*)device);
    self->quitMonkey();
    self->release();
}

bool onMonkeyLoop(DeviceBase *device) {
    AndroidMonkey *self = ((AndroidMonkey*)device);
    int size = self->mMonkeyCmds.size();

    for (int i=0; i < size; i++) {
        std::string cmd = self->mMonkeyCmds[i];
        if (getMonkeyCmd(cmd) != -1) {
            self->sendMonkeyCmd(cmd.c_str());
        }
    }

    return false;
}

bool AndroidMonkey::connectMonkey() {
    BYTE* bytes = nullptr;

    mDeviceSerial = "";
    
    const char *serial = getDeviceSerial();
    if (serial == nullptr){
        return false;
    }

    mDeviceSerial = serial;
    delete serial;

    // 1. forward host port to device port
    if (!createForward(12345, 12345, mDeviceSerial.c_str())) {
        popupMessage(L"Fail to forward monkey port=12345!");
        return false;
    }

    // 2. set device monkey port to 12345
    executeAsyncCommand(this, "shell:monkey --port 12345");

    // 3. connect to host 12345
    mSocket = new SocketClient("127.0.0.1", 12345);
    return true;
}

void AndroidMonkey::release() {
    quitMonkey();
    DeviceBase::release();
}

AndroidMonkey::AndroidMonkey():DeviceBase(){
    s_monkey_callback.device = this;
    s_monkey_callback.onExit = onMonkeyExit;
    s_monkey_callback.onLoop = onMonkeyLoop;
    s_monkey_callback.onStart= onMonkeyStart;
    setDeviceCb(&s_monkey_callback);
}

AndroidMonkey::~AndroidMonkey() {
}