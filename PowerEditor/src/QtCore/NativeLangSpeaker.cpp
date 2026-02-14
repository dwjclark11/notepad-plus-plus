// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

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
// NativeLangSpeaker.cpp - Linux/Qt implementation
// ============================================================================
// This file provides Linux-compatible implementations of NativeLangSpeaker
// methods that are guarded by #ifndef NPP_LINUX in localization.cpp.
// It also adds Qt-specific methods for translating Qt UI elements.
// ============================================================================

#include "localization.h"
#include "Common.h"
#include "NppXml.h"
#include "Parameters.h"
#include "menuCmdID.h"
#include "resource.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QGroupBox>
#include <QAbstractButton>

using namespace std;

// ============================================================================
// Methods guarded by #ifndef NPP_LINUX in localization.cpp
// These are re-implemented here for the Linux/Qt build.
// ============================================================================

// searchDlgNode - recursive XML search (platform-independent)
NppXml::Node NativeLangSpeaker::searchDlgNode(NppXml::Node node, const char* dlgTagName)
{
	NppXml::Node dlgNode = NppXml::firstChildElement(node, dlgTagName);
	if (dlgNode) return dlgNode;
	for (NppXml::Node childNode = NppXml::firstChildElement(node, nullptr);
		childNode;
		childNode = NppXml::nextSibling(childNode))
	{
		dlgNode = searchDlgNode(childNode, dlgTagName);
		if (dlgNode) return dlgNode;
	}
	return {};
}

// getShortcutMapperLangStr - read shortcut mapper translation from XML
wstring NativeLangSpeaker::getShortcutMapperLangStr(const char *nodeName, const wchar_t* defaultStr) const
{
	if (!_nativeLang) return defaultStr;

	NppXml::Node targetNode = NppXml::firstChildElement(_nativeLang, "Dialog");
	if (!targetNode) return defaultStr;

	targetNode = NppXml::firstChildElement(targetNode, "ShortcutMapper");
	if (!targetNode) return defaultStr;

	targetNode = NppXml::firstChildElement(targetNode, nodeName);
	if (!targetNode) return defaultStr;

	const char* name = NppXml::attribute(NppXml::toElement(targetNode), "name");
	if (name && name[0])
	{
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		return wmc.char2wchar(name, _nativeLangEncoding);
	}

	return defaultStr;
}

// changeShortcutLang - translate shortcut names from XML (platform-independent data operation)
void NativeLangSpeaker::changeShortcutLang() const
{
	if (!_nativeLang) return;

	NppParameters& nppParam = NppParameters::getInstance();
	vector<CommandShortcut>& mainshortcuts = nppParam.getUserShortcuts();
	vector<ScintillaKeyMap>& scinshortcuts = nppParam.getScintillaKeyList();

	NppXml::Node shortcuts = NppXml::firstChildElement(_nativeLang, "Shortcuts");
	if (!shortcuts) return;

	shortcuts = NppXml::firstChildElement(shortcuts, "Main");
	if (!shortcuts) return;

	NppXml::Node entriesRoot = NppXml::firstChildElement(shortcuts, "Entries");
	if (!entriesRoot) return;

	for (NppXml::Node childNode = NppXml::firstChildElement(entriesRoot, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int index = NppXml::intAttribute(element, "index", -1);
		const int id = NppXml::intAttribute(element, "id", -1);
		if (index > 0 && id > 0)
		{
			if (static_cast<size_t>(index) < mainshortcuts.size())
			{
				const char* name = NppXml::attribute(element, "name");
				CommandShortcut& csc = mainshortcuts[index];
				if (csc.getID() == (unsigned long)id)
				{
					csc.setName(name);
				}
			}
		}
	}

	// Scintilla shortcuts
	shortcuts = NppXml::firstChildElement(_nativeLang, "Shortcuts");
	if (!shortcuts) return;

	shortcuts = NppXml::firstChildElement(shortcuts, "Scintilla");
	if (!shortcuts) return;

	entriesRoot = NppXml::firstChildElement(shortcuts, "Entries");
	if (!entriesRoot) return;

	for (NppXml::Node childNode = NppXml::firstChildElement(entriesRoot, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int index = NppXml::intAttribute(element, "index", -2);
		if (index > -1 && static_cast<size_t>(index) < scinshortcuts.size())
		{
			const char* name = NppXml::attribute(element, "name");
			ScintillaKeyMap& skm = scinshortcuts[index];
			skm.setName(name);
		}
	}
}

// getDoSaveOrNotStrings - read "Do Save Or Not" dialog strings from XML
bool NativeLangSpeaker::getDoSaveOrNotStrings(wstring& title, wstring& msg)
{
	if (!_nativeLang) return false;

	NppXml::Node dlgNode = NppXml::firstChildElement(_nativeLang, "Dialog");
	if (!dlgNode) return false;

	dlgNode = searchDlgNode(dlgNode, "DoSaveOrNot");
	if (!dlgNode) return false;

	const char* title2set = NppXml::attribute(NppXml::toElement(dlgNode), "title");
	if (!title2set || !title2set[0]) return false;

	WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
	const wchar_t* titleW = wmc.char2wchar(title2set, _nativeLangEncoding);
	title = titleW;

	for (NppXml::Node childNode = NppXml::firstChildElement(dlgNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int id = NppXml::intAttribute(element, "id", -1);
		const char* name = NppXml::attribute(element, "name");
		if (id > 0 && (name && name[0]))
		{
			if (id == IDC_DOSAVEORNOTTEXT)
			{
				const wchar_t* msgW = wmc.char2wchar(name, _nativeLangEncoding);
				msg = msgW;
				return true;
			}
		}
	}
	return false;
}

// changeDlgLang - translate dialog elements
// On Linux/Qt, HWND is void* and Win32 API calls are no-ops.
// This provides the same interface but the actual Qt translation
// is handled by changeDlgLangQt() below.
bool NativeLangSpeaker::changeDlgLang(HWND hDlg, const char* dlgTagName, char* title, size_t titleMaxSize)
{
	if (title)
		title[0] = '\0';

	if (!_nativeLang) return false;

	NppXml::Node dlgNode = NppXml::firstChildElement(_nativeLang, "Dialog");
	if (!dlgNode) return false;

	dlgNode = searchDlgNode(dlgNode, dlgTagName);
	if (!dlgNode) return false;

	// Extract title for caller even if we can't set it on the window
	const char* title2set = NppXml::attribute(NppXml::toElement(dlgNode), "title");
	if (title2set && title2set[0] && title && titleMaxSize)
	{
		strncpy(title, title2set, titleMaxSize - 1);
	}

	return true;
}

// getMsgBoxLang - read message box translation from XML
bool NativeLangSpeaker::getMsgBoxLang(const char* msgBoxTagName, wstring& title, wstring& message)
{
	title = L"";
	message = L"";

	if (!_nativeLang) return false;

	NppXml::Node msgBoxNode = NppXml::firstChildElement(_nativeLang, "MessageBox");
	if (!msgBoxNode) return false;

	msgBoxNode = searchDlgNode(msgBoxNode, msgBoxTagName);
	if (!msgBoxNode) return false;

	WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();

	const char* titre = NppXml::attribute(NppXml::toElement(msgBoxNode), "title");
	const char* msg = NppXml::attribute(NppXml::toElement(msgBoxNode), "message");
	if ((titre && titre[0]) && (msg && msg[0]))
	{
		title = wmc.char2wchar(titre, _nativeLangEncoding);
		message = wmc.char2wchar(msg, _nativeLangEncoding);
		return true;
	}
	return false;
}

// getDlgLangMenuStr - get translated menu string from dialog node
wstring NativeLangSpeaker::getDlgLangMenuStr(const char* firstLevelNodeName, const char* secondLevelNodeName, int cmdID, const wchar_t* defaultStr) const
{
	if (!_nativeLang) return defaultStr;

	NppXml::Node targetNode = NppXml::firstChildElement(_nativeLang, firstLevelNodeName);
	if (!targetNode) return defaultStr;

	if (secondLevelNodeName && secondLevelNodeName[0])
	{
		targetNode = NppXml::firstChildElement(targetNode, secondLevelNodeName);
		if (!targetNode) return defaultStr;
	}

	targetNode = NppXml::firstChildElement(targetNode, "Menu");
	if (!targetNode) return defaultStr;

	const char* name = nullptr;
	for (NppXml::Node childNode = NppXml::firstChildElement(targetNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int id = NppXml::intAttribute(element, "id", -1);
		if (id > 0 && id == cmdID)
		{
			name = NppXml::attribute(element, "name");
			break;
		}
	}

	if (name && name[0])
	{
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		return wmc.char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

// getCmdLangStr - get translated command string from nested nodes
wstring NativeLangSpeaker::getCmdLangStr(vector<const char*> nodeNames, int cmdID, const wchar_t* defaultStr) const
{
	if (!_nativeLang) return defaultStr;
	NppXml::Node targetNode = NppXml::firstChildElement(_nativeLang, nodeNames.at(0));
	if (targetNode == nullptr)
		return defaultStr;

	auto it = nodeNames.begin();
	++it;

	for (auto end = nodeNames.end(); it != end; ++it)
	{
		targetNode = NppXml::firstChildElement(targetNode, *it);
		if (targetNode == nullptr)
			return defaultStr;
	}

	if (targetNode == nullptr)
		return defaultStr;

	const char* name = nullptr;
	for (NppXml::Node childNode = NppXml::firstChildElement(targetNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int id = NppXml::intAttribute(element, "id", -1);
		if (id > 0 && id == cmdID)
		{
			name = NppXml::attribute(element, "name");
			break;
		}
	}

	if (name && name[0])
	{
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		return wmc.char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

// getProjectPanelLangMenuStr - get translated project panel menu string
wstring NativeLangSpeaker::getProjectPanelLangMenuStr(const char* nodeName, int cmdID, const wchar_t* defaultStr) const
{
	if (!_nativeLang) return defaultStr;

	NppXml::Node targetNode = NppXml::firstChildElement(_nativeLang, "ProjectManager");
	if (!targetNode) return defaultStr;

	targetNode = NppXml::firstChildElement(targetNode, "Menus");
	if (!targetNode) return defaultStr;

	targetNode = NppXml::firstChildElement(targetNode, nodeName);
	if (!targetNode) return defaultStr;

	const char* name = nullptr;
	for (NppXml::Node childNode = NppXml::firstChildElement(targetNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const int id = NppXml::intAttribute(element, "id", -1);
		if (id > 0 && id == cmdID)
		{
			name = NppXml::attribute(element, "name");
			break;
		}
	}

	if (name && name[0])
	{
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		return wmc.char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

// getAttrNameStr - get attribute name string from nested XML nodes
wstring NativeLangSpeaker::getAttrNameStr(const wchar_t* defaultStr, const char* nodeL1Name, const char* nodeL2Name, const char* nodeL3Name) const
{
	if (!_nativeLang) return defaultStr;

	NppXml::Node targetNode = NppXml::firstChildElement(_nativeLang, nodeL1Name);
	if (!targetNode) return defaultStr;
	if (nodeL2Name)
		targetNode = NppXml::firstChildElement(targetNode, nodeL2Name);

	if (!targetNode) return defaultStr;

	const char* name = NppXml::attribute(NppXml::toElement(targetNode), nodeL3Name);
	if (name && name[0])
	{
		WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
		return wmc.char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

// getAttrNameByIdStr - get attribute name by ID from XML items
wstring NativeLangSpeaker::getAttrNameByIdStr(const wchar_t* defaultStr, NppXml::Node targetNode, const char* nodeL1Value, const char* nodeL1Name, const char* nodeL2Name)
{
	if (!targetNode) return defaultStr;

	for (NppXml::Node childNode = NppXml::firstChildElement(targetNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const char* id = NppXml::attribute(element, nodeL1Name);
		if (id && id[0] && !strcmp(id, nodeL1Value))
		{
			const char* name = NppXml::attribute(element, nodeL2Name);
			if (name && name[0])
			{
				WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
				return wmc.char2wchar(name, _nativeLangEncoding);
			}
		}
	}
	return defaultStr;
}

// messageBox - show translated message box using QMessageBox
int NativeLangSpeaker::messageBox(const char* msgBoxTagName, HWND hWnd, const wchar_t* defaultMessage, const wchar_t* defaultTitle, int msgBoxType, int intInfo, const wchar_t* strInfo)
{
	(void)hWnd;

	if (NppParameters::getInstance().isEndSessionCritical())
		return IDCANCEL;

	wstring msg;
	wstring title;
	if (!getMsgBoxLang(msgBoxTagName, title, msg))
	{
		title = defaultTitle;
		msg = defaultMessage;
	}
	title = stringReplace(title, L"$INT_REPLACE$", to_wstring(intInfo));
	msg = stringReplace(msg, L"$INT_REPLACE$", to_wstring(intInfo));
	if (strInfo)
	{
		title = stringReplace(title, L"$STR_REPLACE$", strInfo);
		msg = stringReplace(msg, L"$STR_REPLACE$", strInfo);
	}

	// Convert wstring to QString for Qt
	QString qTitle = QString::fromStdWString(title);
	QString qMsg = QString::fromStdWString(msg);

	// Map Win32 MB_* types to QMessageBox
	QMessageBox::StandardButtons buttons = QMessageBox::Ok;
	QMessageBox::Icon icon = QMessageBox::NoIcon;

	// Extract button style (low nibble of msgBoxType)
	int btnStyle = msgBoxType & 0x0F;
	switch (btnStyle)
	{
		case 0: // MB_OK
			buttons = QMessageBox::Ok;
			break;
		case 1: // MB_OKCANCEL
			buttons = QMessageBox::Ok | QMessageBox::Cancel;
			break;
		case 2: // MB_ABORTRETRYIGNORE
			buttons = QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore;
			break;
		case 3: // MB_YESNOCANCEL
			buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
			break;
		case 4: // MB_YESNO
			buttons = QMessageBox::Yes | QMessageBox::No;
			break;
		case 5: // MB_RETRYCANCEL
			buttons = QMessageBox::Retry | QMessageBox::Cancel;
			break;
	}

	// Extract icon style (bits 4-7)
	int iconStyle = msgBoxType & 0xF0;
	switch (iconStyle)
	{
		case 0x10: // MB_ICONERROR / MB_ICONHAND / MB_ICONSTOP
			icon = QMessageBox::Critical;
			break;
		case 0x20: // MB_ICONQUESTION
			icon = QMessageBox::Question;
			break;
		case 0x30: // MB_ICONWARNING / MB_ICONEXCLAMATION
			icon = QMessageBox::Warning;
			break;
		case 0x40: // MB_ICONINFORMATION / MB_ICONASTERISK
			icon = QMessageBox::Information;
			break;
	}

	QMessageBox msgBox;
	msgBox.setIcon(icon);
	msgBox.setWindowTitle(qTitle.isEmpty() ? QString("Notepad++") : qTitle);
	msgBox.setText(qMsg);
	msgBox.setStandardButtons(buttons);

	if (_isRTL)
	{
		msgBox.setLayoutDirection(Qt::RightToLeft);
	}

	int result = msgBox.exec();

	// Map QMessageBox result back to Win32 ID* constants
	switch (result)
	{
		case QMessageBox::Ok:     return IDOK;
		case QMessageBox::Cancel: return IDCANCEL;
		case QMessageBox::Yes:    return IDYES;
		case QMessageBox::No:     return IDNO;
		case QMessageBox::Abort:  return IDABORT;
		case QMessageBox::Retry:  return IDRETRY;
		case QMessageBox::Ignore: return IDIGNORE;
		default:                  return IDCANCEL;
	}
}

// resizeCheckboxRadioBtn - no-op on Qt (layout management handles sizing)
void NativeLangSpeaker::resizeCheckboxRadioBtn(HWND hWnd)
{
	(void)hWnd;
}

// changePreferenceDlgLang - stub for Linux (PreferenceDlg not yet ported)
// The Preference dialog on Linux uses Qt's own layout and the translation
// can be applied when the dialog is fully ported.

// ============================================================================
// Qt-specific localization methods
// ============================================================================

// Helper: convert UTF-8 char* to QString
static QString charToQString(const char* str, int /*encoding*/)
{
	if (!str || !str[0]) return QString();
	return QString::fromUtf8(str);
}

void NativeLangSpeaker::changeMenuLangQt(QMenuBar* menuBar) const
{
	if (!menuBar) return;
	if (!_nativeLang) return;

	NppXml::Node mainMenu = NppXml::firstChildElement(_nativeLang, "Menu");
	if (!mainMenu) return;

	mainMenu = NppXml::firstChildElement(mainMenu, "Main");
	if (!mainMenu) return;

	// Phase 1: Translate top-level menu entries
	NppXml::Node entriesRoot = NppXml::firstChildElement(mainMenu, "Entries");
	if (entriesRoot)
	{
		// Build a map of menuId -> translated name
		struct MenuEntry
		{
			std::string menuId;
			QString translatedName;
		};
		std::vector<MenuEntry> entries;

		for (NppXml::Node childNode = NppXml::firstChildElement(entriesRoot, "Item");
			childNode;
			childNode = NppXml::nextSiblingElement(childNode, "Item"))
		{
			NppXml::Element element = NppXml::toElement(childNode);
			const char* menuIdStr = NppXml::attribute(element, "menuId");
			const char* name = NppXml::attribute(element, "name");
			if (menuIdStr && name)
			{
				const MenuPosition& menuPos = MenuPosition::getMenuPosition(menuIdStr);
				if (menuPos._x != -1)
				{
					// Translate the top-level menu at position _x
					QList<QAction*> actions = menuBar->actions();
					if (menuPos._x < actions.size())
					{
						QString translatedName = charToQString(name, _nativeLangEncoding);
						if (!translatedName.isEmpty())
						{
							actions[menuPos._x]->setText(translatedName);
						}
					}
				}
			}
		}
	}

	// Phase 2: Translate individual menu commands by ID
	NppXml::Node menuCommandsRoot = NppXml::firstChildElement(mainMenu, "Commands");
	if (menuCommandsRoot)
	{
		// Build a map of command ID -> translated name for fast lookup
		QMap<int, QString> commandTranslations;
		for (NppXml::Node childNode = NppXml::firstChildElement(menuCommandsRoot, "Item");
			childNode;
			childNode = NppXml::nextSiblingElement(childNode, "Item"))
		{
			NppXml::Element element = NppXml::toElement(childNode);
			int id = NppXml::intAttribute(element, "id", 0);
			const char* name = NppXml::attribute(element, "name");
			if (id > 0 && name && name[0])
			{
				commandTranslations[id] = charToQString(name, _nativeLangEncoding);
			}
		}

		// Walk all menus and translate actions that have matching command IDs
		if (!commandTranslations.isEmpty())
		{
			QList<QAction*> topActions = menuBar->actions();
			for (QAction* topAction : topActions)
			{
				QMenu* menu = topAction->menu();
				if (menu)
				{
					translateMenuActions(menu, commandTranslations);
				}
			}
		}
	}

	// Phase 3: Translate submenu folder entries
	NppXml::Node subEntriesRoot = NppXml::firstChildElement(mainMenu, "SubEntries");
	if (subEntriesRoot)
	{
		for (NppXml::Node childNode = NppXml::firstChildElement(subEntriesRoot, "Item");
			childNode;
			childNode = NppXml::nextSiblingElement(childNode, "Item"))
		{
			NppXml::Element element = NppXml::toElement(childNode);
			const char* subMenuIdStr = NppXml::attribute(element, "subMenuId");
			const char* name = NppXml::attribute(element, "name");

			if (!subMenuIdStr || !name) continue;

			const MenuPosition& menuPos = MenuPosition::getMenuPosition(subMenuIdStr);
			int x = menuPos._x;
			int y = menuPos._y;
			int z = menuPos._z;

			QList<QAction*> topActions = menuBar->actions();
			if (x < 0 || x >= topActions.size()) continue;

			QMenu* topMenu = topActions[x]->menu();
			if (!topMenu) continue;

			QList<QAction*> subActions = topMenu->actions();
			if (y < 0 || y >= subActions.size()) continue;

			QString translatedName = charToQString(name, _nativeLangEncoding);
			if (translatedName.isEmpty()) continue;

			if (z == -1)
			{
				// Translate the submenu at position y
				subActions[y]->setText(translatedName);
			}
			else
			{
				// Translate sub-sub-menu at position z
				QMenu* subMenu = subActions[y]->menu();
				if (!subMenu) continue;

				QList<QAction*> subSubActions = subMenu->actions();
				if (z < 0 || z >= subSubActions.size()) continue;

				subSubActions[z]->setText(translatedName);
			}
		}
	}
}

// Helper: recursively walk menu tree and translate actions by command ID
void NativeLangSpeaker::translateMenuActions(QMenu* menu, const QMap<int, QString>& translations) const
{
	if (!menu) return;

	for (QAction* action : menu->actions())
	{
		if (action->isSeparator()) continue;

		// Check if action has a data() property with command ID
		QVariant data = action->data();
		if (data.isValid())
		{
			int cmdId = data.toInt();
			if (cmdId > 0)
			{
				auto it = translations.find(cmdId);
				if (it != translations.end())
				{
					// Preserve keyboard shortcut text if present
					QString currentText = action->text();
					int tabPos = currentText.indexOf('\t');
					if (tabPos >= 0)
					{
						action->setText(it.value() + currentText.mid(tabPos));
					}
					else
					{
						action->setText(it.value());
					}
				}
			}
		}

		// Recurse into submenus
		QMenu* subMenu = action->menu();
		if (subMenu)
		{
			translateMenuActions(subMenu, translations);
		}
	}
}

// changeDlgLangQt - translate a QDialog's widgets using the localization XML
bool NativeLangSpeaker::changeDlgLangQt(QDialog* dialog, const char* dlgTagName) const
{
	if (!dialog || !_nativeLang) return false;

	NppXml::Node dlgNode = NppXml::firstChildElement(_nativeLang, "Dialog");
	if (!dlgNode) return false;

	dlgNode = searchDlgNode(dlgNode, dlgTagName);
	if (!dlgNode) return false;

	// Set dialog title
	const char* title = NppXml::attribute(NppXml::toElement(dlgNode), "title");
	if (title && title[0])
	{
		dialog->setWindowTitle(charToQString(title, _nativeLangEncoding));
	}

	// Translate child widgets by their objectName matching the "id" attribute
	// In Qt, we use objectName for widget identification instead of Win32 dialog item IDs
	for (NppXml::Node childNode = NppXml::firstChildElement(dlgNode, "Item");
		childNode;
		childNode = NppXml::nextSiblingElement(childNode, "Item"))
	{
		NppXml::Element element = NppXml::toElement(childNode);
		const char* name = NppXml::attribute(element, "name");
		if (!name || !name[0]) continue;

		const char* widgetName = NppXml::attribute(element, "objectName");
		const int id = NppXml::intAttribute(element, "id", -1);

		QString translatedText = charToQString(name, _nativeLangEncoding);

		if (widgetName && widgetName[0])
		{
			// Find widget by objectName
			QWidget* widget = dialog->findChild<QWidget*>(QString::fromUtf8(widgetName));
			if (widget)
			{
				setWidgetText(widget, translatedText);
			}
		}
		else if (id > 0)
		{
			// Find widget by objectName matching the ID pattern "ctrl_<id>"
			QString objName = QString("ctrl_%1").arg(id);
			QWidget* widget = dialog->findChild<QWidget*>(objName);
			if (widget)
			{
				setWidgetText(widget, translatedText);
			}
		}
	}

	return true;
}

// Helper: set text on various Qt widget types
void NativeLangSpeaker::setWidgetText(QWidget* widget, const QString& text) const
{
	if (!widget) return;

	if (auto* label = qobject_cast<QLabel*>(widget))
	{
		label->setText(text);
	}
	else if (auto* button = qobject_cast<QAbstractButton*>(widget))
	{
		button->setText(text);
	}
	else if (auto* groupBox = qobject_cast<QGroupBox*>(widget))
	{
		groupBox->setTitle(text);
	}
	else
	{
		// For other widgets, try setWindowTitle
		widget->setWindowTitle(text);
	}
}

// getLocalizedStrQt - convenience for Qt code to get translated strings
QString NativeLangSpeaker::getLocalizedStrQt(const char* strID, const QString& defaultString) const
{
	if (!_nativeLang || !strID) return defaultString;

	NppXml::Node node = NppXml::firstChildElement(_nativeLang, "MiscStrings");
	if (!node) return defaultString;

	node = NppXml::firstChildElement(node, strID);
	if (!node) return defaultString;

	NppXml::Element element = NppXml::toElement(node);
	const char* value = NppXml::attribute(element, "value");
	if (!value) return defaultString;

	return QString::fromUtf8(value);
}
