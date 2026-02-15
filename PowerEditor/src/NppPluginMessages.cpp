// This file is part of Notepad++ project
// Copyright (C)2025 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// ============================================================================
// NppPluginMessages.cpp - Linux implementation of NPPM_* message dispatch
// ============================================================================
// On Windows, plugins call ::SendMessage(nppData._nppHandle, NPPM_*, wParam, lParam)
// which enters the Win32 WndProc in NppBigSwitch.cpp.
//
// On Linux, there is no Win32 message loop. Instead, the stub SendMessage()
// in LinuxTypes.h routes calls to Notepad_plus::handlePluginMessage() via a
// global function pointer. This file implements that dispatch method,
// translating each NPPM_* constant to the appropriate Notepad_plus method call.
// ============================================================================

#ifdef NPP_LINUX

#include "Notepad_plus.h"
#include "Parameters.h"
#include "Notepad_plus_msgs.h"
#include "menuCmdID.h"
#include "resource.h"
#include "Scintilla.h"
#include "ScintillaEditView.h"
#include "NppDarkMode.h"
#include <Lexilla.h>
#include "QtControls/MainWindow/Notepad_plus_Window.h"

#include <cstring>
#include <cwchar>
#include <filesystem>
#include <memory>
#include <string>

#ifndef CURRENTWORD_MAXLENGTH
#define CURRENTWORD_MAXLENGTH 2048
#endif

// ============================================================================
// Global dispatch function pointer - set by MainWindow::initPlugins()
// ============================================================================
static Notepad_plus* g_pNotepad_plus = nullptr;

void nppPluginMessageDispatcher_register(Notepad_plus* pNpp)
{
	g_pNotepad_plus = pNpp;
}

void nppPluginMessageDispatcher_unregister()
{
	g_pNotepad_plus = nullptr;
}

// Helper: Get MainWindow from a ScintillaEditView's widget hierarchy
static QtControls::MainWindow::MainWindow* getMainWindowFromEditView(ScintillaEditView& editView)
{
	QWidget* w = editView.getWidget();
	if (!w)
		return nullptr;
	QWidget* topLevel = w->window();
	return qobject_cast<QtControls::MainWindow::MainWindow*>(topLevel);
}

// This is the global SendMessage replacement called from LinuxTypes.h
LRESULT nppPluginMessageDispatcher_sendMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Only dispatch NPPM_* messages (range NPPMSG+0 to NPPMSG+200 and RUNCOMMAND_USER range)
	// Other messages (WM_*, SCI_*) go to their respective handlers or are ignored

	if (!g_pNotepad_plus)
		return 0;

	// Check if this is an NPPM_* message
	bool isNppMessage = (message >= NPPMSG && message < NPPMSG + 200);
	bool isRunCommand = (message >= RUNCOMMAND_USER && message < RUNCOMMAND_USER + 32);

	if (isNppMessage || isRunCommand)
	{
		return g_pNotepad_plus->handlePluginMessage(message, wParam, lParam);
	}

	// For non-NPPM messages directed at Scintilla handles, we could route
	// SCI_* messages, but plugins typically use the Scintilla handle directly.
	// We return 0 for unhandled messages.
	return 0;
}

// ============================================================================
// Helper: Copy a wide string to a plugin-provided buffer with size checking
// ============================================================================
static LRESULT copyStringToPlugin(const std::wstring& src, WPARAM bufferLen, LPARAM destPtr)
{
	if (destPtr == 0)
		return 0;

	wchar_t* dest = reinterpret_cast<wchar_t*>(destPtr);

	if (bufferLen == 0)
	{
		// Plugin did not provide a buffer size -- return the required length
		// so the plugin can allocate appropriately and call again.
		return static_cast<LRESULT>(src.length());
	}

	// Check if buffer is large enough (bufferLen includes null terminator)
	if (static_cast<size_t>(bufferLen) <= src.length())
		return 0;

	wcsncpy(dest, src.c_str(), static_cast<size_t>(bufferLen) - 1);
	dest[bufferLen - 1] = L'\0';
	return static_cast<LRESULT>(src.length());
}

// ============================================================================
// Notepad_plus::handlePluginMessage - Main NPPM_* message dispatch
// ============================================================================
LRESULT Notepad_plus::handlePluginMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	NppParameters& nppParam = NppParameters::getInstance();

	switch (message)
	{
		// ==================================================================
		// Current editor / view queries
		// ==================================================================

		case NPPM_GETCURRENTSCINTILLA:
		{
			int* id = reinterpret_cast<int*>(lParam);
			if (!id)
				return FALSE;
			if (!_pEditView)
			{
				*id = -1;
				return FALSE;
			}
			if (_pEditView == &_mainEditView)
				*id = MAIN_VIEW;
			else if (_pEditView == &_subEditView)
				*id = SUB_VIEW;
			else
				*id = -1;
			return TRUE;
		}

		case NPPM_GETCURRENTLANGTYPE:
		{
			if (!lParam || !_pEditView)
				return FALSE;
			Buffer* buf = _pEditView->getCurrentBuffer();
			if (!buf)
				return FALSE;
			*(reinterpret_cast<LangType*>(lParam)) = buf->getLangType();
			return TRUE;
		}

		case NPPM_SETCURRENTLANGTYPE:
		{
			if (!_pEditView)
				return FALSE;
			Buffer* buf = _pEditView->getCurrentBuffer();
			if (!buf)
				return FALSE;
			buf->setLangType(static_cast<LangType>(lParam));
			return TRUE;
		}

		case NPPM_GETCURRENTLINE:
		{
			if (!_pEditView)
				return 0;
			return _pEditView->getCurrentLineNumber();
		}

		case NPPM_GETCURRENTCOLUMN:
		{
			if (!_pEditView)
				return 0;
			return _pEditView->getCurrentColumnNumber();
		}

		case NPPM_GETCURRENTVIEW:
		{
			return _activeView;
		}

		// ==================================================================
		// File path queries
		// ==================================================================

		case NPPM_GETFULLCURRENTPATH:
		case NPPM_GETCURRENTDIRECTORY:
		case NPPM_GETFILENAME:
		case NPPM_GETNAMEPART:
		case NPPM_GETEXTPART:
		{
			if (!_pEditView)
				return 0;
			Buffer* buf = _pEditView->getCurrentBuffer();
			if (!buf)
				return 0;
			std::wstring fullPath = buf->getFullPathName();
			std::wstring result;

			namespace fs = std::filesystem;
			fs::path fpath(fullPath);

			if (message == NPPM_GETFULLCURRENTPATH)
			{
				result = fullPath;
			}
			else if (message == NPPM_GETCURRENTDIRECTORY)
			{
				result = fpath.parent_path().wstring();
			}
			else if (message == NPPM_GETFILENAME)
			{
				result = fpath.filename().wstring();
			}
			else if (message == NPPM_GETNAMEPART)
			{
				result = fpath.stem().wstring();
			}
			else if (message == NPPM_GETEXTPART)
			{
				result = fpath.extension().wstring();
			}

			return copyStringToPlugin(result, wParam, lParam);
		}

		case NPPM_GETCURRENTWORD:
		{
			wchar_t* pTchar = reinterpret_cast<wchar_t*>(lParam);
			if (!pTchar || !_pEditView)
				return FALSE;

			auto txtW = _pEditView->getSelectedTextToWChar();
			size_t txtLen = txtW.length();

			if (wParam == 0)
			{
				// No buffer size provided -- return required length
				return static_cast<LRESULT>(txtLen);
			}

			if (txtLen < static_cast<size_t>(wParam))
			{
				wcsncpy(pTchar, txtW.c_str(), static_cast<size_t>(wParam));
				pTchar[wParam - 1] = L'\0';
				return TRUE;
			}
			return FALSE;
		}

		case NPPM_GETCURRENTLINESTR:
		{
			wchar_t* pTchar = reinterpret_cast<wchar_t*>(lParam);
			if (!pTchar || !_pEditView)
				return FALSE;

			constexpr int strSize = CURRENTWORD_MAXLENGTH;
			auto str = std::make_unique<wchar_t[]>(strSize);
			std::fill_n(str.get(), strSize, L'\0');

			_pEditView->getLine(_pEditView->getCurrentLineNumber(), str.get(), strSize);
			size_t strLen = wcslen(str.get());

			if (wParam == 0)
			{
				// No buffer size provided -- return required length
				return static_cast<LRESULT>(strLen);
			}

			if (strLen < static_cast<size_t>(wParam))
			{
				wcsncpy(pTchar, str.get(), static_cast<size_t>(wParam));
				pTchar[wParam - 1] = L'\0';
				return TRUE;
			}
			return FALSE;
		}

		case NPPM_GETNPPDIRECTORY:
		case NPPM_GETNPPFULLFILEPATH:
		{
			std::wstring nppPath = nppParam.getNppPath();
			if (message == NPPM_GETNPPDIRECTORY)
			{
				// nppPath is already the directory on Linux
			}
			else
			{
				// Full path to the executable
				nppPath += L"/notepad-plus-plus";
			}
			return copyStringToPlugin(nppPath, wParam, lParam);
		}

		// ==================================================================
		// Open file count and buffer queries
		// ==================================================================

		case NPPM_GETNBOPENFILES:
		{
			size_t nbDocPrimary = _mainDocTab.nbItem();
			size_t nbDocSecond = _subDocTab.nbItem();
			if (lParam == ALL_OPEN_FILES)
				return nbDocPrimary + nbDocSecond;
			else if (lParam == PRIMARY_VIEW)
				return nbDocPrimary;
			else if (lParam == SECOND_VIEW)
				return nbDocSecond;
			else
				return 0;
		}

		case NPPM_GETCURRENTDOCINDEX:
		{
			if (lParam == SUB_VIEW)
			{
				if (!viewVisible(SUB_VIEW))
					return -1;
				return _subDocTab.getCurrentTabIndex();
			}
			else // MAIN_VIEW
			{
				if (!viewVisible(MAIN_VIEW))
					return -1;
				return _mainDocTab.getCurrentTabIndex();
			}
		}

		case NPPM_GETCURRENTBUFFERID:
		{
			if (!_pEditView)
				return 0;
			return reinterpret_cast<LRESULT>(_pEditView->getCurrentBufferID());
		}

		case NPPM_GETBUFFERIDFROMPOS:
		{
			DocTabView* pView = nullptr;
			if (lParam == MAIN_VIEW)
				pView = &_mainDocTab;
			else if (lParam == SUB_VIEW)
				pView = &_subDocTab;
			else
				return reinterpret_cast<LRESULT>(BUFFER_INVALID);

			if (static_cast<size_t>(wParam) < pView->nbItem())
				return reinterpret_cast<LRESULT>(pView->getBufferByIndex(wParam));

			return reinterpret_cast<LRESULT>(BUFFER_INVALID);
		}

		case NPPM_GETFULLPATHFROMBUFFERID:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return -1;
			const wchar_t* fullPath = buf->getFullPathName();
			if (!fullPath)
				return -1;
			size_t pathLen = wcslen(fullPath);
			wchar_t* fn2copy = reinterpret_cast<wchar_t*>(lParam);
			if (fn2copy)
			{
				// Plugin API: lParam is the output buffer, no size param available.
				// Use MAX_PATH as a safe upper bound (matches Windows behavior).
				wcsncpy(fn2copy, fullPath, MAX_PATH - 1);
				fn2copy[MAX_PATH - 1] = L'\0';
			}
			return static_cast<LRESULT>(pathLen);
		}

		case NPPM_GETPOSFROMBUFFERID:
		{
			int i;
			if (lParam == SUB_VIEW)
			{
				if ((i = _subDocTab.getIndexByBuffer(reinterpret_cast<BufferID>(wParam))) != -1)
				{
					long view = SUB_VIEW;
					view <<= 30;
					return view | i;
				}
				if ((i = _mainDocTab.getIndexByBuffer(reinterpret_cast<BufferID>(wParam))) != -1)
				{
					long view = MAIN_VIEW;
					view <<= 30;
					return view | i;
				}
			}
			else
			{
				if ((i = _mainDocTab.getIndexByBuffer(reinterpret_cast<BufferID>(wParam))) != -1)
				{
					long view = MAIN_VIEW;
					view <<= 30;
					return view | i;
				}
				if ((i = _subDocTab.getIndexByBuffer(reinterpret_cast<BufferID>(wParam))) != -1)
				{
					long view = SUB_VIEW;
					view <<= 30;
					return view | i;
				}
			}
			return -1;
		}

		// ==================================================================
		// Buffer property queries
		// ==================================================================

		case NPPM_GETBUFFERLANGTYPE:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return -1;
			return buf->getLangType();
		}

		case NPPM_SETBUFFERLANGTYPE:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return FALSE;
			buf->setLangType(static_cast<LangType>(lParam));
			return TRUE;
		}

		case NPPM_GETBUFFERENCODING:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return -1;
			return buf->getUnicodeMode();
		}

		case NPPM_SETBUFFERENCODING:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return FALSE;
			buf->setUnicodeMode(static_cast<UniMode>(lParam));
			return TRUE;
		}

		case NPPM_GETBUFFERFORMAT:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return -1;
			// Convert Qt Buffer::EolType to global EolType values for plugin API
			auto qtEol = buf->getEolFormat();
			switch (qtEol)
			{
				case Buffer::eolWindows: return static_cast<int>(EolType::windows);
				case Buffer::eolUnix:    return static_cast<int>(EolType::unix);
				case Buffer::eolMac:     return static_cast<int>(EolType::macos);
				default:                 return static_cast<int>(EolType::unknown);
			}
		}

		case NPPM_SETBUFFERFORMAT:
		{
			Buffer* buf = MainFileManager.getBufferByID(reinterpret_cast<BufferID>(wParam));
			if (!buf)
				return FALSE;
			// Convert global EolType from plugin API to Qt Buffer::EolType
			Buffer::EolType qtEol;
			switch (static_cast<EolType>(lParam))
			{
				case EolType::windows: qtEol = Buffer::eolWindows; break;
				case EolType::macos:   qtEol = Buffer::eolMac;     break;
				case EolType::unix:    qtEol = Buffer::eolUnix;    break;
				default:               qtEol = Buffer::eolUnknown;  break;
			}
			buf->setEolFormat(qtEol);
			return TRUE;
		}

		// ==================================================================
		// File operations
		// ==================================================================

		case NPPM_SAVECURRENTFILE:
		{
			return fileSave();
		}

		case NPPM_SAVECURRENTFILEAS:
		{
			if (!_pEditView)
				return FALSE;
			bool asCopy = wParam == TRUE;
			BufferID currentBufferID = _pEditView->getCurrentBufferID();
			const wchar_t* filename = reinterpret_cast<const wchar_t*>(lParam);
			if (!filename)
				return FALSE;
			return doSave(currentBufferID, filename, asCopy);
		}

		case NPPM_SAVEALLFILES:
		{
			return fileSaveAll();
		}

		case NPPM_SAVEFILE:
		{
			return fileSaveSpecific(reinterpret_cast<const wchar_t*>(lParam));
		}

		case NPPM_DOOPEN:
		{
			BufferID id = doOpen(reinterpret_cast<const wchar_t*>(lParam));
			if (id != BUFFER_INVALID)
				return switchToFile(id);
			return FALSE;
		}

		case NPPM_RELOADBUFFERID:
		{
			if (!wParam)
				return FALSE;
			return doReload(reinterpret_cast<BufferID>(wParam), lParam != 0);
		}

		case NPPM_RELOADFILE:
		{
			const wchar_t* pFilePath = reinterpret_cast<const wchar_t*>(lParam);
			if (!pFilePath)
				break;

			BufferID id = MainFileManager.getBufferFromName(pFilePath);
			if (id != BUFFER_INVALID)
				return doReload(id, wParam != 0);
			break;
		}

		case NPPM_SWITCHTOFILE:
		{
			BufferID id = MainFileManager.getBufferFromName(reinterpret_cast<const wchar_t*>(lParam));
			if (id != BUFFER_INVALID)
				return switchToFile(id);
			return FALSE;
		}

		case NPPM_ACTIVATEDOC:
		{
			int whichView = ((wParam != MAIN_VIEW) && (wParam != SUB_VIEW)) ? currentView() : static_cast<int>(wParam);
			int index = static_cast<int>(lParam);
			switchEditViewTo(whichView);
			activateDoc(index);
			return TRUE;
		}

		case NPPM_MAKECURRENTBUFFERDIRTY:
		{
			if (!_pEditView)
				return FALSE;
			Buffer* buf = _pEditView->getCurrentBuffer();
			if (!buf)
				return FALSE;
			buf->setDirty(true);
			return TRUE;
		}

		// ==================================================================
		// Menu and command execution
		// ==================================================================

		case NPPM_MENUCOMMAND:
		{
			// On Linux, command dispatch is not yet wired up for all menu IDs
			// TODO: Route through CommandHandler when available
			return TRUE;
		}

		case NPPM_SETMENUITEMCHECK:
		{
			// On Linux, menu item checking is handled differently
			// We just return TRUE for compatibility
			return TRUE;
		}

		// ==================================================================
		// Version and platform info
		// ==================================================================

		case NPPM_GETNPPVERSION:
		{
			const wchar_t* verStr = VERSION_INTERNAL_VALUE;
			wchar_t mainVerStr[16]{};
			wchar_t auxVerStr[16]{};
			bool isDot = false;
			int j = 0;
			int k = 0;
			for (int i = 0; verStr[i]; ++i)
			{
				if (verStr[i] == '.')
				{
					isDot = true;
				}
				else
				{
					if (!isDot)
						mainVerStr[j++] = verStr[i];
					else
						auxVerStr[k++] = verStr[i];
				}
			}
			mainVerStr[j] = '\0';
			auxVerStr[k] = '\0';

			bool addZeroPadding = wParam == TRUE;
			if (addZeroPadding)
			{
				size_t nbDigit = wcslen(auxVerStr);
				if (nbDigit > 0 && nbDigit <= 3)
				{
					if (nbDigit == 2)
					{
						auxVerStr[2] = '0';
						auxVerStr[3] = '\0';
					}
					else if (nbDigit == 1)
					{
						auxVerStr[1] = '0';
						auxVerStr[2] = '0';
						auxVerStr[3] = '\0';
					}
				}
			}

			int mainVer = 0, auxVer = 0;
			if (mainVerStr[0])
			{
				wchar_t* endPtr = nullptr;
				mainVer = static_cast<int>(wcstol(mainVerStr, &endPtr, 10));
			}
			if (auxVerStr[0])
			{
				wchar_t* endPtr = nullptr;
				auxVer = static_cast<int>(wcstol(auxVerStr, &endPtr, 10));
			}

			return MAKELONG(auxVer, mainVer);
		}

		case NPPM_GETWINDOWSVERSION:
		{
			// On Linux, return a value indicating it's not Windows
			// Plugins checking this should understand they're on Linux
			return 0; // WV_UNKNOWN
		}

		// ==================================================================
		// Plugin configuration paths
		// ==================================================================

		case NPPM_GETPLUGINSCONFIGDIR:
		{
			std::wstring userPluginConfDir = nppParam.getUserPluginConfDir();
			if (lParam != 0)
			{
				if (wParam == 0 || userPluginConfDir.length() >= static_cast<size_t>(wParam))
					return 0;
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, userPluginConfDir.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = L'\0';
				return TRUE;
			}
			return userPluginConfDir.length();
		}

		case NPPM_GETPLUGINHOMEPATH:
		{
			std::wstring pluginHomePath = nppParam.getPluginRootDir();
			if (lParam != 0)
			{
				if (wParam == 0 || pluginHomePath.length() >= static_cast<size_t>(wParam))
					return 0;
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, pluginHomePath.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = L'\0';
			}
			return pluginHomePath.length();
		}

		case NPPM_GETSETTINGSONCLOUDPATH:
		{
			const NppGUI& nppGUI = nppParam.getNppGUI();
			std::wstring settingsOnCloudPath = nppGUI._cloudPath;
			if (lParam != 0)
			{
				if (wParam == 0 || settingsOnCloudPath.length() >= static_cast<size_t>(wParam))
					return 0;
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, settingsOnCloudPath.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = L'\0';
			}
			return settingsOnCloudPath.length();
		}

		case NPPM_GETNPPSETTINGSDIRPATH:
		{
			std::wstring settingsDirPath = nppParam.getUserPath();
			if (lParam != 0)
			{
				if (wParam == 0 || settingsDirPath.length() >= static_cast<size_t>(wParam))
					return 0;
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, settingsDirPath.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = L'\0';
			}
			return settingsDirPath.length();
		}

		// ==================================================================
		// Language info
		// ==================================================================

		case NPPM_GETLANGUAGENAME:
		{
			Lang* pLang = nppParam.getLangFromID(static_cast<LangType>(wParam));
			if (!pLang)
				return 0;
			if (lParam)
			{
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, pLang->_langName.c_str(), MAX_PATH - 1);
				dest[MAX_PATH - 1] = L'\0';
			}
			return pLang->_langName.length();
		}

		case NPPM_GETLANGUAGEDESC:
		{
			// On Linux, return the language name as the description (full descriptions not available)
			Lang* pLang = nppParam.getLangFromID(static_cast<LangType>(wParam));
			if (!pLang)
				return 0;
			if (lParam)
			{
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, pLang->_langName.c_str(), MAX_PATH - 1);
				dest[MAX_PATH - 1] = L'\0';
			}
			return pLang->_langName.length();
		}

		case NPPM_GETCURRENTNATIVELANGENCODING:
		{
			return _nativeLangSpeaker.getLangEncoding();
		}

		case NPPM_GETNATIVELANGFILENAME:
		{
			const char* fn = _nativeLangSpeaker.getFileName();
			if (!fn)
				return 0;

			std::string fileName = fn;
			if (lParam != 0)
			{
				if (wParam == 0 || fileName.length() >= static_cast<size_t>(wParam))
					return 0;
				char* dest = reinterpret_cast<char*>(lParam);
				strncpy(dest, fileName.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = '\0';
			}
			return fileName.length();
		}

		// ==================================================================
		// Shortcut and resource allocation
		// ==================================================================

		case NPPM_GETSHORTCUTBYCMDID:
		{
			int cmdID = static_cast<int>(wParam);
			ShortcutKey* sk = reinterpret_cast<ShortcutKey*>(lParam);
			return _pluginsManager.getShortcutByCmdID(cmdID, sk);
		}

		case NPPM_REMOVESHORTCUTBYCMDID:
		{
			int cmdID = static_cast<int>(wParam);
			return _pluginsManager.removeShortcutByCmdID(cmdID);
		}

		case NPPM_ALLOCATESUPPORTED_DEPRECATED:
		{
			return TRUE;
		}

		case NPPM_ALLOCATECMDID:
		{
			return _pluginsManager.allocateCmdID(static_cast<int>(wParam), reinterpret_cast<int*>(lParam));
		}

		case NPPM_ALLOCATEMARKER:
		{
			return _pluginsManager.allocateMarker(static_cast<int>(wParam), reinterpret_cast<int*>(lParam));
		}

		case NPPM_ALLOCATEINDICATOR:
		{
			return _pluginsManager.allocateIndicator(static_cast<int>(wParam), reinterpret_cast<int*>(lParam));
		}

		// ==================================================================
		// Plugin messaging
		// ==================================================================

		case NPPM_MSGTOPLUGIN:
		{
			return _pluginsManager.relayPluginMessages(message, wParam, lParam);
		}

		// ==================================================================
		// Status bar
		// ==================================================================

		case NPPM_SETSTATUSBAR:
		{
			wchar_t* str2set = reinterpret_cast<wchar_t*>(lParam);
			if (!str2set || !str2set[0])
				return FALSE;

			switch (wParam)
			{
				case STATUSBAR_DOC_TYPE:
				case STATUSBAR_DOC_SIZE:
				case STATUSBAR_CUR_POS:
				case STATUSBAR_EOF_FORMAT:
				case STATUSBAR_UNICODE_TYPE:
				case STATUSBAR_TYPING_MODE:
					_statusBar.setText(str2set, static_cast<int>(wParam));
					return TRUE;
				default:
					return FALSE;
			}
		}

		// ==================================================================
		// Docking dialog management
		// ==================================================================

		case NPPM_DMMSHOW:
		{
			_dockingManager.showDockableDlg(reinterpret_cast<HWND>(lParam), SW_SHOW);
			return TRUE;
		}

		case NPPM_DMMHIDE:
		{
			_dockingManager.showDockableDlg(reinterpret_cast<HWND>(lParam), SW_HIDE);
			return TRUE;
		}

		case NPPM_DMMUPDATEDISPINFO:
		{
			_dockingManager.updateContainerInfo(reinterpret_cast<HWND>(lParam));
			return TRUE;
		}

		case NPPM_DMMREGASDCKDLG:
		{
			// Docking manager registration not yet supported on Linux
			return TRUE;
		}

		case NPPM_DMMVIEWOTHERTAB:
		{
			_dockingManager.showDockableDlg(reinterpret_cast<wchar_t*>(lParam), SW_SHOW);
			return TRUE;
		}

		case NPPM_DMMGETPLUGINHWNDBYNAME:
		{
			if (!lParam)
				return static_cast<LRESULT>(NULL);

			// On Linux, DockingCont is not fully implemented yet
			// Return NULL for now - plugins will need to handle this gracefully
			return static_cast<LRESULT>(NULL);
		}

		// ==================================================================
		// Modeless dialog tracking
		// ==================================================================

		case NPPM_MODELESSDIALOG:
		{
			if (wParam == MODELESSDIALOGADD)
			{
				for (size_t i = 0, len = _hModelessDlgs.size(); i < len; ++i)
				{
					if (_hModelessDlgs[i] == reinterpret_cast<HWND>(lParam))
						return static_cast<LRESULT>(NULL);
				}
				_hModelessDlgs.push_back(reinterpret_cast<HWND>(lParam));
				return lParam;
			}
			else if (wParam == MODELESSDIALOGREMOVE)
			{
				for (size_t i = 0, len = _hModelessDlgs.size(); i < len; ++i)
				{
					if (_hModelessDlgs[i] == reinterpret_cast<HWND>(lParam))
					{
						_hModelessDlgs.erase(_hModelessDlgs.begin() + i);
						return static_cast<LRESULT>(NULL);
					}
				}
				return lParam;
			}
			return TRUE;
		}

		// ==================================================================
		// Dark mode support
		// ==================================================================

		case NPPM_ISDARKMODEENABLED:
		{
			return NppDarkMode::isEnabled();
		}

		case NPPM_GETDARKMODECOLORS:
		{
			if (static_cast<size_t>(wParam) != sizeof(NppDarkMode::Colors))
				return static_cast<LRESULT>(false);

			NppDarkMode::Colors* currentColors = reinterpret_cast<NppDarkMode::Colors*>(lParam);
			if (currentColors != nullptr)
			{
				currentColors->background = NppDarkMode::getBackgroundColor();
				currentColors->softerBackground = NppDarkMode::getCtrlBackgroundColor();
				currentColors->hotBackground = NppDarkMode::getHotBackgroundColor();
				currentColors->pureBackground = NppDarkMode::getDlgBackgroundColor();
				currentColors->errorBackground = NppDarkMode::getErrorBackgroundColor();
				currentColors->text = NppDarkMode::getTextColor();
				currentColors->darkerText = NppDarkMode::getDarkerTextColor();
				currentColors->disabledText = NppDarkMode::getDisabledTextColor();
				currentColors->linkText = NppDarkMode::getLinkTextColor();
				currentColors->edge = NppDarkMode::getEdgeColor();
				currentColors->hotEdge = NppDarkMode::getHotEdgeColor();
				currentColors->disabledEdge = NppDarkMode::getDisabledEdgeColor();
				return static_cast<LRESULT>(true);
			}
			return static_cast<LRESULT>(false);
		}

		case NPPM_DARKMODESUBCLASSANDTHEME:
		{
			// On Linux with Qt, dark mode theming is handled by the platform
			// Return success for compatibility
			return static_cast<LRESULT>(true);
		}

		// ==================================================================
		// Editor default colors
		// ==================================================================

		case NPPM_GETEDITORDEFAULTFOREGROUNDCOLOR:
		{
			return nppParam.getCurrentDefaultFgColor();
		}

		case NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR:
		{
			return nppParam.getCurrentDefaultBgColor();
		}

		// ==================================================================
		// UI visibility queries
		// ==================================================================

		case NPPM_HIDETABBAR:
		{
			bool hide = (lParam != 0);
			NppGUI& nppGUI = nppParam.getNppGUI();
			bool oldVal = (nppGUI._tabStatus & TAB_HIDE);
			if (hide == oldVal)
				return oldVal;
			if (hide)
				nppGUI._tabStatus |= TAB_HIDE;
			else
				nppGUI._tabStatus &= ~TAB_HIDE;
			return oldVal;
		}

		case NPPM_ISTABBARHIDDEN:
		{
			NppGUI& nppGUI = nppParam.getNppGUI();
			return (nppGUI._tabStatus & TAB_HIDE) != 0;
		}

		case NPPM_HIDETOOLBAR:
		{
			// On Linux, toolbar visibility is handled by Qt
			return FALSE;
		}

		case NPPM_ISTOOLBARHIDDEN:
		{
			return FALSE;
		}

		case NPPM_HIDEMENU:
		{
			// On Linux, menu visibility is handled by Qt
			return FALSE;
		}

		case NPPM_ISMENUHIDDEN:
		{
			return FALSE;
		}

		case NPPM_HIDESTATUSBAR:
		{
			bool show = (lParam != TRUE);
			NppGUI& nppGUI = nppParam.getNppGUI();
			bool oldVal = nppGUI._statusBarShow;
			if (show == oldVal)
				return oldVal;
			nppGUI._statusBarShow = show;
			return oldVal;
		}

		case NPPM_ISSTATUSBARHIDDEN:
		{
			const NppGUI& nppGUI = nppParam.getNppGUI();
			return !nppGUI._statusBarShow;
		}

		// ==================================================================
		// User language
		// ==================================================================

		case NPPM_GETNBUSERLANG:
		{
			if (lParam)
				*(reinterpret_cast<int*>(lParam)) = IDM_LANG_USER;
			return nppParam.getNbUserLang();
		}

		// ==================================================================
		// Bookmarks
		// ==================================================================

		case NPPM_GETBOOKMARKID:
		{
			return MARK_BOOKMARK;
		}

		// ==================================================================
		// Macro status
		// ==================================================================

		case NPPM_GETCURRENTMACROSTATUS:
		{
			if (_recordingMacro)
				return static_cast<LRESULT>(MacroStatus::RecordInProgress);
			if (_playingBackMacro)
				return static_cast<LRESULT>(MacroStatus::PlayingBack);
			return (_macro.empty()) ? static_cast<LRESULT>(MacroStatus::Idle) : static_cast<LRESULT>(MacroStatus::RecordingStopped);
		}

		// ==================================================================
		// Command line
		// ==================================================================

		case NPPM_GETCURRENTCMDLINE:
		{
			std::wstring cmdLineString = nppParam.getCmdLineString();
			if (lParam != 0)
			{
				if (wParam == 0 || cmdLineString.length() >= static_cast<size_t>(wParam))
					return 0;
				wchar_t* dest = reinterpret_cast<wchar_t*>(lParam);
				wcsncpy(dest, cmdLineString.c_str(), static_cast<size_t>(wParam) - 1);
				dest[wParam - 1] = L'\0';
			}
			return cmdLineString.length();
		}

		// ==================================================================
		// Auto-indent
		// ==================================================================

		case NPPM_ISAUTOINDENTON:
		{
			return nppParam.getNppGUI()._maintainIndent;
		}

		case NPPM_GETEXTERNALLEXERAUTOINDENTMODE:
		{
			int index = nppParam.getExternalLangIndexFromName(reinterpret_cast<wchar_t*>(wParam));
			if (index < 0)
				return FALSE;
			*(reinterpret_cast<ExternalLexerAutoIndentMode*>(lParam)) = nppParam.getELCFromIndex(index)->_autoIndentMode;
			return TRUE;
		}

		case NPPM_SETEXTERNALLEXERAUTOINDENTMODE:
		{
			int index = nppParam.getExternalLangIndexFromName(reinterpret_cast<wchar_t*>(wParam));
			if (index < 0)
				return FALSE;
			nppParam.getELCFromIndex(index)->_autoIndentMode = static_cast<ExternalLexerAutoIndentMode>(lParam);
			return TRUE;
		}

		// ==================================================================
		// Document list / Panels
		// ==================================================================

		case NPPM_SHOWDOCLIST:
		{
			BOOL toShow = static_cast<BOOL>(lParam);
			if (toShow)
			{
				launchDocumentListPanel();
			}
			else
			{
				auto* mainWin = getMainWindowFromEditView(_mainEditView);
				if (mainWin)
				{
					auto* dockMgr = mainWin->getDockingManager();
					if (dockMgr && dockMgr->isPanelVisible("documentList"))
						dockMgr->hidePanel("documentList");
				}
			}
			return TRUE;
		}

		case NPPM_ISDOCLISTSHOWN:
		{
			auto* mainWin = getMainWindowFromEditView(_mainEditView);
			if (!mainWin)
				return FALSE;
			auto* dockMgr = mainWin->getDockingManager();
			if (!dockMgr)
				return FALSE;
			return dockMgr->isPanelVisible("documentList");
		}

		case NPPM_DOCLISTDISABLEEXTCOLUMN:
		case NPPM_DOCLISTDISABLEPATHCOLUMN:
		{
			BOOL isOff = static_cast<BOOL>(lParam);
			NppGUI& nppGUI = nppParam.getNppGUI();

			if (message == NPPM_DOCLISTDISABLEEXTCOLUMN)
				nppGUI._fileSwitcherWithoutExtColumn = isOff == TRUE;
			else
				nppGUI._fileSwitcherWithoutPathColumn = isOff == TRUE;

			// Reload not yet supported via DockingManager on Linux
			return TRUE;
		}

		// ==================================================================
		// Scintilla handles
		// ==================================================================

		case NPPM_CREATESCINTILLAHANDLE:
		{
			return reinterpret_cast<LRESULT>(_scintillaCtrls4Plugins.createSintilla(lParam ? reinterpret_cast<HWND>(lParam) : nullptr));
		}

		case NPPM_DESTROYSCINTILLAHANDLE_DEPRECATED:
		{
			// Do nothing - kept for compatibility (see Windows version comment)
			return TRUE;
		}

		case NPPM_CREATELEXER:
		{
			// On Linux, create a lexer by name
			const wchar_t* lexerNameW = reinterpret_cast<wchar_t*>(lParam);
			if (!lexerNameW)
				return 0;

			// Convert wide string to narrow for Lexilla
			char lexerName[256]{};
			size_t len = wcstombs(lexerName, lexerNameW, sizeof(lexerName) - 1);
			if (len == static_cast<size_t>(-1))
				return 0;

			return reinterpret_cast<LRESULT>(CreateLexer(lexerName));
		}

		// ==================================================================
		// Menu handle
		// ==================================================================

		case NPPM_GETMENUHANDLE:
		{
			if (wParam == NPPPLUGINMENU)
				return reinterpret_cast<LRESULT>(_pluginsManager.getMenuHandle());
			else if (wParam == NPPMAINMENU)
				return reinterpret_cast<LRESULT>(_mainMenuHandle);
			else
				return static_cast<LRESULT>(NULL);
		}

		// ==================================================================
		// Toolbar icons
		// ==================================================================

		case NPPM_ADDTOOLBARICON_DEPRECATED:
		case NPPM_ADDTOOLBARICON_FORDARKMODE:
		{
			// On Linux, toolbar icon registration is handled differently
			// Accept the call but don't do anything Windows-specific
			return TRUE;
		}

		case NPPM_GETTOOLBARICONSETCHOICE:
		{
			return 0; // Default icon set
		}

		// ==================================================================
		// Line number width
		// ==================================================================

		case NPPM_SETLINENUMBERWIDTHMODE:
		{
			if (lParam != LINENUMWIDTH_DYNAMIC && lParam != LINENUMWIDTH_CONSTANT)
				return FALSE;

			ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
			svp._lineNumberMarginDynamicWidth = lParam == LINENUMWIDTH_DYNAMIC;
			return TRUE;
		}

		case NPPM_GETLINENUMBERWIDTHMODE:
		{
			const ScintillaViewParams& svp = nppParam.getSVP();
			return svp._lineNumberMarginDynamicWidth ? LINENUMWIDTH_DYNAMIC : LINENUMWIDTH_CONSTANT;
		}

		// ==================================================================
		// Smooth font
		// ==================================================================

		case NPPM_SETSMOOTHFONT:
		{
			int param = (lParam == 0 ? SC_EFF_QUALITY_DEFAULT : SC_EFF_QUALITY_LCD_OPTIMIZED);
			_mainEditView.execute(SCI_SETFONTQUALITY, param);
			_subEditView.execute(SCI_SETFONTQUALITY, param);
			return TRUE;
		}

		// ==================================================================
		// Editor border edge
		// ==================================================================

		case NPPM_SETEDITORBORDEREDGE:
		{
			// Border edge styling not applicable on Linux/Qt
			return TRUE;
		}

		// ==================================================================
		// Session management
		// ==================================================================

		case NPPM_LOADSESSION:
		{
			fileLoadSession(reinterpret_cast<const wchar_t*>(lParam));
			return TRUE;
		}

		case NPPM_SAVECURRENTSESSION:
		{
			return reinterpret_cast<LRESULT>(fileSaveSession(0, NULL, reinterpret_cast<const wchar_t*>(lParam)));
		}

		case NPPM_SAVESESSION:
		{
			sessionInfo* pSi = reinterpret_cast<sessionInfo*>(lParam);
			return reinterpret_cast<LRESULT>(fileSaveSession(pSi->nbFile, pSi->files, pSi->sessionFilePathName));
		}

		case NPPM_GETNBSESSIONFILES:
		{
			size_t nbSessionFiles = 0;
			const wchar_t* sessionFileName = reinterpret_cast<const wchar_t*>(lParam);
			BOOL* pbIsValidXML = reinterpret_cast<BOOL*>(wParam);
			if (pbIsValidXML)
				*pbIsValidXML = false;
			if (sessionFileName && (sessionFileName[0] != '\0'))
			{
				Session session2Load;
				if (nppParam.loadSession(session2Load, sessionFileName, true))
				{
					if (pbIsValidXML)
						*pbIsValidXML = true;
					nbSessionFiles = session2Load.nbMainFiles() + session2Load.nbSubFiles();
				}
			}
			return nbSessionFiles;
		}

		case NPPM_GETSESSIONFILES:
		{
			const wchar_t* sessionFileName = reinterpret_cast<const wchar_t*>(lParam);
			wchar_t** sessionFileArray = reinterpret_cast<wchar_t**>(wParam);

			if (!sessionFileName || sessionFileName[0] == '\0' || !sessionFileArray)
				return FALSE;

			Session session2Load;
			if (nppParam.loadSession(session2Load, sessionFileName, true))
			{
				size_t i = 0;
				for (; i < session2Load.nbMainFiles();)
				{
					const wchar_t* pFn = session2Load._mainViewFiles[i]._fileName.c_str();
					wcsncpy(sessionFileArray[i], pFn, MAX_PATH - 1);
					sessionFileArray[i][MAX_PATH - 1] = L'\0';
					++i;
				}

				for (size_t j = 0, len = session2Load.nbSubFiles(); j < len; ++j)
				{
					const wchar_t* pFn = session2Load._subViewFiles[j]._fileName.c_str();
					wcsncpy(sessionFileArray[i], pFn, MAX_PATH - 1);
					sessionFileArray[i][MAX_PATH - 1] = L'\0';
					++i;
				}
				return TRUE;
			}
			return FALSE;
		}

		// ==================================================================
		// Tab color
		// ==================================================================

		case NPPM_GETTABCOLORID:
		{
			const auto view = static_cast<int>(wParam);
			auto tabIndex = static_cast<int>(lParam);

			auto colorId = -1;
			auto pDt = _pDocTab;
			if (view == MAIN_VIEW)
				pDt = &_mainDocTab;
			else if (view == SUB_VIEW)
				pDt = &_subDocTab;

			if (tabIndex == -1)
				tabIndex = pDt->getCurrentTabIndex();

			if ((tabIndex >= 0) && (tabIndex < static_cast<int>(pDt->nbItem())))
				colorId = pDt->getIndividualTabColourId(tabIndex);

			return colorId;
		}

		// ==================================================================
		// Untitled tab naming
		// ==================================================================

		case NPPM_SETUNTITLEDNAME:
		{
			// Renaming untitled tabs not yet supported on Linux
			return FALSE;
		}

		// ==================================================================
		// App data plugins
		// ==================================================================

		case NPPM_GETAPPDATAPLUGINSALLOWED:
		{
			const wchar_t* appDataNpp = nppParam.getAppDataNppDir();
			if (appDataNpp[0])
				return TRUE;
			return FALSE;
		}

		// ==================================================================
		// Auto update
		// ==================================================================

		case NPPM_DISABLEAUTOUPDATE:
		{
			NppGUI& nppGUI = nppParam.getNppGUI();
			nppGUI._autoUpdateOpt._doAutoUpdate = NppGUI::autoupdate_disabled;
			return TRUE;
		}

		// ==================================================================
		// Deprecated open file name queries (still supported for compatibility)
		// ==================================================================

		case NPPM_GETOPENFILENAMESPRIMARY_DEPRECATED:
		case NPPM_GETOPENFILENAMESSECOND_DEPRECATED:
		case NPPM_GETOPENFILENAMES_DEPRECATED:
		{
			if (!wParam)
				return 0;

			wchar_t** fileNames = reinterpret_cast<wchar_t**>(wParam);
			size_t nbFileNames = static_cast<size_t>(lParam);

			size_t j = 0;
			if (message != NPPM_GETOPENFILENAMESSECOND_DEPRECATED)
			{
				for (size_t i = 0; i < _mainDocTab.nbItem() && j < nbFileNames; ++i)
				{
					BufferID id = _mainDocTab.getBufferByIndex(i);
					Buffer* buf = MainFileManager.getBufferByID(id);
					if (!buf)
						continue;
					const wchar_t* path = buf->getFullPathName();
					if (path)
					{
						wcsncpy(fileNames[j], path, MAX_PATH - 1);
						fileNames[j][MAX_PATH - 1] = L'\0';
					}
					++j;
				}
			}

			if (message != NPPM_GETOPENFILENAMESPRIMARY_DEPRECATED)
			{
				for (size_t i = 0; i < _subDocTab.nbItem() && j < nbFileNames; ++i)
				{
					BufferID id = _subDocTab.getBufferByIndex(i);
					Buffer* buf = MainFileManager.getBufferByID(id);
					if (!buf)
						continue;
					const wchar_t* path = buf->getFullPathName();
					if (path)
					{
						wcsncpy(fileNames[j], path, MAX_PATH - 1);
						fileNames[j][MAX_PATH - 1] = L'\0';
					}
					++j;
				}
			}
			return j;
		}

		// ==================================================================
		// Find in Files dialog
		// ==================================================================

		case NPPM_LAUNCHFINDINFILESDLG:
		{
			// Find in Files dialog launch not yet fully supported on Linux
			return TRUE;
		}

		// ==================================================================
		// SCN modified flags
		// ==================================================================

		case NPPM_ADDSCNMODIFIEDFLAGS:
		{
			nppParam.addScintillaModEventMask(static_cast<unsigned long>(lParam));
			auto newModEventMask = nppParam.getScintillaModEventMask();
			_mainEditView.execute(SCI_SETMODEVENTMASK, newModEventMask);
			_subEditView.execute(SCI_SETMODEVENTMASK, newModEventMask);
			return TRUE;
		}

		// ==================================================================
		// Encode/Decode (kept for compatibility but mostly no-op on Linux)
		// ==================================================================

		case NPPM_ENCODESCI:
		case NPPM_DECODESCI:
		{
			// Encoding conversion is handled differently on Linux
			// Return -1 to indicate not supported
			return -1;
		}

		// ==================================================================
		// Trigger tab bar context menu
		// ==================================================================

		case NPPM_TRIGGERTABBARCONTEXTMENU:
		{
			int whichView = ((wParam != MAIN_VIEW) && (wParam != SUB_VIEW)) ? currentView() : static_cast<int>(wParam);
			int index = static_cast<int>(lParam);
			switchEditViewTo(whichView);
			activateDoc(index);
			// On Linux, context menu triggering would need Qt-specific handling
			return TRUE;
		}

		// ==================================================================
		// Deprecated functions that still return TRUE for compatibility
		// ==================================================================

		case NPPM_GETENABLETHEMETEXTUREFUNC_DEPRECATED:
		{
			return 0; // No theme texture function on Linux
		}

		default:
		{
			// Relay unhandled messages to plugins
			_pluginsManager.relayNppMessages(message, wParam, lParam);
			return 0;
		}
	}

	// For messages that use 'break' (to allow relay to other plugins)
	_pluginsManager.relayNppMessages(message, wParam, lParam);
	return 0;
}

#endif // NPP_LINUX
