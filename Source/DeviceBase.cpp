#include "DeviceBase.h"
#include "Plugin.h"
#include <sstream>
#include <iomanip>
#include <process.h>
#include <sstream>
#include "AndroidDevice.h"

struct Args{
    DeviceBase *device;
    char *cmd;
};

DeviceBase::DeviceBase(): mSocket(nullptr) {
}
DeviceBase::~DeviceBase() {
	mListeners.clear();
    stop();
}

std::string DeviceBase::formAdbRequest(const char *req){
    std::string result;
    std::stringstream ss;

    ss<<std::hex<<std::setw(4)<<std::setfill('0')<<strlen(req);
    ss<<req;
    result = ss.str();
    return result;
}

void popupMessage(LPCTSTR message) {
    tstring msg(message);
    msg += L"\n\nPlease make sure: \n 1. Device connected to PC! \n 2. adb server started!";
    msg += L"\n\nNotice: adb server can start by \"adb start-server\" or \"adb shell\" etc.";
    ::MessageBox(nullptr, msg.c_str(), MSGBOX_TITLE, MB_OK);
}

bool checkResult(SocketClient *s, const char* expected) {
    BYTE* result = s->ReceiveBytes(4);
    if (result == nullptr) {
        return false;
    }
    int ret = memcmp(result, expected, 4);
    delete result;
    return ret == 0;
}

bool checkResult(const char *value, const char* expected) {
    if (value == nullptr) {
        return false;
    }
    bool ret = (value[0] == expected[0])
        && (value[1] == expected[1])
        && (value[2] == expected[2])
        && (value[3] == expected[3]);
    delete value;
    return ret;
}

/**
 * oops, c++ oo dynamic socket->send awayws fails
 * FIX: change to global method
 */
unsigned __stdcall DeviceBase::looper(void *arg) {
    DeviceCb *cb = (DeviceCb*)arg;
    cb->onStart(cb->device);
    while(cb->onLoop(cb->device));
    cb->onExit(cb->device);
    return 0;
};

void CALLBACK DeviceBase::waitStopTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime ) {
    DeviceBase *device = nullptr;

    if ( idEvent == mShellClient->mTimerId) {
        device = mShellClient;
    } else if ( idEvent == mFilerClient->mTimerId) {
        device = mFilerClient;
    } else if ( idEvent == mFileClient->mTimerId) {
        device = mFileClient;
    } else if ( idEvent == mMonkeyClient->mTimerId) {
        device = mMonkeyClient;
    }

    if ( device->mThread == (HANDLE)INVALID_HANDLE_VALUE ) {
        KillTimer(NULL, idEvent);
    }

    DWORD ret = ::WaitForSingleObject(device->mThread, 1);
    if ( ret == WAIT_TIMEOUT ) {
        return;
    }

    KillTimer(NULL, idEvent);

    device->mTimerId = 0;
    device->mThread = ((HANDLE)INVALID_HANDLE_VALUE);
    ::CloseHandle(device->mThread);
}

void DeviceBase::stop() {
    mIsContinue = false;
    mSocket->Close();

    if ( mThread == (HANDLE)INVALID_HANDLE_VALUE ) {
        release();
        return;
    }

    DWORD ret = ::WaitForSingleObject(mThread, 1);
    if ( ret == WAIT_TIMEOUT ) {
        mTimerId = ::SetTimer(NULL, 0, 10, waitStopTimer);
        return;
    }

     ::CloseHandle(mThread);
    mThread = (HANDLE)INVALID_HANDLE_VALUE;
}

void DeviceBase::asyncRun(void *arg) {
    Args *args = (Args*)arg;
    DeviceBase *device = args->device;

    char *cmd = args->cmd;
    char* serial = getDeviceSerial();
    SocketClient *client = getDeviceChannel(serial);
    delete serial;

    std::string adbcmd(cmd);
    delete cmd;

    device->executeRemoteCommand(adbcmd, device->getDeviceSerial());
}

void DeviceBase::executeAsyncCommand(DeviceBase* device, const char* cmd) {
    Args *args = new Args();
    args->device = device;
    args->cmd = SU::strdup(cmd);
    ::_beginthread(asyncRun, 0, args);
}

bool DeviceBase::executeRemoteCommand(std::string cmd, std::string serial, int timeout) {
    SocketClient* s = getDeviceChannel(serial.c_str());
    if (!s) {
        return false;
    }

    std::string output;
    bool ret = true;

    try {
        std::string adb = formAdbRequest(cmd.c_str());
        s->SendBytes(adb);
        ret = checkResult(s, ID_OKAY);

        // block then get and check
        int maxTry = 100;
        s->SetTimeOut(0, 10);
        output = s->ReceiveLine();
        while (output.empty() && maxTry ) {
            output = s->ReceiveLine();
            maxTry--;
        }
    }catch (...) {
        popupMessage(L"executeRemoteCommand failed!");
        ret = false;
    }

    delete s;
    return ret;
}

bool DeviceBase::isDeviceOn() {
    SocketClient detect("127.0.0.1", 5037);
    detect.SendLine("0012host:transport-any");
    return checkResult(&detect, ID_OKAY);
}

char* DeviceBase::getDeviceSerial() {
    SocketClient *s = nullptr;
    std::string cmd;
    std::string searil;

    try {
        s = new SocketClient("127.0.0.1", 5037);
        cmd = formAdbRequest("host:devices");
        s->SendLine(cmd.c_str());

        if (checkResult(s, ID_OKAY)) {
            searil = s->ReceiveLine();
            int offset =  searil.find_first_of('\t');
            searil = searil.substr(4, offset - 4);
        }
    } catch (...) {
        popupMessage(L"Get device serial failed!");
    }

    freeif(s);
    if (searil.empty()) {
        return nullptr;
    }

    return SU::strdup(searil.c_str());
}

SocketClient* DeviceBase::getDeviceChannel(LPCSTR device, int timeout) {
    SocketClient* s = new SocketClient("127.0.0.1", 5037);
    //client->SetTimeOut(timeout, timeout);
    
    BYTE *bytes = nullptr;
    bool ret = false;

    try {
        std::string cmd = "host";

        if (device == nullptr || device[0] == '\0') {
            cmd += ":transport-any";
        } else {
            cmd += ":transport:";
            cmd += device;
        }
        cmd = formAdbRequest(cmd.c_str());
        s->SendLine(cmd);
        ret = checkResult(s, ID_OKAY);
    } catch (...) {
        popupMessage(L"Get Device Serial Fail!");
    }

    if (!ret) {
        freeif(s);
    }
    return s;
}

void DeviceBase::startLoop() {
    unsigned ret;
    mThread = (HANDLE)_beginthreadex(0, 0, looper, (void*)&mDeviceCb, 0, &ret);
}

bool DeviceBase::connect() {
    if ( mSocket != nullptr ) {
        stop();
        return false;
    }
    
    mDeviceSerial = "";
    
    const char *serial = getDeviceSerial();
	if (serial == nullptr) {
		popupMessage(L"No device connected!");
		return false;
	}
    mDeviceSerial = serial;
    delete serial;

    mSocket = getDeviceChannel(mDeviceSerial.c_str());
    return mSocket != nullptr;
}

bool DeviceBase::sendLine(const char* cmd) {
    try{
        mSocket->SendLine(cmd);
        return true;
    } catch (...) {
        popupMessage(L"Send cmd fail!");
    }
    return false;
}

int DeviceBase::sendBytes(const char* bytes, int count) {
    try{
        return mSocket->SendBytes(bytes, count);
    } catch (...) {
        popupMessage(L"Send cmd fail!");
    }
    return 0;
}

int DeviceBase::getLine(std::string &line) {
    try{
        line = mSocket->ReceiveLine();
    } catch (...) {
        popupMessage(L"Read data fail!");
    }
    return line.size();
}

BYTE* DeviceBase::getBytes(int count) {
    try {
        return mSocket->ReceiveBytes(count);
    } catch (...) {
        ::MessageBox(nullptr, L"Receive data fail!", L"AndroidLogger", MB_OK);
    }
    return nullptr;
}

bool DeviceBase::createForward(int localPort, int remotePort, const char *serail) {
    SocketClient* s = nullptr;
    bool ret = false;

    try {
        std::string cmd("host-serial:");
        cmd += serail;
        cmd += ":forward:tcp:12345;tcp:12345";
        cmd = formAdbRequest(cmd.c_str());

        s = new SocketClient("127.0.0.1", 5037);
        s->SendLine(cmd.c_str());
        ret = checkResult(s, ID_OKAY);
    } catch (...) {
        ret = false;
    }

    freeif(s);
    if (!ret) {
        popupMessage(L"Forward port failed!");
    }
    
    return ret; 
}

/**
 * notify all windows regist to listen
 */
void DeviceBase::notifyListeners(UINT msg, WPARAM wparam, LPARAM lparam) {
	while(mListeners.size() > 0) {
		::PostMessage(mListeners.back(), msg, wparam, lparam);
		mListeners.pop_back();
	}
}

/**
 * release socket
 */
void DeviceBase::release() {
    freeif(mSocket);
    mLastClock = 0;
}