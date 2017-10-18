#include "FileObject.h"
#include "StringUtils.h"
#include <tchar.h>

#define MB (1<<20)
#define KB (1<<10)

enum {
    IS_FOLDER,
    IS_APK,
    IS_FILE,
    IS_IMAGE,
    IS_MUSIC,
	IS_ZIP
};

FileObject::FileObject(tstring ls_l, tstring &parentDir) {
    std::wregex lsRegex(LS_REGEX);
    std::match_results<tstring::const_iterator> result;
    mIsValid = regex_match(ls_l, result, lsRegex);

    mDetail = ls_l;
	mParentDir = parentDir;

    if ( !mIsValid ) {
		mName = L"Permission denied!";
        return;
    }
    
    mPerm  = result[1];
    mOwner = result[2];
    mGroup = result[3];
    mDate  = result[5];
    mTime  = result[6];
    mName  = result[7];

    if (mPerm[0] == L'd') {
        mFileType  = IS_FOLDER;
        return;
    }

    if (mPerm[0] == L'l') {
        mFileType = IS_FILE;
        return;
    }

	mFileType  = IS_FILE;

    TCHAR str1[32] = {0};
    TCHAR str2[32] = {0};
    int size = std::stoi(result[4]);
    
    mIntSize = size;

    if (size >= MB) {
        _itow(size/MB, str1, 10);
        mSize  = str1;

        int dot = 10*size%MB/MB;
        if ( dot > 0 ) {
            mSize += L".";
            _itow(dot, str2, 10);
            mSize += str2;
        }
        mSize += L"M";
    } else if (size >= KB) {
        _itow(size/KB, str1, 10);
        mSize  = str1;
        int dot = 10*size%KB/KB;
        if ( dot > 0 ) {
            mSize += L".";
            _itow(dot, str2, 10);
            mSize += str2;
        }
        mSize += L"K";
    } else {
        _itow(size, str1, 10);
        mSize += str1;
    }

    int len = mName.size();
    int pos = mName.find_last_of(L'.');

    if ( pos != std::string::npos) {
        tstring ext = mName.substr(pos, len-pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if ( ext == L".apk" ) {
            mFileType = IS_APK;
        } else if ( ext == L".jpg" || ext == L".png" || ext == L".gif" || ext == L".bmp") {
            mFileType = IS_IMAGE;
        } else if ( ext == L".mp3" || ext == L".mp4" || ext == L".wma" || ext == L".ogg" || ext == L".wav") {
            mFileType = IS_MUSIC;
        }else if ( ext == L".zip" || ext == L".rar" || ext == L".jar" || ext == L".7z" || ext == L".tar" || ext == L".z" || ext == L".iso") {
			mFileType = IS_ZIP;
		} else {
            mFileType = IS_FILE;
        }
    }
}

FileList::FileList(std::string &lines, tstring &parentDir){
    BuildFileListObjects(lines, parentDir);
}

FileList::~FileList() { 
    FreeFileListObjects();
}

void FileList::BuildFileListObjects(std::string &lines, tstring &parentDir) {
    tstring wlines = SU::Utf8ToTChar(lines.c_str());
    size_t last = 0;
    size_t index=wlines.find_first_of(L"\r\n", last);

    while (index != std::string::npos) {
        FileObject *fo = new FileObject(wlines.substr(last, index - last), parentDir);
        mFiles.push_back(fo);
        last  = index + 2;
        index = wlines.find_first_of(L"\r\n",last);     
    }
}

void myfree(FileObject *file) {
    delete file;
}

void FileList::FreeFileListObjects() {
    std::for_each(mFiles.begin(), mFiles.end(), myfree);
    mFiles.clear();
}
