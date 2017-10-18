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
#ifndef FILERPANLE_H
#define FILERPANLE_H

#include "DockableWindow.h"
#include "Toolbar.h"
#include "FileObject.h"
#include <vector>

#define WM_UPDATE_FILER  WM_USER+1
#define WM_FILE_PROGRESS WM_USER+2
#define WM_FILE_SHELLCMD WM_USER+3

class FilerWindow : public Window {
public:
                            FilerWindow();
    virtual                 ~FilerWindow();
    virtual int             Create(HWND hParent);
private:

};

class FilerPanel : public DockableWindow{
public:
                            FilerPanel();
    virtual                 ~FilerPanel();

    ////////////////////////
    //DockableWindow
    virtual int             Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand);
    virtual int             Destroy();
    virtual int             Show(bool show);
    virtual int             Focus();
    virtual int             Init();

    virtual int             OnSize(int newWidth, int newHeight);
    static int              RegisterClass();
    virtual LRESULT         MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


protected:
    virtual int             CreateMenus();
    virtual int             SetToolbarState();
	LRESULT                 OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT                 OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    void                    showCurDir();
	void                    doFileOp(int cmdId);

public:
	tstring&                currentDir() {return m_szCurDir;}
	bool                    isNormalFileCurSel();

private:
    int                     createFilerCmd(std::string &cmd);
	void                    updateFilerList();



    Toolbar                 m_toolbar;
    Rebar                   m_rebar;
    FilerWindow             m_hFileList;
    HWND                    m_hProgressBar;

    HIMAGELIST              m_hImageList;
    HBRUSH                  m_backgroundBrush;
    HMENU                   m_popupStars;
    
    tstring                 m_szCurDir;
    int                     m_curSel;
    FileList*               m_files;

    static const TCHAR     *FILERWINDOWCLASS;
};

#endif //FILERPANLE_H
