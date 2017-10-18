#ifndef FILERONBJECT_H
#define FILERONBJECT_H

#include <vector>
#include <regex>

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#define LS_REGEX L"^([bcdlsp-][-r][-w][-xsS][-r][-w][-xsS][-r][-w][-xstST])\\s+(\\S+)\\s+(\\S+)\\s+([\\d\\s,]*)\\s+(\\d{4}-\\d\\d-\\d\\d)\\s+(\\d\\d:\\d\\d)\\s+(.*)$"

class FileObject {
friend class FilerPanel;
friend class FilerPanel;

public:
    FileObject(tstring ls_l, tstring &parentDir);
    FileObject(){}

public:
    tstring getFullPath() {return mParentDir+mName;}
	bool isValid() { return mIsValid;}

private:
    tstring mPerm;    // dr-xr-xr-x
    tstring mOwner;   // root
    tstring mGroup;   // root
    tstring mSize;    // 
    tstring mDate;    // 1970-01-01
    tstring mTime;    // 08:00
    tstring mName;    // sbin
    tstring mDetail;  // drwxr-x--- root root 1970-01-01 08:00 sbin

    int     mFileType;    // 0:folder 1: other 2:apk 3:image
    int     mIntSize;
    tstring mParentDir;
	bool    mIsValid;
};

class FileList {
friend class FilerPanel;
public:
    FileList(std::string &lines, tstring &parentDir);
    virtual ~FileList();

public:
    void BuildFileListObjects(std::string &lines, tstring &parentDir);
    void FreeFileListObjects();

private:
    std::vector<FileObject*> mFiles;
};

#endif //FILERONBJECT_H