/*
    NppFTP: FTP/SFTP functionality for Notepad++
    Copyright (C) 2010  Harry (harrybharry@users.sourceforge.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdInc.h"
#include "FilerPanel.h"
#include "resource.h"
#include "Notepad_plus_msgs.h"
#include "AndroidDevice.h"
#include "Plugin.h"
#include "Commands.h"
#include <fstream>
#include <Commctrl.h>
#include <Windowsx.h>
#include <algorithm>
#include <direct.h>


extern HINSTANCE g_hInstance;
extern void pullFile(const char* dir, const TCHAR* local, void* args);
extern void pushFile(const char* dir, const TCHAR* local, void* args);
extern bool pushFile2(const char* remote, const TCHAR* path, void* args);
extern void getFileList(const char* curDir, FilerPanel *filer);
extern void shellCmd(std::vector<std::string> cmds, HWND hwnd);

const TCHAR * FilerPanel::FILERWINDOWCLASS = TEXT("AndroidFiler");


FilerWindow::FilerWindow() :
    Window(NULL, WC_LISTVIEW)
{
    m_style = WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL;
    m_exStyle = WS_EX_CLIENTEDGE;
}

FilerWindow::~FilerWindow() {
}

int FilerWindow::Create(HWND hParent) {
    int ret = Window::Create(hParent);
    if (ret == -1)
        return -1;

    ListView_SetExtendedListViewStyle(m_hwnd, LVS_EX_FULLROWSELECT);

    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvc.fmt = LVCFMT_LEFT;

    lvc.cx = 140;
    TCHAR strAction[] = TEXT("File");
    lvc.pszText = strAction;
    ListView_InsertColumn(m_hwnd, 0, &lvc);

    lvc.cx = 80;
    TCHAR strProgress[] = TEXT("Attrib");
    lvc.pszText = strProgress;
    ListView_InsertColumn(m_hwnd, 1, &lvc);

    lvc.cx = 50;
    TCHAR strFile[] = TEXT("Size");
    lvc.pszText = strFile;
    ListView_InsertColumn(m_hwnd, 2, &lvc);

    return 0;
}

FilerPanel::FilerPanel() :
    DockableWindow(FILERWINDOWCLASS)
{
    m_files = nullptr;
    m_exStyle = 0;
    m_style = 0;
    m_szCurDir = L"/sdcard/";
    //Create background brush
    m_backgroundBrush = GetSysColorBrush(COLOR_3DFACE);
}

FilerPanel::~FilerPanel() {
    DeleteObject(m_backgroundBrush);

    if (m_files) {
        delete m_files;
        m_files = nullptr;
    }
}

int FilerPanel::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand) {
    SetTitle(TEXT("Filer"));
    SetInfo(TEXT("AndroidLogger"));
    SetLocation(DWS_DF_CONT_RIGHT);

    int res = DockableWindow::Create(hParent, hNpp, MenuID, MenuCommand);
    if (res != 0)
        return -1;

    res = m_rebar.Create(m_hwnd);
    if (res != 0) {
        Destroy();
        return -1;
    }

    Init();

    res = m_toolbar.Create(m_rebar.GetHWND());
    if (res != 0) {
        Destroy();
        return -1;
    }

    res = m_hFileList.Create(m_hwnd);
    if (res != 0) {
        Destroy();
        return -1;
    }
    ListView_SetImageList(m_hFileList.GetHWND(), m_hImageList, LVSIL_SMALL);

    m_toolbar.AddToRebar(&m_rebar);

    CreateMenus();

    m_toolbar.Show(true);
    m_rebar.Show(true);
    m_hFileList.Show(true);

    SetToolbarState();

    m_hProgressBar = ::CreateWindow(PROGRESS_CLASS, TEXT("Filer Progress Bar"),
                    WS_CHILD | WS_VISIBLE | PBS_SMOOTH, CW_USEDEFAULT,CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, NULL, g_hInstance, NULL);

    return 0;
}

int FilerPanel::Destroy() {
    m_hFileList.Destroy();
    m_toolbar.Destroy();
    m_rebar.Destroy();

    ImageList_Destroy(m_hImageList);
    m_hImageList = NULL;

    DestroyMenu(m_popupStars);
    return Window::Destroy();
}

int FilerPanel::Show(bool show) {
    int ret = DockableWindow::Show(show);
    if (ret != 0)
        return -1;

    m_hFileList.Show(true);
    //Focus();
    showCurDir();
    return 0;
}

int FilerPanel::Focus() {
    return m_hFileList.Focus();
}

int FilerPanel::Init() {
    //Load the simple imagelist
    HBITMAP hBitmap = LoadBitmap(g_hInstance,(LPCTSTR)MAKEINTRESOURCE(IDB_FILER_ICONS));
    HBITMAP hBitmapMask = LoadBitmap(g_hInstance,(LPCTSTR)MAKEINTRESOURCE(IDB_FILER_ICONS_MASK));

    m_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 5, 2);
    ImageList_Add(m_hImageList, hBitmap, hBitmapMask);

    DeleteObject(hBitmap);
    DeleteObject(hBitmapMask);
    return 0;
}

int FilerPanel::OnSize(int newWidth, int newHeight) {
    int toolbarHeight = m_rebar.GetHeight();

    m_rebar.Resize(newWidth, toolbarHeight);

    int clientheight = newHeight - toolbarHeight;
    ::SetWindowPos(m_hProgressBar, 0, 0, toolbarHeight, newWidth, 4, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
    m_hFileList.Move(0, toolbarHeight+4, newWidth, clientheight);
    return 0;
}

int FilerPanel::RegisterClass() {
    WNDCLASSEX FilerPanelClass;
    FilerPanelClass.cbSize = sizeof(WNDCLASSEX);
    FilerPanelClass.style = CS_DBLCLKS;//|CS_NOCLOSE;
    FilerPanelClass.cbClsExtra = 0;
    FilerPanelClass.cbWndExtra = 0;
    FilerPanelClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    FilerPanelClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    FilerPanelClass.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
    FilerPanelClass.lpszMenuName = NULL;
    FilerPanelClass.hIconSm = NULL;

    FilerPanelClass.lpfnWndProc = NULL;
    FilerPanelClass.hInstance = NULL;
    FilerPanelClass.lpszClassName = NULL;
    
    //register the class
    int ret = Window::RegisterClass(FILERWINDOWCLASS, FilerPanelClass);
    if (ret != 0) {
        return -1;
    }

    return 0;
}

LRESULT FilerPanel::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    bool doDefaultProc = false;
    LRESULT result = 0;

    switch(uMsg) {
        case WM_SETFOCUS: {
            //Why restore focus here? This window should never be able to get focus in the first place
            HWND hPrev = (HWND)wParam;
            if (hPrev != NULL) {
                ::SetFocus(hPrev);
            }
            break; }

        case WM_ERASEBKGND: {
            HDC hDC = (HDC) wParam;
            RECT rectClient;
            GetClientRect(m_hwnd, &rectClient);
            FillRect(hDC, &rectClient, m_backgroundBrush);
            result = TRUE;
            break; }

        case WM_NOTIFY:
			return OnNotify(uMsg, wParam, lParam);

        case WM_COMMAND:
			return OnCommand(uMsg, wParam, lParam);

        case WM_UPDATE_FILER: {
            if (m_files != nullptr) {
                delete m_files;
                m_files = nullptr;
            }

            m_files = (FileList*)wParam;
            //int size = lParam;
            updateFilerList();
            break;}

        case WM_FILE_PROGRESS:
			if (wParam > 0) {
				::PostMessage(m_hProgressBar, PBM_DELTAPOS, wParam, 0);
				return TRUE;
			}

            ::ShowWindow(m_hProgressBar, SW_HIDE);
            ::PostMessage(m_hProgressBar, PBM_SETPOS, 0, 0);
			if (lParam == 0) {
				::MessageBox(m_hwnd, L"Download complete!", MSGBOX_TITLE, 0);
			} else {
				::MessageBox(m_hwnd, L"Push complete!", MSGBOX_TITLE, 0);
				showCurDir();
			}
            return TRUE;

		case WM_FILE_SHELLCMD:
			if (m_toolbar.GetChecked(ID_TOOLBAR_READ)) {
				m_toolbar.SetChecked(ID_TOOLBAR_READ, false);
			} else {
				showCurDir();
			}
			return TRUE;

        default:
            doDefaultProc = true;
            break;
    }

    if (doDefaultProc)
        result = DockableWindow::MessageProc(uMsg, wParam, lParam);

    return result;
}

LRESULT FilerPanel::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	NMHDR nmh = (NMHDR) *((NMHDR*)lParam);
	LRESULT result = 0;
	bool doDefaultProc = false;

	// 1. toolbar message
    if (nmh.hwndFrom == m_toolbar.GetHWND()) {
		NMTOOLBAR * pnmtb = (NMTOOLBAR*)lParam;
		if (nmh.code != TBN_DROPDOWN) {
			return DockableWindow::MessageProc(uMsg, wParam, lParam);
		}

		if (pnmtb->iItem == ID_TOOLBAR_FAVOR) {
			return m_toolbar.DoDropDown(ID_TOOLBAR_FAVOR);
		}
		return TBDDRET_NODEFAULT;
    } 
	
	// 2. toolbar overflow popup message
	else if (nmh.hwndFrom == m_rebar.GetHWND() && nmh.code == RBN_CHEVRONPUSHED ) {
        NMREBARCHEVRON *lpnm = (NMREBARCHEVRON*) lParam;
		POINT pt ={lpnm->rc.left, lpnm->rc.bottom};
        ClientToScreen(m_rebar.GetHWND(), &pt);
        m_toolbar.DoPopop(pt);
		return TRUE;
    } 
	
	// 3. filer list message
	else if (nmh.hwndFrom == m_hFileList.GetHWND()) {
        if(nmh.code == NM_DBLCLK ) {
            int iSelect = SendMessage(m_hFileList.GetHWND(), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if( iSelect == -1) {
				return TRUE;
			}

			if (!m_files->mFiles[iSelect]->isValid()) {
				return TRUE;
			}

			if (!(m_files->mFiles[iSelect]->mPerm[0] == 'd')) {
				return TRUE;
			}

            tstring newPath = m_files->mFiles[iSelect]->mName;
            m_szCurDir += newPath;
            m_szCurDir += L"/";
            showCurDir();                    
        } else if(nmh.code == NM_CLICK ){
            int iSelect = SendMessage(m_hFileList.GetHWND(), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if( iSelect != -1) {
                m_curSel = iSelect;
            }
        }
    } 

	// 4. other message
	else {
        switch(nmh.code) {
            case TTN_GETDISPINFO: {
                LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;
                lpttt->hinst = NULL;
                lpttt->lpszText = (TCHAR*)m_toolbar.GetTooltip(lpttt->hdr.idFrom);
                result = TRUE;
                break; }
            default: {
                doDefaultProc = true;
                break; }
        }
    }

	if (doDefaultProc) {
        result = DockableWindow::MessageProc(uMsg, wParam, lParam);
	}
	return result;
}

LRESULT FilerPanel::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(LOWORD(wParam)) {
    case IDM_POPUP_PROFILE_FIRST:
        m_szCurDir = L"/sdcard/";
        showCurDir();
        return TRUE;

    case IDM_POPUP_PROFILE_FIRST+1:
        m_szCurDir = L"/storage/";
        showCurDir();
        return TRUE;

    case ID_TOOLBAR_UP: {
		if ( m_szCurDir.size() == 1) {
            return TRUE;
        }
        tstring dir = m_szCurDir.substr(0, m_szCurDir.size()-1);
        dir =  dir.substr(0, dir.find_last_of(L"/")+1);
        m_szCurDir = dir;
        showCurDir();
        return TRUE;}

    case ID_TOOLBAR_DELETE: {
		int iSelect = SendMessage(m_hFileList.GetHWND(), LVM_GETNEXTITEM, -1, LVNI_SELECTED);
        if( iSelect == -1) {
			return TRUE;
		}
        doFileOp(ID_TOOLBAR_DELETE);
        return TRUE; }

    case ID_TOOLBAR_READ: {
        if (!isNormalFileCurSel()){
            return TRUE;;
        }
        doFileOp(ID_TOOLBAR_READ);
        return TRUE;}

    case ID_TOOLBAR_DETAIL: {
        if ( m_files->mFiles[m_curSel]->mPerm.empty() ) {
            return TRUE;
        }
        ::MessageBox(m_hwnd, m_files->mFiles[m_curSel]->mDetail.c_str(), MSGBOX_TITLE, 0);
        return TRUE;}

    case ID_TOOLBAR_PUSH: {
        TCHAR path[MAX_PATH] = {0};
        PU::GetOpenFilename(path, MAX_PATH, m_hwnd);
                
        std::ifstream in(path);
        in.seekg(0, std::ios::end);
        long size = in.tellg(); 
        in.close();

        tstring local = path;
        std::string::size_type pos = local.find_last_of(L"\\");
        if (pos == std::string::npos) {
            return TRUE;
        }

        tstring remote = m_szCurDir + local.substr(pos+1);

        ::ShowWindow(m_hProgressBar, SW_SHOW);
        ::SendMessage(m_hProgressBar, PBM_SETRANGE32, 0, size);
        char* dir = SU::TCharToUtf8(remote.c_str());
        pushFile(dir, path, this);
        delete dir;
		return TRUE;}

    case ID_TOOLBAR_PULL: {
        if (!isNormalFileCurSel()){
            return TRUE;
        }

		const TCHAR *workingDir = getWorkingDirectory();

        FileObject *file = m_files->mFiles[m_curSel];
        tstring local = tstring(workingDir) + L"\\" + file->mName;
        tstring remote = file->getFullPath();

        ::ShowWindow(m_hProgressBar, SW_SHOW);
        ::SendMessage(m_hProgressBar, PBM_SETRANGE32, 0, m_files->mFiles[m_curSel]->mIntSize);

        char* uRemote = SU::TCharToUtf8(remote.c_str());
        pullFile(uRemote, local.c_str(), this);
        delete uRemote;
        return TRUE;}

    case ID_TOOLBAR_REFRESH:
        showCurDir();
        return TRUE;
    default:
        break;
    }

	 return FALSE;
}

void FilerPanel::doFileOp(int cmdId) {
    std::vector<std::string> cmds;
    std::string cmd;

    switch(cmdId) {
    case ID_TOOLBAR_DELETE:{
		if (m_toolbar.GetChecked(ID_TOOLBAR_READ)) {
			::MessageBox(m_hwnd, L"Please wait/cancel read complete!", MSGBOX_TITLE, 0);
			return;
		}
        cmd = "rm -rf";
        tstring msg(L"Delete the file ");
        msg += m_files->mFiles[m_curSel]->mName;
        msg += L"?";
        if (::MessageBox(m_hwnd, msg.c_str(), 
                MSGBOX_TITLE, MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDNO) {
            return;
        }
        break;}
    case ID_TOOLBAR_READ:
        cmd = "cat";
		m_toolbar.SetChecked(ID_TOOLBAR_READ, TRUE);
        break;
    default:
        return;
    }

    createFilerCmd(cmd);
    cmds.push_back(cmd);
    shellCmd(cmds, m_hwnd);
}

bool FilerPanel::isNormalFileCurSel() {
    if ( m_files->mFiles[m_curSel]->mPerm.empty() ) {
        return false;
    }
    if (m_files->mFiles[m_curSel]->mPerm[0] == 'd' ||  m_files->mFiles[m_curSel]->mPerm[0] == 'l') {
        return false;
    }
    return true;
}

int FilerPanel::createFilerCmd(std::string &cmd) {
    FileObject *file = m_files->mFiles[m_curSel];

    cmd += " \"";
    char* uRemote = SU::TCharToUtf8(file->getFullPath().c_str());
    cmd += uRemote;
    cmd += "\"";

    delete uRemote;
    return cmd.size();
}

void FilerPanel::showCurDir() {
    SetInfo(m_szCurDir.c_str());
    char* dir = SU::TCharToUtf8(m_szCurDir.c_str());
    getFileList(dir, this);
    delete dir;
}

int FilerPanel::CreateMenus() {
    //Create menu for settings button on toolbar
    m_popupStars = ::CreatePopupMenu();
    ::AppendMenu(m_popupStars, MF_STRING, IDM_POPUP_PROFILE_FIRST, L"/sdcard/");
    ::AppendMenu(m_popupStars, MF_STRING, IDM_POPUP_PROFILE_FIRST+1, L"/storage/");

    m_toolbar.SetMenu(ID_TOOLBAR_FAVOR, m_popupStars);
    return 0;
}

int FilerPanel::SetToolbarState() {
	//m_toolbar.Enable(ID_TOOLBAR_DELETE, true);
    return 0;
}

void FilerPanel::updateFilerList() {
    int nLine = m_files->mFiles.size();
    int ret = 0;

    ListView_DeleteAllItems(m_hFileList.GetHWND());

	if (nLine == 0 || !m_files->mFiles[0]->isValid()){
		return;
	}

    for(int i = 0; i < nLine; i++ ) {
        LVITEM lvi = {0};
        lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
        lvi.iItem = i;

        TCHAR* name = SU::DupString(m_files->mFiles[i]->mName.c_str());
        TCHAR* perm = SU::DupString(m_files->mFiles[i]->mPerm.c_str());
        TCHAR* size = SU::DupString(m_files->mFiles[i]->mSize.c_str());

        lvi.iImage = m_files->mFiles[i]->mFileType;
        
        lvi.iSubItem = 0;
        lvi.pszText = name;
        ret = ListView_InsertItem(m_hFileList.GetHWND(),  &lvi);

        ListView_SetItemText(m_hFileList.GetHWND(), ret, 1, perm);
        ListView_SetItemText(m_hFileList.GetHWND(), ret, 2, size );

        delete name;
        delete perm;
        delete size;
    }
}
