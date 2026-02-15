// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// ============================================================================
// Qt/Linux Implementation of Notepad_plus file operations
// ============================================================================
// This file provides Linux/Qt implementations of the Notepad_plus class methods
// that are implemented in NppIO.cpp on Windows.

// This file contains Qt/Linux-specific implementations
// These implementations use QtCore::Buffer and QtCore::BufferManager
#ifdef NPP_LINUX

#include "Notepad_plus.h"
#include "NppIO.h"
#include "Buffer.h"
#include "ScintillaEditView.h"
// Note: DocTabView.h is already included via Notepad_plus.h
#include "Parameters.h"
#include "lastRecentFileList.h"
#include "MISC/Common/LinuxTypes.h"
#include "Platform/Clipboard.h"
#include "Platform/FileWatcher.h"
#include "menuCmdID.h"
#include "MISC/PluginsManager/Notepad_plus_msgs.h"

// Panel headers needed for destructor
#include "ClipboardHistory/ClipboardHistoryPanel.h"
#include "ProjectPanel/ProjectPanel.h"
#include "DocumentMap/DocumentMap.h"
#include "FunctionList/FunctionListPanel.h"
#include "FileBrowser/FileBrowser.h"
#include "VerticalFileSwitcher/VerticalFileSwitcher.h"

// MainWindow for panel management
#include "MainWindow/Notepad_plus_Window.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QAbstractButton>
#include <QClipboard>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <memory>
#include <mutex>
#include <chrono>
#include <iostream>

// ============================================================================
// Global Variables
// ============================================================================

std::chrono::steady_clock::time_point g_nppStartTimePoint{};
std::chrono::steady_clock::duration g_pluginsLoadingTime{};

// ============================================================================
// Constructor and Destructor
// ============================================================================

Notepad_plus::Notepad_plus()
	: _autoCompleteMain(&_mainEditView)
	, _autoCompleteSub(&_subEditView)
	, _smartHighlighter(&_findReplaceDlg)
{
	// Initialize member variables that need explicit initialization
	memset(&_prevSelectedRange, 0, sizeof(_prevSelectedRange));

	NppParameters& nppParam = NppParameters::getInstance();
	NppXml::Document nativeLangDocRoot = nppParam.getNativeLang();
	_nativeLangSpeaker.init(nativeLangDocRoot);

	LocalizationSwitcher & localizationSwitcher = nppParam.getLocalizationSwitcher();
	const char *fn = _nativeLangSpeaker.getFileName();
	if (fn)
	{
		localizationSwitcher.setFileName(fn);
	}

	nppParam.setNativeLangSpeaker(&_nativeLangSpeaker);

	// Note: On Linux, admin mode detection is handled differently
	// For now, assume non-admin mode (can be enhanced later)
	nppParam.setAdminMode(false);
	_isAdministrator = false;

	// Initialize view pointers - critical for file operations
	_pEditView = &_mainEditView;
	_pDocTab = &_mainDocTab;
	_pNonEditView = &_subEditView;
	_pNonDocTab = &_subDocTab;
	_mainWindowStatus = WindowMainActive;
	_activeView = MAIN_VIEW;
}

Notepad_plus::~Notepad_plus()
{
	// ATTENTION : the order of the destruction is very important
	// because if the parent's window handle is destroyed before
	// the destruction of its children windows' handles,
	// its children windows' handles will be destroyed automatically!

	(NppParameters::getInstance()).destroyInstance();

#ifndef NPP_LINUX
	delete _pTrayIco;
#endif
	delete _pAnsiCharPanel;
	delete _pClipboardHistoryPanel;
	delete _pDocumentListPanel;
	delete _pProjectPanel_1;
	delete _pProjectPanel_2;
	delete _pProjectPanel_3;
	delete _pDocMap;
	delete _pFuncList;
	delete _pFileBrowser;
}

// ============================================================================
// File Operations
// ============================================================================

void Notepad_plus::fileNew()
{
    std::cout << "[fileNew] Starting..." << std::endl;
    BufferID newBufID = MainFileManager.newEmptyDocument();
    std::cout << "[fileNew] newEmptyDocument returned: " << newBufID << std::endl;
    if (newBufID != BUFFER_INVALID)
    {
        std::cout << "[fileNew] Loading buffer into view..." << std::endl;
        loadBufferIntoView(newBufID, currentView(), true);
        std::cout << "[fileNew] Switching to file..." << std::endl;
        switchToFile(newBufID);
        std::cout << "[fileNew] Completed successfully." << std::endl;
    }
    else
    {
        std::cerr << "[fileNew] Failed to create new buffer!" << std::endl;
    }
}

void Notepad_plus::fileOpen()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QStringList fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        QObject::tr("Open"),
        dir,
        QObject::tr("All Files (*);;Text Files (*.txt)"));

    BufferID lastOpened = BUFFER_INVALID;
    for (const QString& fileName : fileNames)
    {
        if (!fileName.isEmpty())
        {
            BufferID test = doOpen(fileName.toStdWString());
            if (test != BUFFER_INVALID)
                lastOpened = test;
        }
    }

    if (lastOpened != BUFFER_INVALID)
    {
        switchToFile(lastOpened);
    }
}

bool Notepad_plus::fileReload()
{
    if (!_pEditView)
        return false;

    BufferID buf = _pEditView->getCurrentBufferID();
    Buffer* buffer = MainFileManager.getBufferByID(buf);
    if (!buffer)
        return false;

    return doReload(buf, buffer->isDirty());
}

bool Notepad_plus::fileClose(BufferID id, int curView)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    int viewToClose = currentView();
    if (curView != -1)
        viewToClose = curView;

    // Determine if it's a cloned buffer
    DocTabView* nonCurrentTab = (viewToClose == MAIN_VIEW) ? &_subDocTab : &_mainDocTab;
    bool isCloned = nonCurrentTab->getIndexByBuffer(bufferID) != -1;

    if ((buf->isUntitled() && buf->docLength() == 0) || isCloned)
    {
        // Do nothing
    }
    else if (buf->isDirty())
    {
        const wchar_t* fileNamePath = buf->getFullPathName();
        int res = doSaveOrNot(fileNamePath);

        if (res == IDYES)
        {
            if (!fileSave(id))
                return false;
        }
        else if (res == IDCANCEL)
        {
            return false;
        }
    }

    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();
    bool doDeleteBackup = isSnapshotMode;
    if (isSnapshotMode && isCloned)
        doDeleteBackup = false;

    doClose(bufferID, viewToClose, doDeleteBackup);
    return true;
}

bool Notepad_plus::fileCloseAll(bool doDeleteBackup, bool isSnapshotMode)
{
    bool noSaveToAll = false;
    bool saveToAll = false;

    // Check in both views
    std::vector<BufferID> uniqueBuffers;

    for (size_t i = 0; i < _mainDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _mainDocTab.getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);

        uniqueBuffers.push_back(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, MAIN_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    for (size_t i = 0; i < _subDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _subDocTab.getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);

        // Check if already processed
        bool alreadyProcessed = false;
        for (BufferID processedId : uniqueBuffers)
        {
            if (processedId == id)
            {
                alreadyProcessed = true;
                break;
            }
        }
        if (alreadyProcessed)
            continue;

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, SUB_VIEW);
            switchEditViewTo(SUB_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Now close all
    for (size_t i = _mainDocTab.nbItem(); i > 0; --i)
    {
        doClose(_mainDocTab.getBufferByIndex(i - 1), MAIN_VIEW, doDeleteBackup);
    }

    for (size_t i = _subDocTab.nbItem(); i > 0; --i)
    {
        doClose(_subDocTab.getBufferByIndex(i - 1), SUB_VIEW, doDeleteBackup);
    }

    return true;
}

bool Notepad_plus::fileCloseAllButCurrent()
{
    BufferID current = _pEditView->getCurrentBufferID();
    int active = _pDocTab->getCurrentTabIndex();
    bool noSaveToAll = false;
    bool saveToAll = false;

    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();

    // First check main view
    for (size_t i = 0; i < _mainDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _mainDocTab.getBufferByIndex(i);
        if (id == current)
            continue;

        Buffer* buf = MainFileManager.getBufferByID(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, MAIN_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Then check sub view
    for (size_t i = 0; i < _subDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _subDocTab.getBufferByIndex(i);
        if (id == current)
            continue;

        Buffer* buf = MainFileManager.getBufferByID(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, SUB_VIEW);
            switchEditViewTo(SUB_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Close all but current in current view
    int currentViewID = currentView();
    DocTabView* pDocTab = (currentViewID == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    for (int32_t i = static_cast<int32_t>(pDocTab->nbItem()) - 1; i >= 0; --i)
    {
        if (i == active)
            continue;

        doClose(pDocTab->getBufferByIndex(i), currentViewID, isSnapshotMode);
    }

    // Close all in other view
    int otherViewID = otherFromView(currentViewID);
    DocTabView* pOtherDocTab = (otherViewID == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    for (int32_t i = static_cast<int32_t>(pOtherDocTab->nbItem()) - 1; i >= 0; --i)
    {
        doClose(pOtherDocTab->getBufferByIndex(i), otherViewID, isSnapshotMode);
    }

    return true;
}

void Notepad_plus::fileCloseAllButPinned()
{
    std::vector<BufferViewInfo> bufsToClose;

    int iPinned = -1;
    for (int j = 0; j < int(_mainDocTab.nbItem()); ++j)
    {
        BufferID id = _mainDocTab.getBufferByIndex(j);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf->isPinned())
            iPinned++;
        else
            break;
    }

    for (int i = int(_mainDocTab.nbItem()) - 1; i > iPinned; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_mainDocTab.getBufferByIndex(i), MAIN_VIEW));
    }

    iPinned = -1;
    for (int j = 0; j < int(_subDocTab.nbItem()); ++j)
    {
        BufferID id = _subDocTab.getBufferByIndex(j);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf->isPinned())
            iPinned++;
        else
            break;
    }

    for (int i = int(_subDocTab.nbItem()) - 1; i > iPinned; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_subDocTab.getBufferByIndex(i), SUB_VIEW));
    }

    fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllToLeft()
{
    std::vector<BufferViewInfo> bufsToClose;
    for (int i = _pDocTab->getCurrentTabIndex() - 1; i >= 0; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
    }
    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllToRight()
{
    const int iActive = _pDocTab->getCurrentTabIndex();
    std::vector<BufferViewInfo> bufsToClose;
    for (int i = int(_pDocTab->nbItem()) - 1; i > iActive; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
    }
    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllUnchanged()
{
    std::vector<BufferViewInfo> bufsToClose;

    for (int i = int(_pDocTab->nbItem()) - 1; i >= 0; i--)
    {
        BufferID id = _pDocTab->getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if ((buf->isUntitled() && buf->docLength() == 0) || !buf->isDirty())
        {
            bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
        }
    }

    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileSave(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    if (!buf->getFileReadOnly() && buf->isDirty())
    {
        if (buf->isUntitled())
        {
            return fileSaveAs(bufferID);
        }

        // TODO: Implement backup functionality
        // For now, just save directly

        return doSave(bufferID, buf->getFullPathName(), false);
    }
    return false;
}

bool Notepad_plus::fileSaveAll()
{
    size_t nbDirty = getNbDirtyBuffer(MAIN_VIEW) + getNbDirtyBuffer(SUB_VIEW);

    if (!nbDirty)
        return false;

    if (fileSaveAllConfirm())
    {
        // Save all in main view
        for (size_t i = 0; i < _mainDocTab.nbItem(); ++i)
        {
            BufferID id = _mainDocTab.getBufferByIndex(i);
            Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf->isDirty())
            {
                if (!fileSave(id))
                    return false;
            }
        }

        // Save all in sub view
        for (size_t i = 0; i < _subDocTab.nbItem(); ++i)
        {
            BufferID id = _subDocTab.getBufferByIndex(i);
            Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf->isDirty())
            {
                if (!fileSave(id))
                    return false;
            }
        }
    }

    return true;
}

bool Notepad_plus::fileSaveAs(BufferID id, bool isSaveCopy)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString defaultName = buf->getFileNameQString();
    QString defaultDir = buf->isUntitled() ? QString() : buf->getFilePath();

    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QObject::tr("Save As"),
        defaultDir.isEmpty() ? defaultName : defaultDir,
        QObject::tr("All Files (*)"));

    if (fileName.isEmpty())
        return false;

    bool success = doSave(bufferID, fileName.toStdWString().c_str(), isSaveCopy);

    if (success && !isSaveCopy)
    {
        // Update buffer filename - use QString overload to avoid wchar_t* issues
        buf->setFileName(fileName);
    }

    return success;
}

bool Notepad_plus::fileDelete(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString filePath = QString::fromStdWString(buf->getFullPathName());

    int ret = QMessageBox::question(
        nullptr,
        QObject::tr("Delete File"),
        QObject::tr("Are you sure you want to delete \"%1\"?").arg(filePath),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
        return false;

    // Notify plugins that file is about to be deleted
    SCNotification scnN{};
    scnN.nmhdr.hwndFrom = nullptr;
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(bufferID);
    scnN.nmhdr.code = NPPN_FILEBEFOREDELETE;
    _pluginsManager.notify(&scnN);

    // Delete the file first, then close the buffer
    bool deleted = QFile::remove(filePath);
    if (!deleted)
    {
        scnN.nmhdr.code = NPPN_FILEDELETEFAILED;
        _pluginsManager.notify(&scnN);
        return false;
    }

    // Notify plugins that file has been deleted
    scnN.nmhdr.code = NPPN_FILEDELETED;
    _pluginsManager.notify(&scnN);

    // Close the buffer after deletion
    fileClose(bufferID);
    return true;
}

bool Notepad_plus::fileRename(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString oldPath = QString::fromStdWString(buf->getFullPathName());
    QString oldName = QString::fromStdWString(buf->getFileName());

    bool ok;
    QString newName = QInputDialog::getText(
        nullptr,
        QObject::tr("Rename"),
        QObject::tr("New name:"),
        QLineEdit::Normal,
        oldName,
        &ok);

    if (!ok || newName.isEmpty() || newName == oldName)
        return false;

    // Notify plugins that file is about to be renamed
    SCNotification scnN{};
    scnN.nmhdr.hwndFrom = nullptr;
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(bufferID);
    scnN.nmhdr.code = NPPN_FILEBEFORERENAME;
    _pluginsManager.notify(&scnN);

    QString newPath = QFileInfo(oldPath).path() + "/" + newName;

    if (QFile::rename(oldPath, newPath))
    {
        buf->setFileName(newPath.toStdWString().c_str());

        // Notify plugins that file has been renamed
        scnN.nmhdr.code = NPPN_FILERENAMED;
        _pluginsManager.notify(&scnN);
        return true;
    }

    // Notify plugins that rename was cancelled/failed
    scnN.nmhdr.code = NPPN_FILERENAMECANCEL;
    _pluginsManager.notify(&scnN);
    return false;
}

void Notepad_plus::filePrint(bool showDialog)
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setDocName(QObject::tr("Notepad++"));

	if (showDialog)
	{
		QPrintDialog printDialog(&printer, nullptr);
		if (printDialog.exec() != QDialog::Accepted)
			return;
	}

	// Get text from current Scintilla view
	size_t textLen = _pEditView->execute(SCI_GETLENGTH);
	if (textLen == 0)
		return;

	std::string textBuf(textLen + 1, '\0');
	_pEditView->execute(SCI_GETTEXT, textLen + 1, reinterpret_cast<sptr_t>(textBuf.data()));

	QTextDocument doc;
	doc.setPlainText(QString::fromUtf8(textBuf.c_str()));
	doc.print(&printer);
}

// ============================================================================
// Session Operations
// ============================================================================

bool Notepad_plus::fileLoadSession(const wchar_t* fn)
{
    QString sessionFile;
    if (fn == nullptr)
    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        sessionFile = QFileDialog::getOpenFileName(
            nullptr,
            QObject::tr("Load Session"),
            dir,
            QObject::tr("Session Files (*.session);;All Files (*)"));

        if (sessionFile.isEmpty())
            return false;
    }
    else
    {
        sessionFile = QString::fromStdWString(fn);
    }

    QFile file(sessionFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QXmlStreamReader xml(&file);
    bool filesOpened = false;

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == u"File")
            {
                QString filename = xml.attributes().value("filename").toString();
                if (!filename.isEmpty())
                {
                    doOpen(filename.toStdWString());
                    filesOpened = true;
                }
            }
        }
    }

    file.close();
    return filesOpened;
}

const wchar_t* Notepad_plus::fileSaveSession(size_t nbFile, wchar_t** fileNames, const wchar_t* sessionFile2save, bool includeFileBrowser)
{
    Q_UNUSED(includeFileBrowser);

    QString sessionFile;
    if (sessionFile2save == nullptr)
    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        sessionFile = QFileDialog::getSaveFileName(
            nullptr,
            QObject::tr("Save Session"),
            dir + "/session.session",
            QObject::tr("Session Files (*.session);;All Files (*)"));

        if (sessionFile.isEmpty())
            return nullptr;
    }
    else
    {
        sessionFile = QString::fromStdWString(sessionFile2save);
    }

    QFile file(sessionFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return nullptr;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("NotepadPlus");
    xml.writeStartElement("Session");
    xml.writeAttribute("activeView", "0");
    xml.writeStartElement("mainView");
    xml.writeAttribute("activeIndex", "0");

    // Write files from the provided list or from currently open buffers
    if (nbFile > 0 && fileNames != nullptr)
    {
        for (size_t i = 0; i < nbFile; ++i)
        {
            if (fileNames[i] != nullptr)
            {
                xml.writeStartElement("File");
                xml.writeAttribute("filename", QString::fromWCharArray(fileNames[i]));
                xml.writeEndElement();
            }
        }
    }
    else
    {
        for (size_t i = 0; i < _mainDocTab.nbItem(); ++i)
        {
            BufferID id = _mainDocTab.getBufferByIndex(i);
            Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf && !buf->isUntitled())
            {
                xml.writeStartElement("File");
                xml.writeAttribute("filename", QString::fromStdWString(buf->getFullPathName()));
                xml.writeEndElement();
            }
        }
    }

    xml.writeEndElement(); // mainView
    xml.writeEndElement(); // Session
    xml.writeEndElement(); // NotepadPlus
    xml.writeEndDocument();
    file.close();

    // Return the session file path (store in a static to keep it valid)
    static std::wstring sessionPath;
    sessionPath = sessionFile.toStdWString();
    return sessionPath.c_str();
}

const wchar_t* Notepad_plus::fileSaveSession(size_t nbFile, wchar_t** fileNames)
{
    return fileSaveSession(nbFile, fileNames, nullptr, false);
}

// ============================================================================
// Comment Operations
// ============================================================================

bool Notepad_plus::doBlockComment(comment_mode currCommentMode)
{
    Buffer* buf = _pEditView->getCurrentBuffer();
    if (!buf)
        return false;

    // Avoid side-effects when file is read-only
    if (buf->isReadOnly())
        return false;

    // Get comment symbols for current language
    QString commentLineSymbol = buf->getCommentLineSymbol();
    if (commentLineSymbol.isEmpty())
    {
        // Try stream comment as fallback
        QString commentStart = buf->getCommentStart();
        QString commentEnd = buf->getCommentEnd();
        if (!commentStart.isEmpty() && !commentEnd.isEmpty())
        {
            // Use stream comment for block comment
            if (currCommentMode == cm_comment)
            {
                return doStreamComment();
            }
        }
        return false;
    }

    // Get selection
    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);
    intptr_t selStartLine = _pEditView->execute(SCI_LINEFROMPOSITION, selectionStart);
    intptr_t selEndLine = _pEditView->execute(SCI_LINEFROMPOSITION, selectionEnd);

    // Adjust for caret at beginning of line
    if (selectionEnd == static_cast<size_t>(_pEditView->execute(SCI_POSITIONFROMLINE, selEndLine)))
        selEndLine--;

    std::string comment = commentLineSymbol.toStdString();
    comment += " ";
    size_t comment_length = comment.length();

    _pEditView->execute(SCI_BEGINUNDOACTION);

    for (intptr_t i = selStartLine; i <= selEndLine; ++i)
    {
        size_t lineStart = _pEditView->execute(SCI_POSITIONFROMLINE, i);
        size_t lineIndent = _pEditView->execute(SCI_GETLINEINDENTPOSITION, i);
        size_t lineEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, i);

        // Skip empty lines
        if (lineIndent == lineEnd)
            continue;

        if (currCommentMode == cm_comment)
        {
            // Add comment at beginning of line (after indentation)
            _pEditView->execute(SCI_INSERTTEXT, lineIndent, reinterpret_cast<sptr_t>(comment.c_str()));
        }
        else if (currCommentMode == cm_uncomment)
        {
            // Remove comment if present
            size_t bufferSize = lineEnd - lineIndent + 1;
            auto linebuf = std::make_unique<char[]>(bufferSize);
            _pEditView->getGenericText(linebuf.get(), bufferSize, lineIndent, lineEnd);

            std::string linebufStr = linebuf.get();
            std::string commentSymbolStr = commentLineSymbol.toStdString();
            size_t pos = linebufStr.find(commentSymbolStr);
            if (pos != std::string::npos)
            {
                size_t len = comment_length;
                if (pos + len < linebufStr.size() && linebufStr[pos + len - 1] == ' ')
                {
                    // Remove the space too
                }
                else
                {
                    len = commentSymbolStr.length();
                }

                _pEditView->execute(SCI_SETSEL, lineIndent + pos, lineIndent + pos + len);
                _pEditView->replaceSelWith("");
            }
        }
        else if (currCommentMode == cm_toggle)
        {
            // Toggle comment
            size_t bufferSize = lineEnd - lineIndent + 1;
            auto linebuf = std::make_unique<char[]>(bufferSize);
            _pEditView->getGenericText(linebuf.get(), bufferSize, lineIndent, lineEnd);

            std::string linebufStr = linebuf.get();
            std::string commentSymbolStr = commentLineSymbol.toStdString();
            size_t pos = linebufStr.find(commentSymbolStr);
            if (pos != std::string::npos)
            {
                // Uncomment
                size_t len = comment_length;
                if (pos + len > linebufStr.size() || linebufStr[pos + len - 1] != ' ')
                    len = commentSymbolStr.length();

                _pEditView->execute(SCI_SETSEL, lineIndent + pos, lineIndent + pos + len);
                _pEditView->replaceSelWith("");
            }
            else
            {
                // Comment
                _pEditView->execute(SCI_INSERTTEXT, lineIndent, reinterpret_cast<sptr_t>(comment.c_str()));
            }
        }
    }

    _pEditView->execute(SCI_ENDUNDOACTION);
    return true;
}

bool Notepad_plus::doStreamComment()
{
    Buffer* buf = _pEditView->getCurrentBuffer();
    if (!buf)
        return false;

    if (buf->isReadOnly())
        return false;

    QString commentStart = buf->getCommentStart();
    QString commentEnd = buf->getCommentEnd();

    if (commentStart.isEmpty() || commentEnd.isEmpty())
        return false;

    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);

    // If no selection, select current line
    if (selectionStart == selectionEnd)
    {
        intptr_t currentLine = _pEditView->getCurrentLineNumber();
        selectionStart = _pEditView->execute(SCI_POSITIONFROMLINE, currentLine);
        selectionEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, currentLine);
    }

    _pEditView->execute(SCI_BEGINUNDOACTION);

    // Insert end comment first (so positions don't shift)
    std::string commentEndStr = commentEnd.toStdString();
    std::string commentStartStr = commentStart.toStdString();
    _pEditView->execute(SCI_INSERTTEXT, selectionEnd, reinterpret_cast<sptr_t>(commentEndStr.c_str()));

    // Insert start comment
    _pEditView->execute(SCI_INSERTTEXT, selectionStart, reinterpret_cast<sptr_t>(commentStartStr.c_str()));

    _pEditView->execute(SCI_ENDUNDOACTION);
    return true;
}

// ============================================================================
// Buffer Change Notification
// ============================================================================

void Notepad_plus::notifyBufferChanged(Buffer* buffer, int mask)
{
    if (!buffer)
        return;

    // Update views
    _mainEditView.bufferUpdated(buffer, mask);
    _subEditView.bufferUpdated(buffer, mask);
    _mainDocTab.bufferUpdated(buffer, mask);
    _subDocTab.bufferUpdated(buffer, mask);

    bool mainActive = (_mainEditView.getCurrentBuffer() == buffer);
    bool subActive = (_subEditView.getCurrentBuffer() == buffer);

    if (mask & BufferChangeStatus)
    {
        switch (buffer->getStatus())
        {
            case QtCore::DOC_UNNAMED:
            case QtCore::DOC_REGULAR:
            case QtCore::DOC_INACCESSIBLE:
                break;

            case QtCore::DOC_MODIFIED:
            {
                // File modified externally
                if (!buffer->isMonitoringOn())
                {
                    // Ask user to reload
                    QString fileName = QString::fromStdWString(buffer->getFullPathName());
                    int ret = QMessageBox::question(
                        nullptr,
                        QObject::tr("Reload"),
                        QObject::tr("\"%1\"\n\nThis file has been modified by another program.\nDo you want to reload it?").arg(fileName),
                        QMessageBox::Yes | QMessageBox::No);

                    if (ret == QMessageBox::Yes)
                    {
                        buffer->setDirty(false);
                        buffer->setUnsync(false);
                        doReload(buffer->getID(), false);
                        if (mainActive || subActive)
                        {
                            performPostReload(mainActive ? MAIN_VIEW : SUB_VIEW);
                        }
                    }
                    else
                    {
                        buffer->setDirty(true);
                        buffer->setUnsync(true);
                    }
                }
                break;
            }

            case QtCore::DOC_NEEDRELOAD:
            {
                doReload(buffer->getID(), false);
                if (buffer == _mainEditView.getCurrentBuffer())
                {
                    _mainEditView.setPositionRestoreNeeded(false);
                    _mainEditView.execute(SCI_DOCUMENTEND);
                }
                if (buffer == _subEditView.getCurrentBuffer())
                {
                    _subEditView.setPositionRestoreNeeded(false);
                    _subEditView.execute(SCI_DOCUMENTEND);
                }
                break;
            }

            case QtCore::DOC_DELETED:
            {
                // Notify plugins about file deletion
                SCNotification scnN{};
                scnN.nmhdr.hwndFrom = nullptr;
                scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(buffer->getID());
                scnN.nmhdr.code = NPPN_FILEDELETED;
                _pluginsManager.notify(&scnN);

                // File deleted externally
                QString fileName = QString::fromStdWString(buffer->getFullPathName());
                int ret = QMessageBox::question(
                    nullptr,
                    QObject::tr("Keep File"),
                    QObject::tr("\"%1\"\n\nThis file has been deleted by another program.\nDo you want to keep it in the editor?").arg(fileName),
                    QMessageBox::Yes | QMessageBox::No);

                if (ret == QMessageBox::No)
                {
                    doClose(buffer->getID(), otherView());
                    doClose(buffer->getID(), currentView());
                }
                break;
            }
        }
    }

    // Notify plugins about read-only state changes
    if (mask & BufferChangeReadonly)
    {
        SCNotification scnN{};
        scnN.nmhdr.code = NPPN_READONLYCHANGED;
        scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(buffer->getID());
        int readonlyFlags = 0;
        if (buffer->isFileReadOnly() || buffer->isUserReadOnly())
            readonlyFlags |= DOCSTATUS_READONLY;
        if (buffer->isDirty())
            readonlyFlags |= DOCSTATUS_BUFFERDIRTY;
        scnN.nmhdr.idFrom = static_cast<uptr_t>(readonlyFlags);
        _pluginsManager.notify(&scnN);
    }

    // Notify plugins about language changes
    if (mask & BufferChangeLanguage)
    {
        SCNotification scnN{};
        scnN.nmhdr.hwndFrom = nullptr;
        scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(buffer->getID());
        scnN.nmhdr.code = NPPN_LANGCHANGED;
        _pluginsManager.notify(&scnN);
    }
}

// ============================================================================
// File Browser
// ============================================================================

void Notepad_plus::launchFileBrowser(const std::vector<std::wstring>& folders, const std::wstring& selectedItemPath, bool fromScratch)
{
    Q_UNUSED(fromScratch);

    // Get the MainWindow via the edit view's widget hierarchy
    QWidget* w = _mainEditView.getWidget();
    if (!w) return;
    QWidget* topLevel = w->window();
    auto* mainWin = qobject_cast<QtControls::MainWindow::MainWindow*>(topLevel);
    if (!mainWin) return;

    // Show the file browser panel
    mainWin->showPanel("fileBrowser", true);

    auto* fileBrowser = mainWin->getFileBrowser();
    if (!fileBrowser) return;

    // Add root folders
    for (const auto& folder : folders)
    {
        fileBrowser->addRootFolder(QString::fromStdWString(folder));
    }

    // Navigate to selected item if specified
    if (!selectedItemPath.empty())
    {
        fileBrowser->navigateToFile(QString::fromStdWString(selectedItemPath));
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

bool Notepad_plus::fileSaveAllConfirm()
{
    bool confirmed = false;

    if (NppParameters::getInstance().getNppGUI()._saveAllConfirm)
    {
        int answer = doSaveAll();

        if (answer == IDYES)
        {
            confirmed = true;
        }

        if (answer == IDRETRY)
        {
            NppParameters::getInstance().getNppGUI()._saveAllConfirm = false;
            confirmed = true;
        }
    }
    else
    {
        confirmed = true;
    }

    return confirmed;
}

bool Notepad_plus::fileSaveSpecific(const std::wstring& fileNameToSave)
{
    BufferID idToSave = _mainDocTab.findBufferByName(fileNameToSave.c_str());
    if (idToSave == BUFFER_INVALID)
    {
        idToSave = _subDocTab.findBufferByName(fileNameToSave.c_str());
    }

    if (idToSave != BUFFER_INVALID)
    {
        fileSave(idToSave);
        checkDocState();
        return true;
    }

    return false;
}

bool Notepad_plus::fileCloseAllGiven(const std::vector<BufferViewInfo>& krvecBuffer)
{
    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();

    for (const auto& i : krvecBuffer)
    {
        doClose(i._bufID, i._iView, isSnapshotMode);
    }

    return true;
}

void Notepad_plus::prepareBufferChangedDialog(Buffer* buffer)
{
    // Switch to the file that changed
    int index = _pDocTab->getIndexByBuffer(buffer->getID());
    int iView = currentView();
    if (index == -1)
        iView = otherView();

    activateBuffer(buffer->getID(), iView);
}

bool Notepad_plus::isFileSession(const wchar_t* filename)
{
    const wchar_t* definedSessionExt = NppParameters::getInstance().getNppGUI()._definedSessionExt.c_str();
    if (*definedSessionExt != L'\0')
    {
        std::wstring fncp = filename;
        std::wstring usrSessionExt = L"";
        if (*definedSessionExt != L'.')
        {
            usrSessionExt += L".";
        }
        usrSessionExt += definedSessionExt;

        size_t pos = fncp.rfind(L'.');
        if (pos != std::wstring::npos)
        {
            std::wstring ext = fncp.substr(pos);
            if (ext == usrSessionExt)
                return true;
        }
    }

    // Check default extension
    std::wstring fn = filename;
    if (fn.size() > 8 && fn.substr(fn.size() - 8) == L".session")
        return true;

    return false;
}

bool Notepad_plus::isFileWorkspace(const wchar_t* filename)
{
    const wchar_t* definedWorkspaceExt = NppParameters::getInstance().getNppGUI()._definedWorkspaceExt.c_str();
    if (*definedWorkspaceExt != L'\0')
    {
        std::wstring fncp = filename;
        std::wstring usrWorkspaceExt = L"";
        if (*definedWorkspaceExt != L'.')
        {
            usrWorkspaceExt += L".";
        }
        usrWorkspaceExt += definedWorkspaceExt;

        size_t pos = fncp.rfind(L'.');
        if (pos != std::wstring::npos)
        {
            std::wstring ext = fncp.substr(pos);
            if (ext == usrWorkspaceExt)
                return true;
        }
    }

    // Check default extension
    std::wstring fn = filename;
    if (fn.size() > 10 && fn.substr(fn.size() - 10) == L".workspace")
        return true;

    return false;
}

// ============================================================================
// Core File Operations (doSave, doReload, doClose)
// ============================================================================

bool Notepad_plus::doSave(BufferID id, const wchar_t* filename, bool isCopy)
{
    const int index = MainFileManager.getBufferIndexByID(id);
    if (index == -1)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Cannot save: Buffer is invalid."));
        return false;
    }

    // Notify plugins that current file is about to be saved
    SCNotification scnN{};
    scnN.nmhdr.hwndFrom = nullptr;
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(id);
    if (!isCopy)
    {
        scnN.nmhdr.code = NPPN_FILEBEFORESAVE;
        _pluginsManager.notify(&scnN);
    }

    SavingStatus res = MainFileManager.saveBuffer(id, filename, isCopy);

    if (!isCopy)
    {
        scnN.nmhdr.code = NPPN_FILESAVED;
        _pluginsManager.notify(&scnN);
    }

    if (res == SavingStatus::FullReadOnlySavingForbidden)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Cannot save file.\nThe Notepad++ full read-only saving forbidden mode prevented the file from being saved."));
    }
    else if (res == SavingStatus::NotEnoughRoom)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Failed to save file.\nIt seems there's not enough space on disk to save file. Your file is not saved."));
    }
    else if (res == SavingStatus::SaveWritingFailed)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Failed to write file."));
    }
    else if (res == SavingStatus::SaveOpenFailed)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Please check whether if this file is opened in another program or is write-protected."));
    }

    return res == SavingStatus::SaveOK;
}

bool Notepad_plus::doReload(BufferID id, bool alert)
{
    if (id == BUFFER_INVALID)
        return false;

    Buffer* buf = MainFileManager.getBufferByID(id);
    if (!buf)
        return false;

    // Check if file is dirty and alert user
    if (alert && buf->isDirty())
    {
        QString fileName = QString::fromStdWString(buf->getFullPathName());
        int ret = QMessageBox::question(nullptr, QObject::tr("Reload"),
            QObject::tr("\"%1\"\n\nThis file has been modified by another program.\nDo you want to reload it and lose the changes made in Notepad++?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return false;
    }

    // Perform reload
    bool result = MainFileManager.reloadBuffer(id);

    if (result)
    {
        // Update UI
        buf->setDirty(false);
        buf->setUnsync(false);
    }

    return result;
}

void Notepad_plus::doClose(BufferID id, int whichOne, bool doDeleteBackup)
{
    DocTabView* tabToClose = (whichOne == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    int i = tabToClose->getIndexByBuffer(id);
    if (i == -1)
        return;

    if (doDeleteBackup)
        MainFileManager.deleteBufferBackup(id);

    Buffer* buf = MainFileManager.getBufferByID(id);
    if (!buf)
        return;

    // Notify plugins that current file is about to be closed
    SCNotification scnN{};
    scnN.nmhdr.hwndFrom = nullptr;
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(id);
    scnN.nmhdr.code = NPPN_FILEBEFORECLOSE;
    _pluginsManager.notify(&scnN);

    // Get file path for recent files
    std::wstring fileFullPath;
    if (!buf->isUntitled())
    {
        const wchar_t* fn = buf->getFullPathName();
        if (QFile::exists(QString::fromStdWString(fn)))
            fileFullPath = fn;
    }

    size_t nbDocs = (whichOne == MAIN_VIEW) ? _mainDocTab.nbItem() : _subDocTab.nbItem();

    // Turn off monitoring if active
    if (buf->isMonitoringOn())
    {
        monitoringStartOrStopAndUpdateUI(buf, false);
    }

    // Do all the work
    bool isBufRemoved = removeBufferFromView(id, whichOne);
    BufferID hiddenBufferID = BUFFER_INVALID;

    if (nbDocs == 1 && canHideView(whichOne))
    {
        // Close the view if both visible
        hideView(whichOne);
        hiddenBufferID = (whichOne == MAIN_VIEW) ?
            _mainDocTab.getBufferByIndex(0) : _subDocTab.getBufferByIndex(0);
    }

    checkSyncState();

    // Notify plugins that current file is closed
    if (isBufRemoved)
    {
        scnN.nmhdr.code = NPPN_FILECLOSED;
        _pluginsManager.notify(&scnN);

        // Add to recent files if file was removed and exists
        if (!fileFullPath.empty())
        {
            _lastRecentFileList.add(fileFullPath.c_str());
        }
    }
}

// ============================================================================
// Additional Helper Methods
// ============================================================================

BufferID Notepad_plus::doOpen(const std::wstring& fileName, bool isRecursive, bool isReadOnly, int encoding, const wchar_t* backupFileName, FILETIME fileNameTimestamp)
{
    Q_UNUSED(isRecursive);
    Q_UNUSED(fileNameTimestamp);

    if (fileName.empty())
        return BUFFER_INVALID;

    QString qFileName = QString::fromStdWString(fileName);

    // Check if file already open
    BufferID existingID = MainFileManager.getBufferFromName(fileName.c_str());
    if (existingID != BUFFER_INVALID)
    {
        // File already open, just switch to it
        activateBuffer(existingID, currentView());
        return existingID;
    }

    // Check if file exists
    if (!QFile::exists(qFileName))
    {
        return BUFFER_INVALID;
    }

    // Notify plugins that a file is about to be loaded
    SCNotification scnN{};
    scnN.nmhdr.hwndFrom = nullptr;
    scnN.nmhdr.idFrom = 0;
    scnN.nmhdr.code = NPPN_FILEBEFORELOAD;
    _pluginsManager.notify(&scnN);

    // Load the file
    BufferID bufferID = MainFileManager.loadFile(fileName.c_str(), static_cast<Document>(NULL), encoding);

    if (bufferID != BUFFER_INVALID)
    {
        Buffer* buf = MainFileManager.getBufferByID(bufferID);
        if (buf)
        {
            if (isReadOnly)
                buf->setFileReadOnly(true);

            // Notify plugins before opening
            scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(bufferID);
            scnN.nmhdr.code = NPPN_FILEBEFOREOPEN;
            _pluginsManager.notify(&scnN);

            loadBufferIntoView(bufferID, currentView());
            switchToFile(bufferID);

            // Notify plugins that file has been opened
            scnN.nmhdr.code = NPPN_FILEOPENED;
            _pluginsManager.notify(&scnN);

            // Handle backup file (snapshot dirty file)
            if (backupFileName && backupFileName[0] != '\0')
            {
                scnN.nmhdr.code = NPPN_SNAPSHOTDIRTYFILELOADED;
                _pluginsManager.notify(&scnN);
            }
        }
    }
    else
    {
        // Notify plugins that file loading failed
        scnN.nmhdr.code = NPPN_FILELOADFAILED;
        _pluginsManager.notify(&scnN);
    }

    return bufferID;
}

void Notepad_plus::monitoringStartOrStopAndUpdateUI(Buffer* pBuf, bool isStarting)
{
    if (!pBuf)
        return;

    if (isStarting)
    {
        pBuf->startMonitoring();

        // Register file with platform FileWatcher for change notifications
        if (!pBuf->isUntitled())
        {
            std::wstring filePath(pBuf->getFullPathName());
            auto& watcher = PlatformLayer::IFileWatcher::getInstance();
            auto handle = watcher.watchFile(filePath,
                [this, pBuf](const PlatformLayer::FileChangeEvent& event)
                {
                    if (event.type == PlatformLayer::FileChangeType::Modified)
                    {
                        // Auto-reload in tail mode and scroll to end
                        doReload(pBuf->getID(), false);
                        // Scroll to end in both views if buffer is active
                        if (_mainEditView.getCurrentBuffer() == pBuf)
                        {
                            _mainEditView.execute(SCI_DOCUMENTEND);
                        }
                        if (_subEditView.getCurrentBuffer() == pBuf)
                        {
                            _subEditView.execute(SCI_DOCUMENTEND);
                        }
                    }
                    else if (event.type == PlatformLayer::FileChangeType::Deleted)
                    {
                        pBuf->setStatus(QtCore::DOC_DELETED);
                        notifyBufferChanged(pBuf, BufferChangeStatus);
                    }
                });

            pBuf->setFileWatchHandle(handle);
        }

        // Set read-only in tail mode
        _pEditView->execute(SCI_SETREADONLY, true);

        // Update tab icon to monitoring state
        _mainDocTab.bufferUpdated(pBuf, BufferChangeReadonly);
        _subDocTab.bufferUpdated(pBuf, BufferChangeReadonly);
    }
    else
    {
        // Unregister from platform FileWatcher
        auto handle = pBuf->getFileWatchHandle();
        if (handle != PlatformLayer::INVALID_WATCH_HANDLE)
        {
            auto& watcher = PlatformLayer::IFileWatcher::getInstance();
            watcher.unwatchFile(handle);
            pBuf->setFileWatchHandle(PlatformLayer::INVALID_WATCH_HANDLE);
        }

        pBuf->stopMonitoring();

        // Remove read-only in tail mode
        _pEditView->execute(SCI_SETREADONLY, false);

        // Update tab icon
        _mainDocTab.bufferUpdated(pBuf, BufferChangeReadonly);
        _subDocTab.bufferUpdated(pBuf, BufferChangeReadonly);
    }
}

// ============================================================================
// Dialog Helper Methods
// ============================================================================

int Notepad_plus::doSaveOrNot(const wchar_t* fn, bool isMulti)
{
    if (NppParameters::getInstance().isEndSessionCritical())
        return IDCANCEL;

    QString fileName = QString::fromStdWString(fn);

    if (!isMulti)
    {
        int ret = QMessageBox::question(
            nullptr,
            QObject::tr("Save"),
            QObject::tr("Save file \"%1\"?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (ret == QMessageBox::Yes)
            return IDYES;
        else if (ret == QMessageBox::No)
            return IDNO;
        else
            return IDCANCEL;
    }
    else
    {
        // For multiple files, use a custom dialog with Yes to All/No to All options
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("Save All"));
        msgBox.setText(QObject::tr("Save file \"%1\"?").arg(fileName));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.addButton(QObject::tr("Yes to All"), QMessageBox::YesRole);
        msgBox.addButton(QObject::tr("No to All"), QMessageBox::NoRole);

        int ret = msgBox.exec();

        // Map button roles to return values
        QAbstractButton* clickedButton = msgBox.clickedButton();
        QString buttonText = clickedButton->text();

        if (buttonText == QObject::tr("Yes to All"))
            return IDRETRY;  // Using IDRETRY for "Yes to All"
        else if (buttonText == QObject::tr("No to All"))
            return IDIGNORE; // Using IDIGNORE for "No to All"
        else if (ret == QMessageBox::Yes)
            return IDYES;
        else if (ret == QMessageBox::No)
            return IDNO;
        else
            return IDCANCEL;
    }
}

int Notepad_plus::doSaveAll()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("Save All"));
    msgBox.setText(QObject::tr("Save all modified files?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.addButton(QObject::tr("Always Yes"), QMessageBox::YesRole);

    int ret = msgBox.exec();

    QAbstractButton* clickedButton = msgBox.clickedButton();
    if (clickedButton->text() == QObject::tr("Always Yes"))
        return IDRETRY;  // Using IDRETRY for "Always Yes"
    else if (ret == QMessageBox::Yes)
        return IDYES;
    else
        return IDNO;
}

// ============================================================================
// Stub Implementations for Missing Methods
// These are simplified implementations that need to be fully implemented later
// ============================================================================

size_t Notepad_plus::getNbDirtyBuffer(int view)
{
    DocTabView* pDocTab = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    size_t count = 0;
    for (size_t i = 0; i < pDocTab->nbItem(); ++i)
    {
        BufferID id = pDocTab->getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf && buf->isDirty())
            count++;
    }
    return count;
}

int Notepad_plus::otherView()
{
    return (currentView() == MAIN_VIEW) ? SUB_VIEW : MAIN_VIEW;
}

void Notepad_plus::checkSyncState()
{
    // TODO: Implement sync state checking
}

void Notepad_plus::checkDocState()
{
    // TODO: Implement document state checking
}

bool Notepad_plus::activateBuffer(BufferID id, int view, bool forceApplyHilite)
{
    Q_UNUSED(forceApplyHilite);
    if (view == MAIN_VIEW)
    {
        bool activated = _mainDocTab.activateBuffer(id);
        if (activated)
            _mainEditView.activateBuffer(id, forceApplyHilite);
        return activated;
    }
    else
    {
        bool activated = _subDocTab.activateBuffer(id);
        if (activated)
            _subEditView.activateBuffer(id, forceApplyHilite);
        return activated;
    }
}

void Notepad_plus::loadBufferIntoView(BufferID id, int view, bool dontClose)
{
    Q_UNUSED(dontClose);
    DocTabView* tabToOpen = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    // Check if buffer already exists
    int index = tabToOpen->getIndexByBuffer(id);
    if (index != -1)  // Already open, done
        return;

    // Add buffer to tab
    tabToOpen->addBuffer(id);

    // Connect file monitoring signal
    Buffer* buf = MainFileManager.getBufferByID(id);
    if (buf)
    {
        // Disconnect first to prevent duplicates (Qt::UniqueConnection
        // is incompatible with lambda connections in Qt6)
        QObject::disconnect(buf, &QtCore::Buffer::fileModifiedExternally, buf, nullptr);
        QObject::connect(buf, &QtCore::Buffer::fileModifiedExternally,
            buf, [this, buf]()
            {
                notifyBufferChanged(buf, BufferChangeStatus);
            });
    }
}

bool Notepad_plus::switchToFile(BufferID id)
{
    std::cout << "[Notepad_plus::switchToFile] ENTER - id=" << id << std::endl;

    if (id == BUFFER_INVALID) {
        std::cerr << "[Notepad_plus::switchToFile] ERROR: BUFFER_INVALID" << std::endl;
        return false;
    }

    if (!_pDocTab) {
        std::cerr << "[Notepad_plus::switchToFile] ERROR: _pDocTab is null" << std::endl;
        return false;
    }

    if (!_pEditView) {
        std::cerr << "[Notepad_plus::switchToFile] ERROR: _pEditView is null" << std::endl;
        return false;
    }

    // Activate the buffer in the current view
    std::cout << "[Notepad_plus::switchToFile] Calling _pDocTab->activateBuffer..." << std::endl;
    bool activated = _pDocTab->activateBuffer(id);
    std::cout << "[Notepad_plus::switchToFile] _pDocTab->activateBuffer returned " << activated << std::endl;

    if (activated) {
        std::cout << "[Notepad_plus::switchToFile] Calling _pEditView->activateBuffer..." << std::endl;
        _pEditView->activateBuffer(id, false);
        std::cout << "[Notepad_plus::switchToFile] _pEditView->activateBuffer completed" << std::endl;

        // Notify plugins that buffer has been activated
        SCNotification scnN{};
        scnN.nmhdr.hwndFrom = nullptr;
        scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(id);
        scnN.nmhdr.code = NPPN_BUFFERACTIVATED;
        _pluginsManager.notify(&scnN);
    } else {
        std::cerr << "[Notepad_plus::switchToFile] WARNING: _pDocTab->activateBuffer returned false" << std::endl;
    }

    std::cout << "[Notepad_plus::switchToFile] EXIT - returning " << activated << std::endl;
    return activated;
}

bool Notepad_plus::removeBufferFromView(BufferID id, int view)
{
    std::cout << "[removeBufferFromView] ENTER - bufferID=" << id << " view=" << view << std::endl;

    DocTabView* tabToClose = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    ScintillaEditView* viewToClose = (view == MAIN_VIEW) ? &_mainEditView : &_subEditView;

    // Check if buffer exists
    int index = tabToClose->getIndexByBuffer(id);
    if (index == -1) {
        std::cout << "[removeBufferFromView] Buffer not found in tab, returning false" << std::endl;
        return false;
    }

    Buffer* buf = MainFileManager.getBufferByID(id);
    std::wstring bufFileName = buf->getFullPathName();
    std::wcout << L"[removeBufferFromView] Tab count=" << tabToClose->nbItem()
               << L" Buffer dirty=" << buf->isDirty()
               << L" untitled=" << buf->isUntitled()
               << L" name=" << bufFileName << std::endl;

    // Cannot close doc if last and clean and not renamed (Windows behavior)
    if (tabToClose->nbItem() == 1) {
        std::wstring newTitle = QtCore::UNTITLED_STR;  // "new "
        std::cout << "[removeBufferFromView] Checking prevent-close: bufFileName starts with 'new '? "
                  << (bufFileName.rfind(newTitle, 0) == 0 ? "YES" : "NO") << std::endl;

        if (!buf->isDirty() && buf->isUntitled() && bufFileName.rfind(newTitle, 0) == 0) {
            std::cout << "[removeBufferFromView] PREVENTING CLOSE - empty clean new document" << std::endl;
            return false;
        }
    }

    int active = tabToClose->getCurrentTabIndex();
    std::cout << "[removeBufferFromView] Active tab index=" << active << " closing index=" << index << std::endl;

    if (active == index) {
        // Need an alternative (close real doc, put empty one back)
        if (tabToClose->nbItem() == 1) {
            // Need alternative doc, add new one. Use special logic to prevent flicker
            std::cout << "[removeBufferFromView] Creating replacement document" << std::endl;
            BufferID newID = MainFileManager.newEmptyDocument();
            MainFileManager.addBufferReference(newID, viewToClose);
            tabToClose->setBuffer(0, newID);        // Can safely use id 0, last (only) tab open
            activateBuffer(newID, view);            // Activate. DocTab already activated but not a problem
            std::cout << "[removeBufferFromView] Replacement document created and activated" << std::endl;
        } else {
            int toActivate = 0;
            // Activate next doc, otherwise prev if not possible
            if (static_cast<size_t>(active) == tabToClose->nbItem() - 1) {
                toActivate = active - 1;  // prev
                std::cout << "[removeBufferFromView] Will activate previous tab: " << toActivate << std::endl;
            } else {
                toActivate = active;      // activate the 'active' index. Since we remove the tab first, the indices shift
                std::cout << "[removeBufferFromView] Will activate same index (shifted): " << toActivate << std::endl;
            }

            tabToClose->deletItemAt(static_cast<size_t>(index));  // Delete first
            activateBuffer(tabToClose->getBufferByIndex(toActivate), view);  // Then activate
        }
    } else {
        std::cout << "[removeBufferFromView] Removing non-active tab" << std::endl;
        tabToClose->deletItemAt(static_cast<size_t>(index));
    }

    std::cout << "[removeBufferFromView] Calling closeBuffer for buffer=" << id << std::endl;
    MainFileManager.closeBuffer(id, viewToClose);
    std::cout << "[removeBufferFromView] EXIT - returning true" << std::endl;
    return true;
}

bool Notepad_plus::canHideView(int view)
{
    if (!viewVisible(view))
        return false;   //cannot hide hidden view
    if (!bothActive())
        return false;   //cannot hide only window

    DocTabView* tabToCheck = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    Buffer* buf = MainFileManager.getBufferByID(tabToCheck->getBufferByIndex(0));
    bool canHide = ((tabToCheck->nbItem() == 1) && !buf->isDirty() && buf->isUntitled());
    return canHide;
}

void Notepad_plus::hideView(int whichOne)
{
    if (!bothActive())  //cannot close if not both views visible
        return;

    if (whichOne == MAIN_VIEW)
    {
        _mainWindowStatus &= ~WindowMainActive;
        _mainEditView.display(false);
        _mainDocTab.display(false);
        // Hide the container widget in the splitter
        QWidget* editWidget = _mainEditView.getWidget();
        if (editWidget)
        {
            QWidget* container = editWidget->parentWidget();
            if (container)
                container->hide();
        }
    }
    else if (whichOne == SUB_VIEW)
    {
        _mainWindowStatus &= ~WindowSubActive;
        _subEditView.display(false);
        _subDocTab.display(false);
        // Hide the container widget in the splitter
        QWidget* editWidget = _subEditView.getWidget();
        if (editWidget)
        {
            QWidget* container = editWidget->parentWidget();
            if (container)
                container->hide();
        }
    }
}

void Notepad_plus::performPostReload(int whichOne)
{
    Q_UNUSED(whichOne);
    // TODO: Implement post-reload actions
}

// ============================================================================
// Marked/Bookmarked Lines Operations
// ============================================================================

std::mutex mark_mutex;

void Notepad_plus::copyMarkedLines()
{
    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    std::wstring globalStr = L"";
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            std::wstring currentStr = getMarkedLine(i) + globalStr;
            globalStr = currentStr;
        }
    }

    // Use platform clipboard
    PlatformLayer::IClipboard::getInstance().setText(globalStr);
}

void Notepad_plus::cutMarkedLines()
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    std::wstring globalStr = L"";

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            std::wstring currentStr = getMarkedLine(i) + globalStr;
            globalStr = currentStr;

            deleteMarkedline(i);
        }
    }
    _pEditView->execute(SCI_ENDUNDOACTION);

    PlatformLayer::IClipboard::getInstance().setText(globalStr);
}

void Notepad_plus::deleteMarkedLines(bool isMarked)
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i) == isMarked)
            deleteMarkedline(i);
    }
    _pEditView->execute(SCI_ENDUNDOACTION);
}

void Notepad_plus::pasteToMarkedLines()
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    // Get text from clipboard
    std::wstring clipboardStr = PlatformLayer::IClipboard::getInstance().getText();
    if (clipboardStr.empty())
        return;

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            replaceMarkedline(i, clipboardStr.c_str());
        }
    }
    _pEditView->execute(SCI_ENDUNDOACTION);
}

void Notepad_plus::inverseMarks()
{
    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    for (intptr_t i = 0; i <= lastLine; ++i)
    {
        if (bookmarkPresent(i))
        {
            bookmarkDelete(i);
        }
        else
        {
            bookmarkAdd(i);
        }
    }
}

void Notepad_plus::deleteMarkedline(size_t ln)
{
    intptr_t lineLen = _pEditView->execute(SCI_LINELENGTH, ln);
    intptr_t lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);

    bookmarkDelete(ln);
    wchar_t emptyString[2] = L"";
    _pEditView->replaceTarget(emptyString, lineBegin, lineBegin + lineLen);
}

void Notepad_plus::replaceMarkedline(size_t ln, const wchar_t* str)
{
    intptr_t lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);
    intptr_t lineEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, ln);

    _pEditView->replaceTarget(str, lineBegin, lineEnd);
}

std::wstring Notepad_plus::getMarkedLine(size_t ln)
{
    auto lineLen = _pEditView->execute(SCI_LINELENGTH, ln);
    auto lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);

    wchar_t* buf = new wchar_t[lineLen + 1];
    _pEditView->getGenericText(buf, lineLen + 1, lineBegin, lineBegin + lineLen);
    std::wstring line = buf;
    delete[] buf;

    return line;
}

// ============================================================================
// Comment Operations
// ============================================================================

bool Notepad_plus::undoStreamComment(bool tryBlockComment)
{
    QString commentStart;
    QString commentEnd;
    QString commentLineSymbol;

    std::string symbolStart;
    std::string symbolEnd;
    std::string symbol;

    Buffer* buf = _pEditView->getCurrentBuffer();
    // Avoid side-effects when file is read-only
    if (buf->isReadOnly())
        return false;

    if (buf->getLangType() == L_USER)
    {
        // User defined language - get from user lang container
        // For now, return false as this requires more complex handling
        return false;
    }
    else
    {
        commentLineSymbol = buf->getCommentLineSymbol();
        commentStart = buf->getCommentStart();
        commentEnd = buf->getCommentEnd();
    }

    // If there is no stream-comment symbol and we came not from doBlockComment, try the block comment
    if (commentStart.isEmpty() || commentEnd.isEmpty())
    {
        if (!commentLineSymbol.isEmpty() && tryBlockComment)
            return doBlockComment(cm_uncomment);
        else
            return false;
    }

    std::string start_comment = commentStart.toStdString();
    std::string end_comment = commentEnd.toStdString();
    size_t start_comment_length = start_comment.length();
    size_t end_comment_length = end_comment.length();

    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);

    _pEditView->execute(SCI_BEGINUNDOACTION);

    // Simple implementation: find and remove stream comments within selection
    // This is a simplified version - the full version handles nested comments etc.
    bool found = false;
    size_t pos = selectionStart;
    while (pos < selectionEnd)
    {
        // Use string_view overload for searchInTarget
        std::string_view startView(start_comment);
        intptr_t foundPos = _pEditView->searchInTarget(startView, pos, selectionEnd);
        if (foundPos == -1)
            break;

        std::string_view endView(end_comment);
        intptr_t endPos = _pEditView->searchInTarget(endView, foundPos + start_comment_length, selectionEnd + end_comment_length);
        if (endPos == -1)
            break;

        // Remove end comment first (so positions don't shift)
        _pEditView->replaceTarget("", static_cast<size_t>(endPos), static_cast<size_t>(endPos) + end_comment_length);
        // Remove start comment
        _pEditView->replaceTarget("", static_cast<size_t>(foundPos), static_cast<size_t>(foundPos) + start_comment_length);

        found = true;
        pos = static_cast<size_t>(foundPos);
        selectionEnd -= (start_comment_length + end_comment_length);
    }

    _pEditView->execute(SCI_ENDUNDOACTION);

    return found;
}

// ============================================================================
// Read-Only Mode Operations
// ============================================================================

void Notepad_plus::changeReadOnlyUserModeForAllOpenedTabs(const bool ro)
{
    if (ro != true && NppParameters::getInstance().getNppGUI()._isFullReadOnlySavingForbidden)
        return; // safety for FullReadOnlySavingForbidden mode, refuse to cease the R/O state

    // make R/O changes in both views
    std::vector<DocTabView*> tabViews = { &_mainDocTab, &_subDocTab };
    for (auto& pTabView : tabViews)
    {
        for (size_t i = 0; i < pTabView->nbItem(); ++i)
        {
            BufferID id = pTabView->getBufferByIndex(i);
            if (id != BUFFER_INVALID)
            {
                Buffer* buf = MainFileManager.getBufferByID(id);
                if (buf != nullptr)
                    buf->setUserReadOnly(ro);
            }
        }
    }
}

// ============================================================================
// Helper: Get MainWindow from widget hierarchy
// ============================================================================

static QtControls::MainWindow::MainWindow* getMainWindow(ScintillaEditView& editView)
{
	QWidget* w = editView.getWidget();
	if (!w)
		return nullptr;
	QWidget* topLevel = w->window();
	return qobject_cast<QtControls::MainWindow::MainWindow*>(topLevel);
}

// ============================================================================
// View Mode Operations (Full Screen, Post-It, Distraction Free)
// ============================================================================

void Notepad_plus::fullScreenToggle()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (mainWin)
	{
		mainWin->toggleFullScreen();
	}
}

void Notepad_plus::postItToggle()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (mainWin)
	{
		mainWin->togglePostItMode();
	}
}

void Notepad_plus::distractionFreeToggle()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (mainWin)
	{
		mainWin->toggleDistractionFreeMode();
	}
}

void Notepad_plus::alwaysOnTopToggle()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (mainWin)
	{
		mainWin->setAlwaysOnTop(!mainWin->isAlwaysOnTop());
	}
}

// ============================================================================
// View Operations (Wrap, Indent Guide, Whitespace, EOL, etc.)
// ============================================================================

void Notepad_plus::wrapAllEditors(bool isWrapped)
{
    _mainEditView.wrap(isWrapped);
    _subEditView.wrap(isWrapped);
}

void Notepad_plus::showIndentGuide(bool show)
{
    _mainEditView.showIndentGuideLine(show);
    _subEditView.showIndentGuideLine(show);
}

bool Notepad_plus::isWhiteSpaceShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._whiteSpaceShow;
}

void Notepad_plus::showWhiteSpace(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._whiteSpaceShow = show;
    _mainEditView.showWSAndTab(show);
    _subEditView.showWSAndTab(show);
}

bool Notepad_plus::isEOLShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._eolShow;
}

void Notepad_plus::showEOL(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._eolShow = show;
    _mainEditView.showEOL(show);
    _subEditView.showEOL(show);
}

bool Notepad_plus::isAllCharactersShown()
{
    return isNpcShown() && isEOLShown();
}

void Notepad_plus::showInvisibleChars(bool show)
{
    showNpc(show);
    showEOL(show);
}

bool Notepad_plus::isNpcShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._npcShow;
}

void Notepad_plus::showNpc(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._npcShow = show;
    _mainEditView.showNpc(show);
    _subEditView.showNpc(show);
}

bool Notepad_plus::isCcUniEolShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._ccUniEolShow;
}

void Notepad_plus::showCcUniEol(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._ccUniEolShow = show;
    _mainEditView.showCcUniEol(show);
    _subEditView.showCcUniEol(show);
}

void Notepad_plus::toggleSyncScrollV()
{
	_syncInfo._isSynScrollV = !_syncInfo._isSynScrollV;
	if (_syncInfo._isSynScrollV)
	{
		intptr_t mainCurrentLine = _mainEditView.execute(SCI_GETFIRSTVISIBLELINE);
		intptr_t subCurrentLine = _subEditView.execute(SCI_GETFIRSTVISIBLELINE);
		_syncInfo._line = mainCurrentLine - subCurrentLine;
	}
}

void Notepad_plus::toggleSyncScrollH()
{
	_syncInfo._isSynScrollH = !_syncInfo._isSynScrollH;
	if (_syncInfo._isSynScrollH)
	{
		intptr_t mxoffset = _mainEditView.execute(SCI_GETXOFFSET);
		intptr_t pixel = _mainEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
		intptr_t mainColumn = mxoffset / pixel;

		intptr_t sxoffset = _subEditView.execute(SCI_GETXOFFSET);
		pixel = _subEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
		intptr_t subColumn = sxoffset / pixel;
		_syncInfo._column = mainColumn - subColumn;
	}
}

void Notepad_plus::doSynScroll(ScintillaEditView* whichView)
{
	intptr_t column = 0;
	intptr_t line = 0;
	ScintillaEditView* pView = nullptr;

	if (whichView == &_mainEditView)
	{
		if (_syncInfo._isSynScrollV)
		{
			intptr_t mainCurrentLine = _mainEditView.execute(SCI_GETFIRSTVISIBLELINE);
			intptr_t subCurrentLine = _subEditView.execute(SCI_GETFIRSTVISIBLELINE);
			line = mainCurrentLine - _syncInfo._line - subCurrentLine;
		}
		if (_syncInfo._isSynScrollH)
		{
			intptr_t mxoffset = _mainEditView.execute(SCI_GETXOFFSET);
			intptr_t pixel = _mainEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
			intptr_t mainColumn = mxoffset / pixel;

			intptr_t sxoffset = _subEditView.execute(SCI_GETXOFFSET);
			pixel = _subEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
			intptr_t subColumn = sxoffset / pixel;
			column = mainColumn - _syncInfo._column - subColumn;
		}
		pView = &_subEditView;
	}
	else if (whichView == &_subEditView)
	{
		if (_syncInfo._isSynScrollV)
		{
			intptr_t mainCurrentLine = _mainEditView.execute(SCI_GETFIRSTVISIBLELINE);
			intptr_t subCurrentLine = _subEditView.execute(SCI_GETFIRSTVISIBLELINE);
			line = subCurrentLine + _syncInfo._line - mainCurrentLine;
		}
		if (_syncInfo._isSynScrollH)
		{
			intptr_t mxoffset = _mainEditView.execute(SCI_GETXOFFSET);
			intptr_t pixel = _mainEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
			intptr_t mainColumn = mxoffset / pixel;

			intptr_t sxoffset = _subEditView.execute(SCI_GETXOFFSET);
			pixel = _subEditView.execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<LPARAM>("P"));
			intptr_t subColumn = sxoffset / pixel;
			column = subColumn + _syncInfo._column - mainColumn;
		}
		pView = &_mainEditView;
	}
	else
	{
		return;
	}

	pView->scroll(column, line);
}

// ============================================================================
// Panel Toggle Operations
// ============================================================================

void Notepad_plus::showSummary()
{
	if (!_pEditView)
		return;

	intptr_t lineCount = _pEditView->execute(SCI_GETLINECOUNT);
	intptr_t charCount = _pEditView->execute(SCI_GETLENGTH);

	// Count words by iterating through characters
	intptr_t wordCount = 0;
	bool inWord = false;
	for (intptr_t i = 0; i < charCount; ++i)
	{
		char ch = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, i));
		bool isSpace = (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
		if (!isSpace && !inWord)
		{
			++wordCount;
			inWord = true;
		}
		else if (isSpace)
		{
			inWord = false;
		}
	}

	// Get selection info
	intptr_t selStart = _pEditView->execute(SCI_GETSELECTIONSTART);
	intptr_t selEnd = _pEditView->execute(SCI_GETSELECTIONEND);
	intptr_t selLength = selEnd - selStart;

	QString summary = QString("Lines: %1\nWords: %2\nCharacters (with spaces): %3")
		.arg(lineCount)
		.arg(wordCount)
		.arg(charCount);

	if (selLength > 0)
	{
		summary += QString("\n\nSelected characters: %1").arg(selLength);
	}

	Buffer* buf = getCurrentBuffer();
	QString title = "Summary";
	if (buf)
	{
		QString fileName = QString::fromStdWString(buf->getFullPathName());
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			title = QString("Summary - %1").arg(fi.fileName());
		}
	}

	QMessageBox::information(
		_mainEditView.getWidget() ? _mainEditView.getWidget()->window() : nullptr,
		title,
		summary);
}

void Notepad_plus::toggleMonitoring()
{
    Buffer* buf = getCurrentBuffer();
    if (buf)
    {
        bool isMonitoring = buf->isMonitoringOn();
        monitoringStartOrStopAndUpdateUI(buf, !isMonitoring);
    }
}

void Notepad_plus::toggleDocumentList()
{
    launchDocumentListPanel(true);
}

void Notepad_plus::toggleDocumentMap()
{
    launchDocMap();
}

void Notepad_plus::toggleFunctionList()
{
    launchFunctionList();
}

void Notepad_plus::toggleFileBrowser()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	if (mainWin->isPanelVisible("fileBrowser"))
	{
		mainWin->showPanel("fileBrowser", false);
	}
	else
	{
		mainWin->showPanel("fileBrowser", true);
	}
}

void Notepad_plus::toggleProjectPanel(int index)
{
    int cmdID = IDM_VIEW_PROJECT_PANEL_1 + index;
    ProjectPanel** ppProjPanel = nullptr;
    if (index == 0)
        ppProjPanel = &_pProjectPanel_1;
    else if (index == 1)
        ppProjPanel = &_pProjectPanel_2;
    else
        ppProjPanel = &_pProjectPanel_3;

    launchProjectPanel(cmdID, ppProjPanel, index);
}

void Notepad_plus::switchToProjectPanel(int index)
{
    // Implementation would activate the project panel
    // For now, just toggle it
    toggleProjectPanel(index);
}

void Notepad_plus::switchToFileBrowser()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	// Make the panel visible if not already
	if (!mainWin->isPanelVisible("fileBrowser"))
	{
		mainWin->showPanel("fileBrowser", true);
	}

	// Focus the file browser widget
	auto* fileBrowser = mainWin->getFileBrowser();
	if (fileBrowser)
	{
		QWidget* w = fileBrowser->getWidget();
		if (w)
			w->setFocus();
	}
}

// ============================================================================
// Search and Find Operations
// ============================================================================

void Notepad_plus::showFindReplaceDlg(int dialogType)
{
    // Initialize the Find/Replace dialog on first use.
    // On Windows this is done in Notepad_plus_Window::init(), but
    // the Qt/Linux path never called _findReplaceDlg.init(), causing
    // a null _tabWidget crash when showDialog() is invoked.
    if (!_findReplaceDlg.isCreated())
    {
        _findReplaceDlg.init(&_pEditView);

        // Set up callbacks for multi-document search operations
        _findReplaceDlg.setGetOpenBuffersCallback([this]() -> std::vector<NppFindReplace::FindReplaceDlg::BufferInfo> {
            std::vector<NppFindReplace::FindReplaceDlg::BufferInfo> buffers;
            for (size_t i = 0; i < _mainDocTab.nbItem(); ++i)
            {
                BufferID id = _mainDocTab.getBufferByIndex(i);
                QtCore::Buffer* buf = MainFileManager.getBufferByID(id);
                if (buf)
                {
                    buffers.push_back({static_cast<void*>(id), buf->getFilePath()});
                }
            }
            return buffers;
        });

        _findReplaceDlg.setActivateBufferCallback([this](void* bufferID) -> bool {
            return activateBuffer(static_cast<BufferID>(bufferID), currentView());
        });

        _findReplaceDlg.setGetActiveFilePathCallback([this]() -> QString {
            if (!_pEditView) return QString();
            BufferID id = _pEditView->getCurrentBufferID();
            QtCore::Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf)
            {
                return buf->getFilePath();
            }
            return QString();
        });

        _findReplaceDlg.setGetProjectFilesCallback([this](int panelIndex) -> QStringList {
            auto* mainWin = getMainWindow(_mainEditView);
            if (!mainWin) return {};
            // Currently only one project panel is supported on Linux
            if (panelIndex == 1)
            {
                auto* panel = mainWin->getProjectPanel();
                if (panel)
                {
                    return panel->getAllFilePaths();
                }
            }
            return {};
        });

        // Connect globalModified signal for plugin notification
        QObject::connect(&_findReplaceDlg, &NppFindReplace::FindReplaceDlg::globalModified,
            [this](void* bufferID)
            {
                SCNotification scnN{};
                scnN.nmhdr.code = NPPN_GLOBALMODIFIED;
                scnN.nmhdr.hwndFrom = bufferID;
                scnN.nmhdr.idFrom = 0;
                _pluginsManager.notify(&scnN);
            });
    }

    // Map dialog type to DIALOG_TYPE enum
    DIALOG_TYPE dlgType = static_cast<DIALOG_TYPE>(dialogType);

    // Show the Find/Replace dialog with the specified type
    _findReplaceDlg.doDialog(dlgType);
}

void Notepad_plus::findNext(int direction)
{
    // Use the find/replace dialog to perform find next
    FindOption opt;
    opt._whichDirection = (direction == DIR_DOWN);
    opt._isWrapAround = true;

    // Get current search text from options
    if (!_findReplaceDlg._env->_str2Search.empty())
    {
        processFindNext(_findReplaceDlg._env->_str2Search.c_str(), &opt);
    }
}

void Notepad_plus::processFindNext(const wchar_t* text, const FindOption* opt)
{
    if (!text || !opt || !_pEditView)
        return;

    std::wstring searchText(text);
    if (searchText.empty())
        return;

    // Use the find/replace dialog's processFindNext method
    FindStatus findStatus = FSNoMessage;
    _findReplaceDlg.processFindNext(text, opt, &findStatus);
}

void Notepad_plus::showIncrementalFindDlg()
{
    // Initialize the incremental find dialog on first use.
    // On Windows this is done in Notepad_plus_Window::init(), but
    // the Qt/Linux path never called _incrementFindDlg.init().
    if (!_incrementFindDlg.isCreated())
    {
        // Ensure FindReplaceDlg is also initialized since
        // FindIncrementDlg depends on it
        if (!_findReplaceDlg.isCreated())
        {
            _findReplaceDlg.init(&_pEditView);
        }
        _incrementFindDlg.init(&_findReplaceDlg, &_pEditView);
    }

    // Show the incremental find dialog
    _incrementFindDlg.display(true);
}

void Notepad_plus::setSearchText(const wchar_t* text)
{
    if (text)
    {
        _findReplaceDlg.setSearchText(text);
    }
}

void Notepad_plus::gotoNextFoundResult(int direction)
{
    // Navigate to next/previous found result in the finder panel
    _findReplaceDlg.gotoNextFoundResult(direction);
}

void Notepad_plus::showGoToLineDlg()
{
    // Show the Go To Line dialog
    // Initialize with Windows-compatible interface (HINSTANCE, HWND, ScintillaEditView**)
    _goToLineDlg.init(nullptr, nullptr, &_pEditView);
    _goToLineDlg.doDialog(false);
}

void Notepad_plus::findMatchingBracePos(intptr_t& braceAtCaret, intptr_t& braceOpposite)
{
    if (!_pEditView)
    {
        braceAtCaret = -1;
        braceOpposite = -1;
        return;
    }

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);
    char charBefore = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, currentPos - 1));
    char charAfter = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, currentPos));

    // Define matching braces
    const char* openBraces = "([{<";
    const char* closeBraces = ")]}>";

    braceAtCaret = -1;
    braceOpposite = -1;

    // Check character before caret
    const char* match = strchr(openBraces, charBefore);
    if (match)
    {
        braceAtCaret = currentPos - 1;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    match = strchr(closeBraces, charBefore);
    if (match)
    {
        braceAtCaret = currentPos - 1;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    // Check character at caret
    match = strchr(openBraces, charAfter);
    if (match)
    {
        braceAtCaret = currentPos;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    match = strchr(closeBraces, charAfter);
    if (match)
    {
        braceAtCaret = currentPos;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }
}

void Notepad_plus::markAll(const wchar_t* text, int styleID)
{
    if (!text || !_pEditView)
        return;

    std::wstring searchText(text);
    if (searchText.empty())
        return;

    // Use the find/replace dialog's markAll function
    _findReplaceDlg.markAll(searchText.c_str(), styleID);
}

bool Notepad_plus::goToNextIndicator(int indicID2Search, bool isWrap) const
{
    if (!_pEditView)
        return false;

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);
    intptr_t endPos = _pEditView->execute(SCI_GETTEXTLENGTH);

    _pEditView->execute(SCI_SETINDICATORCURRENT, indicID2Search);

    // Use SCI_INDICATOREND to efficiently skip to next indicator boundary
    intptr_t pos = currentPos + 1;
    while (pos < endPos)
    {
        intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
        if (value > 0)
        {
            // Found indicator - jump to its start
            intptr_t indicStart = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
            _pEditView->execute(SCI_GOTOPOS, indicStart);
            return true;
        }
        // Jump to end of current non-indicator region
        intptr_t nextBoundary = _pEditView->execute(SCI_INDICATOREND, indicID2Search, pos);
        if (nextBoundary <= pos)
            break;
        pos = nextBoundary;
    }

    // Wrap around if enabled
    if (isWrap)
    {
        pos = 0;
        while (pos < currentPos)
        {
            intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
            if (value > 0)
            {
                intptr_t indicStart = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
                _pEditView->execute(SCI_GOTOPOS, indicStart);
                return true;
            }
            intptr_t nextBoundary = _pEditView->execute(SCI_INDICATOREND, indicID2Search, pos);
            if (nextBoundary <= pos)
                break;
            pos = nextBoundary;
        }
    }

    return false;
}

bool Notepad_plus::goToPreviousIndicator(int indicID2Search, bool isWrap) const
{
    if (!_pEditView)
        return false;

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);

    _pEditView->execute(SCI_SETINDICATORCURRENT, indicID2Search);

    // Search backward for the indicator using SCI_INDICATORSTART
    intptr_t pos = currentPos - 1;
    while (pos >= 0)
    {
        intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
        if (value > 0)
        {
            // Found indicator - jump to its start
            intptr_t indicStart = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
            _pEditView->execute(SCI_GOTOPOS, indicStart);
            return true;
        }
        // Jump backwards to start of current non-indicator region
        intptr_t prevBoundary = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
        if (prevBoundary >= pos)
        {
            --pos;
        }
        else
        {
            pos = prevBoundary - 1;
        }
    }

    // Wrap around if enabled
    if (isWrap)
    {
        intptr_t endPos = _pEditView->execute(SCI_GETTEXTLENGTH);
        pos = endPos - 1;
        while (pos > currentPos)
        {
            intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
            if (value > 0)
            {
                intptr_t indicStart = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
                _pEditView->execute(SCI_GOTOPOS, indicStart);
                return true;
            }
            intptr_t prevBoundary = _pEditView->execute(SCI_INDICATORSTART, indicID2Search, pos);
            if (prevBoundary >= pos)
            {
                --pos;
            }
            else
            {
                pos = prevBoundary - 1;
            }
        }
    }

    return false;
}

void Notepad_plus::showFindCharsInRangeDlg()
{
    // Show the Find Characters in Range dialog
    _findCharsInRangeDlg.doDialog(false);
}

// ============================================================================
// Panel Switching Operations
// ============================================================================

void Notepad_plus::switchToFunctionList()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	if (!mainWin->isPanelVisible("functionList"))
	{
		mainWin->showPanel("functionList", true);
	}

	auto* funcList = mainWin->getFunctionListPanel();
	if (funcList)
	{
		QWidget* w = funcList->getWidget();
		if (w)
			w->setFocus();
	}
}

void Notepad_plus::switchToDocumentList()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	auto* dockMgr = mainWin->getDockingManager();
	if (!dockMgr)
		return;

	if (!dockMgr->isPanelVisible("documentList"))
	{
		mainWin->showPanel("documentList", true);
	}

	// Focus the document list widget
	QWidget* panelWidget = dockMgr->getPanelWidget("documentList");
	if (panelWidget)
	{
		panelWidget->setFocus();
	}
}

// ============================================================================
// View and Document Activation Operations
// ============================================================================

int Notepad_plus::switchEditViewTo(int gid)
{
    // Switch to the specified edit view (MAIN_VIEW or SUB_VIEW)
    if (gid == MAIN_VIEW)
    {
        if (_mainWindowStatus & WindowMainActive)
        {
            _activeView = MAIN_VIEW;
            _pEditView = &_mainEditView;
            _pDocTab = &_mainDocTab;
            _pNonEditView = &_subEditView;
            _pNonDocTab = &_subDocTab;
            // Only grab focus if the widget is visible; calling
            // SCI_GRABFOCUS before the window is shown can crash
            // or cause unexpected behaviour in Qt.
            QWidget* w = _pEditView->getWidget();
            if (w && w->isVisible())
            {
                _pEditView->execute(SCI_GRABFOCUS);
            }
        }
    }
    else if (gid == SUB_VIEW)
    {
        if (_mainWindowStatus & WindowSubActive)
        {
            _activeView = SUB_VIEW;
            _pEditView = &_subEditView;
            _pDocTab = &_subDocTab;
            _pNonEditView = &_mainEditView;
            _pNonDocTab = &_mainDocTab;
            // Only grab focus if the widget is visible
            QWidget* w = _pEditView->getWidget();
            if (w && w->isVisible())
            {
                _pEditView->execute(SCI_GRABFOCUS);
            }
        }
    }
    return _activeView;
}

void Notepad_plus::activateDoc(size_t pos)
{
    // Activate document at specified position in current view
    if (_pDocTab && pos < _pDocTab->nbItem())
    {
        BufferID id = _pDocTab->getBufferByIndex(pos);
        if (id != BUFFER_INVALID)
        {
            switchToFile(id);
        }
    }
}

void Notepad_plus::activateNextDoc(bool direction)
{
    // Activate next/previous document in current view
    if (!_pDocTab)
        return;

    size_t currentIndex = _pDocTab->getCurrentTabIndex();
    size_t nbItems = _pDocTab->nbItem();

    if (nbItems <= 1)
        return;

    size_t newIndex;
    if (direction)
    {
        // Next tab
        newIndex = (currentIndex + 1) % nbItems;
    }
    else
    {
        // Previous tab
        newIndex = (currentIndex == 0) ? nbItems - 1 : currentIndex - 1;
    }

    activateDoc(newIndex);
}

// ============================================================================
// Tab Movement Operations
// ============================================================================

void Notepad_plus::moveTabForward()
{
    // Move current tab forward in the tab bar
    if (!_pDocTab)
        return;

    int currentIndex = _pDocTab->getCurrentTabIndex();
    if (currentIndex < 0)
        return;

    size_t nbItems = _pDocTab->nbItem();
    if (nbItems <= 1 || static_cast<size_t>(currentIndex) >= nbItems - 1)
        return;

    // Use TabBarPlus's exchangeTabItemData to swap with next tab
    // This is a simplified implementation - just swap the buffer IDs
    BufferID currentID = _pDocTab->getBufferByIndex(currentIndex);
    BufferID nextID = _pDocTab->getBufferByIndex(currentIndex + 1);

    // Set the buffers in swapped positions
    _pDocTab->setBuffer(currentIndex, nextID);
    _pDocTab->setBuffer(currentIndex + 1, currentID);
    _pDocTab->activateBuffer(currentID);

    // Notify plugins of tab order change
    SCNotification scnN{};
    scnN.nmhdr.code = NPPN_DOCORDERCHANGED;
    scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(_pDocTab);
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(currentID);
    _pluginsManager.notify(&scnN);
}

void Notepad_plus::moveTabBackward()
{
    // Move current tab backward in the tab bar
    if (!_pDocTab)
        return;

    int currentIndex = _pDocTab->getCurrentTabIndex();
    if (currentIndex <= 0)
        return;

    size_t nbItems = _pDocTab->nbItem();
    if (nbItems <= 1)
        return;

    // Swap with previous tab
    BufferID currentID = _pDocTab->getBufferByIndex(currentIndex);
    BufferID prevID = _pDocTab->getBufferByIndex(currentIndex - 1);

    // Set the buffers in swapped positions
    _pDocTab->setBuffer(currentIndex, prevID);
    _pDocTab->setBuffer(currentIndex - 1, currentID);
    _pDocTab->activateBuffer(currentID);

    // Notify plugins of tab order change
    SCNotification scnN{};
    scnN.nmhdr.code = NPPN_DOCORDERCHANGED;
    scnN.nmhdr.hwndFrom = reinterpret_cast<void*>(_pDocTab);
    scnN.nmhdr.idFrom = reinterpret_cast<uptr_t>(currentID);
    _pluginsManager.notify(&scnN);
}

// ============================================================================
// Macro Operations
// ============================================================================

void Notepad_plus::startMacroRecording()
{
    // Start recording a macro
    if (_recordingMacro)
        return;

    _recordingMacro = true;
    _macro.clear();
    _recordingSaved = false;

    // Notify Scintilla to start recording
    _pEditView->execute(SCI_STARTRECORD);
}

void Notepad_plus::stopMacroRecording()
{
    // Stop recording a macro
    if (!_recordingMacro)
        return;

    _recordingMacro = false;
    _pEditView->execute(SCI_STOPRECORD);

    // If macro is not empty, mark it as saved
    if (!_macro.empty())
    {
        _recordingSaved = true;
    }
}

void Notepad_plus::macroPlayback()
{
    // Playback the recorded macro
    if (_recordingMacro || _macro.empty())
        return;

    macroPlayback(_macro);
}

void Notepad_plus::macroPlayback(const Macro& macro)
{
    _playingBackMacro = true;

    _pEditView->execute(SCI_BEGINUNDOACTION);

    for (size_t i = 0; i < macro.size(); ++i)
    {
        const recordedMacroStep& step = macro[i];
        if (step.isScintillaMacro())
        {
            if (step._macroType == recordedMacroStep::mtUseSParameter)
            {
                _pEditView->execute(step._message, step._wParameter,
                    reinterpret_cast<sptr_t>(step._sParameter.c_str()));
            }
            else
            {
                _pEditView->execute(step._message, step._wParameter, step._lParameter);
            }
        }
        // Menu commands (mtMenuCommand) would require wiring to the command
        // handler; Scintilla-level commands cover the majority of macro use cases.
    }

    _pEditView->execute(SCI_ENDUNDOACTION);

    _playingBackMacro = false;
}

void Notepad_plus::saveCurrentMacro()
{
    if (_macro.empty())
        return;

    addCurrentMacro();
}

void Notepad_plus::showRunMacroDlg()
{
    static bool connected = false;
    if (!connected)
    {
        QObject::connect(&_runMacroDlg, &QtControls::RunMacroDlg::runMacroRequested,
            [this]() { runMacroFromDlg(); });
        connected = true;
    }

    _runMacroDlg.setHasRecordedMacro(!_macro.empty() && _recordingSaved);
    _runMacroDlg.initMacroList();
    _runMacroDlg.doDialog(false);
}

void Notepad_plus::runMacroFromDlg()
{
    if (_recordingMacro)
        return;

    int times = _runMacroDlg.isMulti() ? _runMacroDlg.getTimes() : -1;
    int indexMacro = _runMacroDlg.getMacro2Exec();

    Macro m = _macro;

    if (indexMacro != -1)
    {
        NppParameters& nppParam = NppParameters::getInstance();
        std::vector<MacroShortcut>& ms = nppParam.getMacroList();
        if (indexMacro >= 0 && indexMacro < static_cast<int>(ms.size()))
        {
            m = ms[indexMacro].getMacro();
        }
    }

    if (m.empty())
        return;

    int counter = 0;
    intptr_t lastLine = _pEditView->execute(SCI_GETLINECOUNT) - 1;
    intptr_t currLine = _pEditView->execute(SCI_LINEFROMPOSITION,
        _pEditView->execute(SCI_GETCURRENTPOS));
    intptr_t deltaLastLine = 0;
    intptr_t deltaCurrLine = 0;
    bool cursorMovedUp = false;

    for (;;)
    {
        macroPlayback(m);
        ++counter;
        if (times >= 0)
        {
            if (counter >= times)
                break;
        }
        else // run until EOF
        {
            intptr_t newLastLine = _pEditView->execute(SCI_GETLINECOUNT) - 1;
            intptr_t newCurrLine = _pEditView->execute(SCI_LINEFROMPOSITION,
                _pEditView->execute(SCI_GETCURRENTPOS));

            deltaLastLine = newLastLine - lastLine;
            deltaCurrLine = newCurrLine - currLine;

            if (counter > 2 && cursorMovedUp != (deltaCurrLine < 0) && deltaLastLine >= 0)
                break;

            cursorMovedUp = deltaCurrLine < 0;

            if ((deltaCurrLine == 0) && (deltaLastLine >= 0))
                break;

            if (deltaLastLine < deltaCurrLine)
                lastLine += deltaLastLine;

            currLine += deltaCurrLine;

            if ((currLine > lastLine) || (currLine < 0)
                || ((deltaCurrLine == 0) && (currLine == 0) && ((deltaLastLine >= 0) || cursorMovedUp)))
            {
                break;
            }
        }
    }
}

// ============================================================================
// Encoding and Dialog Operations
// ============================================================================

void Notepad_plus::setEncoding(int encoding)
{
    // Set the encoding for the current buffer
    Buffer* buf = getCurrentBuffer();
    if (!buf)
        return;

    UniMode mode = uni8Bit;
    switch (encoding)
    {
        case 0: // ANSI
            mode = uni8Bit;
            break;
        case 1: // UTF-8
            mode = uniUTF8;
            break;
        case 2: // UTF-16 BE
            mode = uni16BE;
            break;
        case 3: // UTF-16 LE
            mode = uni16LE;
            break;
        case 4: // UTF-8 without BOM
            mode = uniUTF8_NoBOM;
            break;
        default:
            return;
    }

    buf->setUnicodeMode(mode);
    // Update UI to reflect encoding change
}

void Notepad_plus::showUserDefineDlg()
{
    // Show the User Defined Language dialog
    // This dialog allows users to define custom language syntax highlighting
    // For Qt, this would show the UserDefineDialog
    // TODO: Implement when UserDefineDialog is fully ported
}

void Notepad_plus::showRunDlg()
{
    // Ensure the Run dialog has access to this Notepad_plus instance
    _runDlg.setNotepadPlus(this);
    _runDlg.doDialog(false);
}

void Notepad_plus::showPreferenceDlg()
{
    // Show the Preferences dialog
    _preference.doDialog(false);
}

// ============================================================================
// Panel Launching Operations
// ============================================================================

void Notepad_plus::launchDocumentListPanel(bool changeFromBtnCmd)
{
	Q_UNUSED(changeFromBtnCmd);

	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	auto* dockMgr = mainWin->getDockingManager();
	if (!dockMgr)
		return;

	// Check if the document list panel is already registered with the dock manager
	if (!dockMgr->isPanelVisible("documentList") && !dockMgr->hasPanel("documentList"))
	{
		// Create and register the document list panel
		auto* docListPanel = new QtControls::VerticalFileSwitcher(mainWin);
		docListPanel->init(&_pEditView);
		dockMgr->addPanel("documentList", docListPanel->getWidget(),
			QtControls::DockingManager::DockArea::Left, QObject::tr("Document List"));
	}

	// Toggle visibility
	if (dockMgr->isPanelVisible("documentList"))
	{
		dockMgr->hidePanel("documentList");
	}
	else
	{
		dockMgr->showPanel("documentList");
	}
}

void Notepad_plus::launchDocMap()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	// Toggle visibility of the document map panel
	if (mainWin->isPanelVisible("documentMap"))
	{
		mainWin->showPanel("documentMap", false);
	}
	else
	{
		mainWin->showPanel("documentMap", true);

		// Initialize the map with current editor content
		auto* docMap = mainWin->getDocumentMap();
		if (docMap)
		{
			docMap->init(&_pEditView);
			docMap->wrapMap();
			docMap->scrollMap();
		}
	}
}

void Notepad_plus::launchFunctionList()
{
	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	// Toggle visibility of the function list panel
	if (mainWin->isPanelVisible("functionList"))
	{
		mainWin->showPanel("functionList", false);
	}
	else
	{
		mainWin->showPanel("functionList", true);

		// Initialize and parse current document
		auto* funcList = mainWin->getFunctionListPanel();
		if (funcList)
		{
			funcList->init(&_pEditView);
			funcList->parseCurrentDocument();
		}
	}
}

void Notepad_plus::launchProjectPanel(int cmdID, ProjectPanel** ppProjPanel, int panelID)
{
	Q_UNUSED(cmdID);
	Q_UNUSED(ppProjPanel);

	auto* mainWin = getMainWindow(_mainEditView);
	if (!mainWin)
		return;

	auto* dockMgr = mainWin->getDockingManager();
	if (!dockMgr)
		return;

	// Build panel name based on panelID (support up to 3 project panels)
	QString panelName = (panelID == 0) ? "projectPanel" :
		QString("projectPanel_%1").arg(panelID + 1);

	// If the panel does not yet exist in the dock manager, create it
	if (!dockMgr->hasPanel(panelName))
	{
		auto* projPanel = new QtControls::ProjectPanel(mainWin);
		projPanel->init(&_pEditView);
		projPanel->setPanelTitle(QObject::tr("Project %1").arg(panelID + 1));
		dockMgr->addPanel(panelName, projPanel->getWidget(),
			QtControls::DockingManager::DockArea::Left,
			QObject::tr("Project %1").arg(panelID + 1));
	}

	// Toggle visibility
	if (dockMgr->isPanelVisible(panelName))
	{
		dockMgr->hidePanel(panelName);
	}
	else
	{
		dockMgr->showPanel(panelName);
	}
}

// ============================================================================
// Macro Operations
// ============================================================================

bool Notepad_plus::addCurrentMacro()
{
    if (_macro.empty())
        return false;

    NppParameters& nppParams = NppParameters::getInstance();
    std::vector<MacroShortcut>& theMacros = nppParams.getMacroList();

    // Show a dialog to get the macro name from the user
    bool ok = false;
    QString macroName = QInputDialog::getText(
        nullptr,
        QObject::tr("Save Current Macro"),
        QObject::tr("Enter a name for the macro:"),
        QLineEdit::Normal,
        QObject::tr("My Macro"),
        &ok);

    if (!ok || macroName.isEmpty())
        return false;

    int nbMacro = static_cast<int>(theMacros.size());
    int cmdID = ID_MACRO + nbMacro;

    std::string name = macroName.toStdString();
    Shortcut sc(name.c_str(), false, false, false, 0);
    MacroShortcut ms(sc, _macro, cmdID);

    theMacros.push_back(ms);
    nppParams.getMacroMenuItems().push_back(
        MenuItemUnit(cmdID, string2wstring(name, CP_UTF8)));
    nppParams.setShortcutDirty();

    _recordingSaved = true;

    return true;
}

// ============================================================================
// Session Operations
// ============================================================================

void Notepad_plus::loadLastSession()
{
    std::cout << "[loadLastSession] Starting..." << std::endl;
    NppParameters& nppParams = NppParameters::getInstance();
    const NppGUI& nppGui = nppParams.getNppGUI();
    Session lastSession = nppParams.getSession();
    bool isSnapshotMode = nppGui.isSnapshotMode();
    std::cout << "[loadLastSession] Session has " << lastSession.nbMainFiles() << " main files and "
             << lastSession.nbSubFiles() << " sub files. isSnapshotMode: " << isSnapshotMode << std::endl;
    _isFolding = true;
    bool result = loadSession(lastSession, isSnapshotMode);
    _isFolding = false;
    std::cout << "[loadLastSession] Completed. Result: " << result << std::endl;
}

// ============================================================================
// Session Saving Operations
// ============================================================================

void Notepad_plus::saveSession(const Session& session)
{
    NppParameters::getInstance().writeSession(session);
}

void Notepad_plus::saveCurrentSession()
{
    NppParameters& nppParam = NppParameters::getInstance();
    NppGUI& nppGUI = const_cast<NppGUI&>(nppParam.getNppGUI());

    if (!nppGUI._rememberLastSession || nppGUI._isCmdlineNosessionActivated)
        return;

    Session currentSession;
    getCurrentOpenedFiles(currentSession);
    saveSession(currentSession);
}

std::wstring Notepad_plus::getLangFromMenu(const Buffer* buf)
{
    // Qt version: simplified implementation that returns the language type name
    if (!buf)
        return L"";

    // For user-defined languages, return the UDL name
    // Note: isUserDefineLangExt() and getUserDefineLangName() may not be available in Qt version
    // For now, just return the language type as a string
    LangType langType = buf->getLangType();

    // Convert LangType to a readable name
    // This is a simplified version - full implementation would map all language types
    switch (langType)
    {
        case L_TEXT:
            return L"Normal text";
        case L_CPP:
            return L"C++";
        case L_JAVA:
            return L"Java";
        case L_PYTHON:
            return L"Python";
        case L_JAVASCRIPT:
            return L"JavaScript";
        case L_HTML:
            return L"HTML";
        case L_XML:
            return L"XML";
        case L_CSS:
            return L"CSS";
        case L_PHP:
            return L"PHP";
        default:
            return L"";
    }
}

void Notepad_plus::getCurrentOpenedFiles(Session& session, bool includeUntitledDoc)
{
    // Save position so it will be correct in the session
    _mainEditView.saveCurrentPos();
    _subEditView.saveCurrentPos();

    session._activeView = currentView();
    session._activeMainIndex = _mainDocTab.getCurrentTabIndex();
    session._activeSubIndex = _subDocTab.getCurrentTabIndex();

    const int nbElem = 2;
    DocTabView* docTab[nbElem]{};
    docTab[0] = &_mainDocTab;
    docTab[1] = &_subDocTab;

    for (int k = 0; k < nbElem; ++k)
    {
        for (size_t i = 0, len = docTab[k]->nbItem(); i < len; ++i)
        {
            BufferID bufID = docTab[k]->getBufferByIndex(i);
            size_t activeIndex = (k == 0) ? session._activeMainIndex : session._activeSubIndex;
            std::vector<sessionFileInfo>* viewFiles = (k == 0) ? &(session._mainViewFiles) : &(session._subViewFiles);

            Buffer* buf = MainFileManager.getBufferByID(bufID);
            if (!buf)
                continue;

            // Skip empty untitled documents
            if (buf->isUntitled() && buf->docLength() == 0)
                continue;

            if (!includeUntitledDoc)
                if (!doesFileExist(buf->getFullPathName()))
                    continue;

            // Get language name
            std::wstring languageName = getLangFromMenu(buf);
            if (languageName.empty())
            {
                NppParameters& nppParam = NppParameters::getInstance();
                const NppGUI& nppGUI = nppParam.getNppGUI();

                for (size_t j = 0; j < nppGUI._excludedLangList.size(); ++j)
                {
                    if (buf->getLangType() == nppGUI._excludedLangList[j]._langType)
                    {
                        languageName = nppGUI._excludedLangList[j]._langName;
                        break;
                    }
                }
            }

            const wchar_t* langName = languageName.c_str();

            // Get position from the appropriate view
            ScintillaEditView* editView = (k == 0) ? &_mainEditView : &_subEditView;
            Position pos = buf->getPosition(editView);

            // Convert QtCore::MapPosition to global MapPosition
            QtCore::MapPosition qtMapPos = buf->getMapPosition();
            MapPosition mapPos;
            mapPos._firstVisibleDisplayLine = qtMapPos.firstVisibleDisplayLine;
            mapPos._firstVisibleDocLine = qtMapPos.firstVisibleDocLine;
            mapPos._lastVisibleDocLine = qtMapPos.lastVisibleDocLine;
            mapPos._nbLine = qtMapPos.nbLine;
            mapPos._higherPos = qtMapPos.higherPos;
            mapPos._width = qtMapPos.width;
            mapPos._height = qtMapPos.height;
            mapPos._wrapIndentMode = -1;  // Default value, not available in QtCore::MapPosition
            mapPos._KByteInDoc = MapPosition::getMaxPeekLenInKB();  // Default value
            mapPos._isWrap = qtMapPos.isWrap;

            sessionFileInfo sfi(
                buf->getFullPathName(),
                langName,
                buf->getEncodingNumber(),
                buf->isUserReadOnly(),
                buf->isPinned(),
                buf->isUntitledTabRenamed(),
                pos,
                buf->getBackupFileName().c_str(),
                buf->getLastModifiedFileTimestamp(),
                mapPos
            );

            sfi._isMonitoring = buf->isMonitoringOn();
            sfi._individualTabColour = docTab[k]->getIndividualTabColourId(static_cast<int>(i));
            sfi._isRTL = buf->isRTL();

            // Get fold states for active tab
            if (i == activeIndex)
            {
                editView->getCurrentFoldStates(sfi._foldStates);
            }
            else
            {
                sfi._foldStates = buf->getHeaderLineState(editView);
            }

            viewFiles->push_back(sfi);
        }
    }
}

// ============================================================================
// Dual View Operations (Move/Clone to Other View)
// ============================================================================

void Notepad_plus::docGotoAnotherEditView(FileTransferMode mode)
{
	// Get current buffer
	BufferID current = _pEditView->getCurrentBufferID();
	Buffer* buf = MainFileManager.getBufferByID(current);
	if (!buf)
		return;

	// If moving and this is the only doc in a single-view setup,
	// show the other view first so we don't leave an empty tab bar
	if (mode == TransferMove)
	{
		if (_pDocTab->nbItem() == 1 && !viewVisible(otherView()))
		{
			showView(otherView());
		}
	}

	// Save pinned and monitoring state before any move
	bool wasPinned = buf->isPinned();
	bool wasMonitoring = buf->isMonitoringOn();

	int viewToGo = otherView();
	int indexFound = _pNonDocTab->getIndexByBuffer(current);

	if (indexFound != -1)
	{
		// Already in other view, just activate it
		activateBuffer(current, viewToGo);
	}
	else
	{
		// Save current position before moving
		if (_pEditView->isVisible() && _pNonEditView->isVisible())
		{
			_pNonEditView->saveCurrentPos();
		}

		// Load buffer into the other view
		loadBufferIntoView(current, viewToGo);

		// Copy position from current view to new view
		_pEditView->saveCurrentPos();
		buf->setPosition(buf->getPosition(_pEditView), _pNonEditView);
		_pNonEditView->restoreCurrentPosPreStep();

		// Activate in the target view
		activateBuffer(current, viewToGo);
	}

	// Show the target view if it was hidden
	int viewToOpen = (viewToGo == SUB_VIEW ? WindowSubActive : WindowMainActive);
	if (!(_mainWindowStatus & viewToOpen))
	{
		showView(viewToGo);
	}

	// Close the document from source view if moving (not cloning)
	if (mode == TransferMove)
	{
		doClose(_pEditView->getCurrentBufferID(), currentView());
	}

	// Switch focus to the target view
	switchEditViewTo(viewToGo);

	// If the source view is now empty, hide it
	if (mode == TransferMove)
	{
		int sourceView = otherFromView(viewToGo);
		if (canHideView(sourceView))
			hideView(sourceView);
	}

	// Restore pinned state in target view
	if (wasPinned)
	{
		buf->setPinned(true);
	}

	// Restore monitoring state in target view
	if (wasMonitoring)
	{
		monitoringStartOrStopAndUpdateUI(buf, true);
	}
}

// ============================================================================
// View Visibility Operations
// ============================================================================

void Notepad_plus::showView(int whichOne)
{
    if (viewVisible(whichOne))  //no use making visible view visible
        return;

    if (whichOne == MAIN_VIEW)
    {
        _mainWindowStatus |= WindowMainActive;
        _mainEditView.display(true);
        _mainDocTab.display(true);
        // Show the container widget in the splitter
        QWidget* editWidget = _mainEditView.getWidget();
        if (editWidget)
        {
            QWidget* container = editWidget->parentWidget();
            if (container)
                container->show();
        }
    }
    else if (whichOne == SUB_VIEW)
    {
        _mainWindowStatus |= WindowSubActive;
        _subEditView.display(true);
        _subDocTab.display(true);
        // Show the container widget in the splitter
        QWidget* editWidget = _subEditView.getWidget();
        if (editWidget)
        {
            QWidget* container = editWidget->parentWidget();
            if (container)
                container->show();
        }
    }
}

bool Notepad_plus::viewVisible(int whichOne)
{
    int viewToCheck = (whichOne == SUB_VIEW ? WindowSubActive : WindowMainActive);
    return (_mainWindowStatus & viewToCheck) != 0;
}

// ============================================================================
// Session Loading
// ============================================================================

bool Notepad_plus::loadSession(Session& session, bool isSnapshotMode, const wchar_t* userCreatedSessionName)
{
    Q_UNUSED(userCreatedSessionName);

    std::cout << "[loadSession] Starting... isSnapshotMode: " << isSnapshotMode
             << " main files: " << session.nbMainFiles() << " sub files: " << session.nbSubFiles() << std::endl;

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();

    nppParam.setTheWarningHasBeenGiven(false);

    bool allSessionFilesLoaded = true;
    BufferID lastOpened = BUFFER_INVALID;

    showView(MAIN_VIEW);
    switchEditViewTo(MAIN_VIEW);

    // If no session files, just set up RTL if needed
    if (!session.nbMainFiles() && !session.nbSubFiles())
    {
        std::cout << "[loadSession] Empty session - checking if we need to create initial buffer" << std::endl;
        Buffer* buf = getCurrentBuffer();
        if (!buf)
        {
            // No buffer exists yet - this is normal for a fresh start with no session
            std::cout << "[loadSession] No current buffer - creating initial empty document" << std::endl;
            // Create a new empty document to have a valid buffer
            fileNew();
            buf = getCurrentBuffer();
            if (!buf)
            {
                std::cerr << "[loadSession] Failed to create initial buffer!" << std::endl;
                return false;
            }
        }
        if (nppParam.getNativeLangSpeaker()->isRTL() && nppParam.getNativeLangSpeaker()->isEditZoneRTL())
            buf->setRTL(true);
        _mainEditView.changeTextDirection(buf->isRTL());
        std::cout << "[loadSession] Empty session handled successfully" << std::endl;
        return true;
    }

    // Load main view files
    for (size_t i = 0; i < session.nbMainFiles(); )
    {
        const wchar_t* pFn = session._mainViewFiles[i]._fileName.c_str();

        if (isFileSession(pFn) || isFileWorkspace(pFn))
        {
            session._mainViewFiles.erase(session._mainViewFiles.begin() + i);
            continue;
        }

        // Check if file exists
        if (QFile::exists(QString::fromStdWString(pFn)))
        {
            if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty())
                lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding,
                                   session._mainViewFiles[i]._backupFilePath.c_str(),
                                   session._mainViewFiles[i]._originalFileLastModifTimestamp);
            else
                lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding);
        }
        else if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty() &&
                 QFile::exists(QString::fromStdWString(session._mainViewFiles[i]._backupFilePath)))
        {
            lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding,
                               session._mainViewFiles[i]._backupFilePath.c_str(),
                               session._mainViewFiles[i]._originalFileLastModifTimestamp);
        }
        else
        {
            // File doesn't exist - try to find if already open or create placeholder
            BufferID foundBufID = MainFileManager.getBufferFromName(pFn);
            if (foundBufID == BUFFER_INVALID)
            {
                // For now, skip absent files (placeholder documents not implemented)
                lastOpened = BUFFER_INVALID;
            }
        }

        if (lastOpened != BUFFER_INVALID)
        {
            showView(MAIN_VIEW);
            Buffer* buf = MainFileManager.getBufferByID(lastOpened);

            // Set language type
            const wchar_t* pLn = session._mainViewFiles[i]._langName.c_str();
            LangType langTypeToSet = L_TEXT;

            // Try to determine language from menu name
            // Simplified: just use text for now
            if (pLn && *pLn)
            {
                // TODO: Map language name to LangType
                langTypeToSet = L_TEXT;
            }

            // Set position and other properties
            buf->setPosition(session._mainViewFiles[i], &_mainEditView);
            buf->setLangType(langTypeToSet);
            // TODO: Handle user language name (pLn) if needed
            if (session._mainViewFiles[i]._encoding != -1)
                buf->setEncodingNumber(session._mainViewFiles[i]._encoding);

            buf->setUserReadOnly(session._mainViewFiles[i]._isUserReadOnly ||
                                nppGUI._isFullReadOnly ||
                                nppGUI._isFullReadOnlySavingForbidden);
            buf->setPinned(session._mainViewFiles[i]._isPinned);
            // Note: setUntitledTabRenamedStatus not implemented in Qt version

            if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty())
                buf->setDirty(true);

            buf->setRTL(session._mainViewFiles[i]._isRTL);
            if (i == 0 && session._activeMainIndex == 0)
                _mainEditView.changeTextDirection(buf->isRTL());

            ++i;
        }
        else
        {
            session._mainViewFiles.erase(session._mainViewFiles.begin() + i);
            allSessionFilesLoaded = false;
        }
    }

    // Load sub view files
    showView(SUB_VIEW);
    switchEditViewTo(SUB_VIEW);

    for (size_t k = 0; k < session.nbSubFiles(); )
    {
        const wchar_t* pFn = session._subViewFiles[k]._fileName.c_str();

        if (isFileSession(pFn) || isFileWorkspace(pFn))
        {
            session._subViewFiles.erase(session._subViewFiles.begin() + k);
            continue;
        }

        if (QFile::exists(QString::fromStdWString(pFn)) ||
            (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty() &&
             QFile::exists(QString::fromStdWString(session._subViewFiles[k]._backupFilePath))))
        {
            // Check if already open in main view - if so, clone it
            BufferID clonedBuf = _mainDocTab.findBufferByName(pFn);
            if (clonedBuf != BUFFER_INVALID)
            {
                loadBufferIntoView(clonedBuf, SUB_VIEW);
                lastOpened = clonedBuf;
            }
            else
            {
                if (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty())
                    lastOpened = doOpen(pFn, false, false, session._subViewFiles[k]._encoding,
                                       session._subViewFiles[k]._backupFilePath.c_str(),
                                       session._subViewFiles[k]._originalFileLastModifTimestamp);
                else
                    lastOpened = doOpen(pFn, false, false, session._subViewFiles[k]._encoding);
            }
        }
        else
        {
            BufferID foundBufID = MainFileManager.getBufferFromName(pFn);
            if (foundBufID == BUFFER_INVALID)
            {
                lastOpened = BUFFER_INVALID;
            }
        }

        if (lastOpened != BUFFER_INVALID)
        {
            showView(SUB_VIEW);
            Buffer* buf = MainFileManager.getBufferByID(lastOpened);

            const wchar_t* pLn = session._subViewFiles[k]._langName.c_str();
            LangType typeToSet = L_TEXT;

            buf->setPosition(session._subViewFiles[k], &_subEditView);
            buf->setLangType(typeToSet);
            // TODO: Handle user language name (pLn) if needed
            buf->setEncodingNumber(session._subViewFiles[k]._encoding);
            buf->setUserReadOnly(session._subViewFiles[k]._isUserReadOnly ||
                                nppGUI._isFullReadOnly ||
                                nppGUI._isFullReadOnlySavingForbidden);
            buf->setPinned(session._subViewFiles[k]._isPinned);
            // Note: setUntitledTabRenamedStatus not implemented in Qt version

            if (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty())
                buf->setDirty(true);

            buf->setRTL(session._subViewFiles[k]._isRTL);

            ++k;
        }
        else
        {
            session._subViewFiles.erase(session._subViewFiles.begin() + k);
            allSessionFilesLoaded = false;
        }
    }

    // Activate the appropriate files
    if (session._activeMainIndex < session._mainViewFiles.size())
    {
        const wchar_t* fileName = session._mainViewFiles[session._activeMainIndex]._fileName.c_str();
        BufferID buf = _mainDocTab.findBufferByName(fileName);
        if (buf != BUFFER_INVALID)
            activateBuffer(buf, MAIN_VIEW);
    }

    if (session._activeSubIndex < session._subViewFiles.size())
    {
        const wchar_t* fileName = session._subViewFiles[session._activeSubIndex]._fileName.c_str();
        BufferID buf = _subDocTab.findBufferByName(fileName);
        if (buf != BUFFER_INVALID)
            activateBuffer(buf, SUB_VIEW);
    }

    // Switch to the active view
    if ((session.nbSubFiles() > 0) &&
        (session._activeView == MAIN_VIEW || session._activeView == SUB_VIEW))
        switchEditViewTo(static_cast<int32_t>(session._activeView));
    else
        switchEditViewTo(MAIN_VIEW);

    // Hide empty views
    if (canHideView(otherView()))
        hideView(otherView());
    else if (canHideView(currentView()))
        hideView(currentView());

    checkSyncState();

    return allSessionFilesLoaded;
}

bool Notepad_plus::isConditionExprLine(intptr_t lineNumber)
{
	if (lineNumber < 0 || lineNumber > _pEditView->execute(SCI_GETLINECOUNT))
		return false;

	auto startPos = _pEditView->execute(SCI_POSITIONFROMLINE, lineNumber);
	auto endPos = _pEditView->execute(SCI_GETLINEENDPOSITION, lineNumber);
	_pEditView->execute(SCI_SETSEARCHFLAGS, SCFIND_REGEXP | SCFIND_POSIX);
	_pEditView->execute(SCI_SETTARGETRANGE, startPos, endPos);

	const char ifElseForWhileExpr[] = "((else[ \t]+)?if|for|while)[ \t]*[(].*[)][ \t]*|else[ \t]*";

	auto posFound = _pEditView->execute(SCI_SEARCHINTARGET, strlen(ifElseForWhileExpr), reinterpret_cast<LPARAM>(ifElseForWhileExpr));
	if (posFound >= 0)
	{
		auto end = _pEditView->execute(SCI_GETTARGETEND);
		if (end == endPos)
			return true;
	}

	return false;
}

intptr_t Notepad_plus::findMachedBracePos(size_t startPos, size_t endPos, char targetSymbol, char matchedSymbol)
{
	if (startPos == endPos)
		return -1;

	if (startPos > endPos) // backward
	{
		int balance = 0;
		for (intptr_t i = startPos; i >= static_cast<intptr_t>(endPos); --i)
		{
			char aChar = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, i));
			if (aChar == targetSymbol)
			{
				if (balance == 0)
					return i;
				--balance;
			}
			else if (aChar == matchedSymbol)
			{
				++balance;
			}
		}
	}
	else // forward
	{
	}
	return -1;
}

void Notepad_plus::maintainIndentation(wchar_t ch)
{
	const NppGUI& nppGui = NppParameters::getInstance().getNppGUI();
	if (nppGui._maintainIndent == autoIndent_none)
		return;

	intptr_t eolMode = _pEditView->execute(SCI_GETEOLMODE);
	intptr_t curLine = _pEditView->getCurrentLineNumber();
	intptr_t prevLine = curLine - 1;
	intptr_t indentAmountPrevLine = 0;
	intptr_t tabWidth = _pEditView->execute(SCI_GETTABWIDTH);

	// Do not alter indentation if we were at the beginning of the line and we pressed Enter
	if ((((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
		(eolMode == SC_EOL_CR && ch == '\r')) && prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
		return;

	LangType type = _pEditView->getCurrentBuffer()->getLangType();
	ExternalLexerAutoIndentMode autoIndentMode = ExternalLexerAutoIndentMode::Standard;

	// For external languages, query for custom auto-indentation functionality
	if (type >= L_EXTERNAL)
	{
		NppParameters& nppParam = NppParameters::getInstance();
		autoIndentMode = nppParam.getELCFromIndex(type - L_EXTERNAL)->_autoIndentMode;
		if (autoIndentMode == ExternalLexerAutoIndentMode::Custom)
			return;
	}

	if (nppGui._maintainIndent == autoIndent_basic) // Basic indentation mode only
	{
		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
				prevLine--;

			if (prevLine >= 0)
			{
				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			if (indentAmountPrevLine > 0)
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
		}

		return;
	}

	// else nppGui._maintainIndent == autoIndent_advanced

	if (type == L_C || type == L_CPP || type == L_JAVA || type == L_CS || type == L_OBJC ||
		type == L_PHP || type == L_JS_EMBEDDED || type == L_JAVASCRIPT || type == L_JSP || type == L_CSS || type == L_PERL ||
		type == L_RUST || type == L_POWERSHELL || type == L_JSON || type == L_JSON5 || type == L_TYPESCRIPT || type == L_GOLANG || type == L_SWIFT ||
		autoIndentMode == ExternalLexerAutoIndentMode::C_Like)
	{
		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
				prevLine--;

			// Get previous line's Indent
			if (prevLine >= 0)
			{
				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			// get previous char from current line
			intptr_t prevPos = _pEditView->execute(SCI_GETCURRENTPOS) - (eolMode == SC_EOL_CRLF ? 3 : 2);
			UCHAR prevChar = static_cast<UCHAR>(_pEditView->execute(SCI_GETCHARAT, prevPos));
			auto curPos = _pEditView->execute(SCI_GETCURRENTPOS);
			UCHAR nextChar = static_cast<UCHAR>(_pEditView->execute(SCI_GETCHARAT, curPos));

			if (prevChar == '{')
			{
				if (nextChar == '}')
				{
					const char* eolChars;
					if (eolMode == SC_EOL_CRLF)
						eolChars = "\r\n";
					else if (eolMode == SC_EOL_LF)
						eolChars = "\n";
					else
						eolChars = "\r";

					_pEditView->execute(SCI_INSERTTEXT, _pEditView->execute(SCI_GETCURRENTPOS), reinterpret_cast<LPARAM>(eolChars));
					_pEditView->setLineIndent(curLine + 1, indentAmountPrevLine);
				}
				_pEditView->setLineIndent(curLine, indentAmountPrevLine + tabWidth);
			}
			else if (nextChar == '{')
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
			// These languages do not support single line control structures without braces.
			else if (type == L_PERL || type == L_RUST || type == L_POWERSHELL || type == L_JSON || type == L_JSON5)
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
			else if (isConditionExprLine(prevLine))
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine + tabWidth);
			}
			else
			{
				if (indentAmountPrevLine > 0)
				{
					if (prevLine > 0 && isConditionExprLine(prevLine - 1))
						_pEditView->setLineIndent(curLine, indentAmountPrevLine - tabWidth);
					else
						_pEditView->setLineIndent(curLine, indentAmountPrevLine);
				}
			}
		}
		else if (ch == '{')
		{
			// if no character in front of {, aligned with prev line's indentation
			auto startPos = _pEditView->execute(SCI_POSITIONFROMLINE, curLine);
			LRESULT endPos = _pEditView->execute(SCI_GETCURRENTPOS);

			for (LRESULT i = endPos - 2; i > 0 && i >= startPos; --i)
			{
				UCHAR aChar = static_cast<UCHAR>(_pEditView->execute(SCI_GETCHARAT, i));
				if (aChar != ' ' && aChar != '\t')
					return;
			}

			// Search the non-empty previous line
			while (prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
				prevLine--;

			// Get previous line's Indent
			if (prevLine >= 0)
			{
				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);

				auto startPos2 = _pEditView->execute(SCI_POSITIONFROMLINE, prevLine);
				auto endPos2 = _pEditView->execute(SCI_GETLINEENDPOSITION, prevLine);
				_pEditView->execute(SCI_SETSEARCHFLAGS, SCFIND_REGEXP | SCFIND_POSIX);
				_pEditView->execute(SCI_SETTARGETRANGE, startPos2, endPos2);

				const char braceExpr[] = "[ \t]*\\{.*";

				intptr_t posFound = _pEditView->execute(SCI_SEARCHINTARGET, strlen(braceExpr), reinterpret_cast<LPARAM>(braceExpr));
				if (posFound >= 0)
				{
					auto end = _pEditView->execute(SCI_GETTARGETEND);
					if (end == endPos2)
						indentAmountPrevLine += tabWidth;
				}
			}

			_pEditView->setLineIndent(curLine, indentAmountPrevLine);
		}
		else if (ch == '}')
		{
			// Look backward for the pair {
			intptr_t startPos = _pEditView->execute(SCI_GETCURRENTPOS);
			if (startPos != 0)
				startPos -= 1;
			intptr_t posFound = findMachedBracePos(startPos - 1, 0, '{', '}');

			// if no { found, do nothing
			if (posFound == -1)
				return;

			// if { is in the same line, do nothing
			intptr_t matchedPairLine = _pEditView->execute(SCI_LINEFROMPOSITION, posFound);
			if (matchedPairLine == curLine)
				return;

			// { is in another line, get its indentation
			indentAmountPrevLine = _pEditView->getLineIndent(matchedPairLine);

			// aligned } indent with {
			_pEditView->setLineIndent(curLine, indentAmountPrevLine);
		}
	}
	else if (type == L_PYTHON)
	{
		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
				prevLine--;

			// Get previous line's Indent
			if (prevLine >= 0)
			{
				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			_pEditView->execute(SCI_SETSEARCHFLAGS, SCFIND_REGEXP | SCFIND_POSIX);

			auto startPos = _pEditView->execute(SCI_POSITIONFROMLINE, prevLine);
			auto endPos = _pEditView->execute(SCI_GETLINEENDPOSITION, prevLine);
			_pEditView->execute(SCI_SETTARGETRANGE, startPos, endPos);

			// colon optionally followed by only whitespace and/or start-of-comment, but NOT on a line that is already a comment
			const char colonExpr[] = ":[ \t]*(#|$)";

			auto posColon = _pEditView->execute(SCI_SEARCHINTARGET, strlen(colonExpr), reinterpret_cast<LPARAM>(colonExpr));

			// when colon found, additionally check that it is not in a comment, inside a string, etc.
			if ((posColon >= 0) && (_pEditView->execute(SCI_GETSTYLEINDEXAT, posColon) == SCE_P_OPERATOR))
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine + tabWidth);
			}
			else if (indentAmountPrevLine > 0)
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
		}
	}
	else // Basic indentation mode for other language types in advanced mode
	{
		if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
			(eolMode == SC_EOL_CR && ch == '\r'))
		{
			// Search the non-empty previous line
			while (prevLine >= 0 && _pEditView->getLineLength(prevLine) == 0)
				prevLine--;

			if (prevLine >= 0)
			{
				indentAmountPrevLine = _pEditView->getLineIndent(prevLine);
			}

			if (indentAmountPrevLine > 0)
			{
				_pEditView->setLineIndent(curLine, indentAmountPrevLine);
			}
		}
	}
}

// ============================================================================
// Change History Navigation
// ============================================================================

void Notepad_plus::clearChangesHistory(int iView)
{
	ScintillaEditView* pViewToChange = _pEditView;

	if (iView == MAIN_VIEW)
		pViewToChange = &_mainEditView;
	else if (iView == SUB_VIEW)
		pViewToChange = &_subEditView;

	intptr_t pos = pViewToChange->execute(SCI_GETCURRENTPOS);
	int chFlags = static_cast<int>(pViewToChange->execute(SCI_GETCHANGEHISTORY));

	pViewToChange->execute(SCI_EMPTYUNDOBUFFER);
	pViewToChange->execute(SCI_SETCHANGEHISTORY, SC_CHANGE_HISTORY_DISABLED);
	pViewToChange->execute(SCI_SETCHANGEHISTORY, chFlags);
	pViewToChange->execute(SCI_GOTOPOS, pos);
}

void Notepad_plus::changedHistoryGoTo(int idGoTo)
{
	int mask = (1 << SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN) |
	           (1 << SC_MARKNUM_HISTORY_SAVED) |
	           (1 << SC_MARKNUM_HISTORY_MODIFIED) |
	           (1 << SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED);

	intptr_t line = -1;
	intptr_t blockIndicator = _pEditView->getCurrentLineNumber();
	intptr_t lastLine = _pEditView->execute(SCI_GETLINECOUNT);

	if (idGoTo == IDM_SEARCH_CHANGED_NEXT)
	{
		intptr_t currentLine = blockIndicator;

		for (intptr_t i = currentLine; i < lastLine; ++i)
		{
			if (_pEditView->execute(SCI_MARKERGET, i) & mask)
			{
				if (i != blockIndicator)
				{
					line = i;
					break;
				}
				else
				{
					++blockIndicator;
				}
			}
		}

		if (line == -1)
		{
			intptr_t endRange = currentLine + 1;
			for (intptr_t i = 0; i < endRange; ++i)
			{
				if (_pEditView->execute(SCI_MARKERGET, i) & mask)
				{
					line = i;
					break;
				}
			}
		}
	}
	else
	{
		while (true)
		{
			line = _pEditView->execute(SCI_MARKERPREVIOUS, blockIndicator, mask);
			if (line == -1 || line != blockIndicator)
				break;
			else
				--blockIndicator;
		}

		if (line == -1)
		{
			line = _pEditView->execute(SCI_MARKERPREVIOUS, lastLine - 1, mask);
		}
	}

	if (line != -1)
	{
		_pEditView->execute(SCI_ENSUREVISIBLEENFORCEPOLICY, line);
		_pEditView->execute(SCI_GOTOLINE, line);
	}
}

#endif // NPP_LINUX
