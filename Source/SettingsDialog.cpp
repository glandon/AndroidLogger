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
#include "SettingsDialog.h"
#include "resource.h"
#include <Windowsx.h>



SettingsDialog::SettingsDialog() :
	Dialog(IDD_SETTINGS), mPluginSettings(nullptr){
}

SettingsDialog::~SettingsDialog() {
}

int SettingsDialog::Create(HWND hParent, HINSTANCE hInstance, PluginSettings *settings) {
	mPluginSettings = settings;
	m_hInstance = hInstance;
	return Dialog::Create(hParent, true, L"AndroidLogger");
}

INT_PTR SettingsDialog::OnInitDialog() {
	::SetDlgItemText(m_hwnd, IDC_WORKING_DIR, mPluginSettings->workingDirectory);
	::SetDlgItemText(m_hwnd, IDC_LOGCAT_TAG, mPluginSettings->logcatTag);
	
	switch(mPluginSettings->logcatLevel[0]){
	case L'V':
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_V);
		break;
	case L'D':
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_D);
		break;
	case L'I':
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_I);
		break;
	case L'W':
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_W);
		break;
	case L'E':
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_E);
		break;
	default:
		CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_V);
		break;
	}

	if (mPluginSettings->autoShowCapture == 1) {
		Button_SetCheck(::GetDlgItem(m_hwnd, IDC_CHECK_AUTOSHOW),TRUE);
	}
	return Dialog::OnInitDialog();
}

INT_PTR SettingsDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return Dialog::DlgMsgProc(uMsg, wParam, lParam);;
}

INT_PTR SettingsDialog::OnCommand(int ctrlId, int notifCode, HWND idHwnd) {	
	switch(ctrlId) {
		case IDOK:
			SaveAllSettings();
			EndDialog(m_hwnd, 0);
			break;

		case IDC_RADIO_V:
			mPluginSettings->logcatLevel[0] = L'V';
			mPluginSettings->logcatLevel[1] = L'\0';
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_V);
			}
			break;
		case IDC_RADIO_D:
			mPluginSettings->logcatLevel[0] = L'D';
			mPluginSettings->logcatLevel[1] = L'\0';
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_D);
			}
			break;
		case IDC_RADIO_I:
			mPluginSettings->logcatLevel[0] = L'I';
			mPluginSettings->logcatLevel[1] = L'\0';
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_I);
			}
			break;
		case IDC_RADIO_W:
			mPluginSettings->logcatLevel[0] = L'W';
			mPluginSettings->logcatLevel[1] = L'\0';
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_W);
			}
			break;
		case IDC_RADIO_E:
			mPluginSettings->logcatLevel[0] = L'E';
			mPluginSettings->logcatLevel[1] = L'\0';
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hwnd, IDC_RADIO_V, IDC_RADIO_E, IDC_RADIO_E);
				
			}
			break;

		default: {
			return Dialog::OnCommand(ctrlId, notifCode, idHwnd);}
	}

	return TRUE;
}

INT_PTR SettingsDialog::OnNotify(NMHDR * pnmh) {
	return Dialog::OnNotify(pnmh);
}

void SettingsDialog::SaveAllSettings() {
	TCHAR workingDirBuffer[MAX_PATH];
	::GetDlgItemText(m_hwnd, IDC_WORKING_DIR, workingDirBuffer, MAX_PATH);
	wcscpy(mPluginSettings->workingDirectory, workingDirBuffer);
	
	TCHAR logcatTag[64] = {0};
	::GetDlgItemText(m_hwnd, IDC_LOGCAT_TAG, logcatTag, MAX_PATH);
	wcscpy(mPluginSettings->logcatTag, logcatTag);

	LRESULT checked = Button_GetCheck(::GetDlgItem(m_hwnd, IDC_CHECK_AUTOSHOW));
	if (checked == BST_CHECKED) {
		mPluginSettings->autoShowCapture = 1;
	} else {
		mPluginSettings->autoShowCapture = 0;
	}

	saveSettings(mPluginSettings);
}

