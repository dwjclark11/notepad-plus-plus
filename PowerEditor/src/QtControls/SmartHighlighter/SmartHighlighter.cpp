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
// Qt/Linux Implementation of SmartHighlighter
// ============================================================================

#include "ScintillaComponent/SmartHighlighter.h"
#include "ScintillaComponent/ScintillaEditView.h"
#include "Notepad_plus.h"

#define MAXLINEHIGHLIGHT 400

SmartHighlighter::SmartHighlighter(FindReplaceDlg* pFRDlg)
	: _pFRDlg(pFRDlg)
{
}

void SmartHighlighter::highlightViewWithWord(ScintillaEditView* pHighlightView, const std::wstring& word2Hilite)
{
	// Save target locations for other search functions
	auto originalStartPos = pHighlightView->execute(SCI_GETTARGETSTART);
	auto originalEndPos = pHighlightView->execute(SCI_GETTARGETEND);

	// Get the range of text visible and highlight everything in it
	intptr_t firstLine = pHighlightView->execute(SCI_GETFIRSTVISIBLELINE);
	intptr_t nbLineOnScreen = pHighlightView->execute(SCI_LINESONSCREEN);
	intptr_t nbLines = std::min<intptr_t>(nbLineOnScreen, MAXLINEHIGHLIGHT) + 1;
	intptr_t lastLine = firstLine + nbLines;
	auto currentLine = firstLine;
	intptr_t prevDocLineChecked = -1;

	// Determine mode for SmartHighlighting
	bool isWordOnly = true;
	bool isCaseSensitive = true;

	const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();

	if (nppGUI._smartHiliteUseFindSettings)
	{
		NppParameters& nppParams = NppParameters::getInstance();
		const FindHistory& findHistory = nppParams.getFindHistory();
		isWordOnly = findHistory._isMatchWord;
		isCaseSensitive = findHistory._isMatchCase;
	}
	else
	{
		isWordOnly = nppGUI._smartHiliteWordOnly;
		isCaseSensitive = nppGUI._smartHiliteCaseSensitive;
	}

	int searchFlags = (isCaseSensitive ? SCFIND_MATCHCASE : 0) | (isWordOnly ? SCFIND_WHOLEWORD : 0);

	// Convert wchar_t search text to UTF-8 for Scintilla
	WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
	size_t cp = pHighlightView->execute(SCI_GETCODEPAGE);
	const char* text2FindA = wmc.wchar2char(word2Hilite.c_str(), cp);
	size_t text2FindLen = strlen(text2FindA);

	if (text2FindLen == 0)
	{
		pHighlightView->execute(SCI_SETTARGETRANGE, originalStartPos, originalEndPos);
		return;
	}

	pHighlightView->execute(SCI_SETSEARCHFLAGS, searchFlags);

	for (; currentLine < lastLine; ++currentLine)
	{
		intptr_t docLine = pHighlightView->execute(SCI_DOCLINEFROMVISIBLE, currentLine);
		if (docLine == prevDocLineChecked)
			continue;
		prevDocLineChecked = static_cast<intptr_t>(docLine);
		size_t startPos = pHighlightView->execute(SCI_POSITIONFROMLINE, docLine);
		intptr_t endPos = pHighlightView->execute(SCI_POSITIONFROMLINE, docLine + 1);

		if (endPos == -1)
			endPos = pHighlightView->getCurrentDocLen();

		// Search within this line range
		size_t searchStart = startPos;
		while (static_cast<intptr_t>(searchStart) < endPos)
		{
			pHighlightView->execute(SCI_SETTARGETRANGE, searchStart, endPos);
			intptr_t pos = pHighlightView->execute(SCI_SEARCHINTARGET, text2FindLen, reinterpret_cast<LPARAM>(text2FindA));
			if (pos < 0)
				break;

			intptr_t targetEnd = pHighlightView->execute(SCI_GETTARGETEND);
			intptr_t foundLen = targetEnd - pos;

			if (foundLen > 0)
			{
				pHighlightView->execute(SCI_SETINDICATORCURRENT, SCE_UNIVERSAL_FOUND_STYLE_SMART);
				pHighlightView->execute(SCI_INDICATORFILLRANGE, pos, foundLen);
			}

			searchStart = targetEnd;
		}
	}

	// Restore the original targets to avoid conflicts with the search/replace functions
	pHighlightView->execute(SCI_SETTARGETRANGE, originalStartPos, originalEndPos);
}

void SmartHighlighter::highlightView(ScintillaEditView* pHighlightView, ScintillaEditView* unfocusView)
{
	// Clear marks
	pHighlightView->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_SMART);

	const NppGUI& nppGUI = NppParameters::getInstance().getNppGUI();

	// If nothing selected or smart highlighting disabled, don't mark anything
	if ((!nppGUI._enableSmartHilite) || (pHighlightView->execute(SCI_GETSELECTIONEMPTY) == 1))
	{
		if (nppGUI._smartHiliteOnAnotherView && unfocusView && unfocusView->isVisible()
			&& unfocusView->getCurrentBufferID() != pHighlightView->getCurrentBufferID())
		{
			unfocusView->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_SMART);
		}
		return;
	}

	auto curPos = pHighlightView->execute(SCI_GETCURRENTPOS);
	auto range = pHighlightView->getSelection();
	intptr_t textlen = range.cpMax - range.cpMin;

	// Determine mode for SmartHighlighting
	bool isWordOnly = true;

	if (nppGUI._smartHiliteUseFindSettings)
	{
		NppParameters& nppParams = NppParameters::getInstance();
		const FindHistory& findHistory = nppParams.getFindHistory();
		isWordOnly = findHistory._isMatchWord;
	}
	else
	{
		isWordOnly = nppGUI._smartHiliteWordOnly;
	}

	// Additional checks for wordOnly mode
	if (isWordOnly)
	{
		auto wordStart = pHighlightView->execute(SCI_WORDSTARTPOSITION, curPos, true);
		auto wordEnd = pHighlightView->execute(SCI_WORDENDPOSITION, wordStart, true);

		if (wordStart == wordEnd || wordStart != range.cpMin || wordEnd != range.cpMax)
			return;
	}
	else
	{
		auto line = pHighlightView->execute(SCI_LINEFROMPOSITION, curPos);
		auto lineLength = pHighlightView->execute(SCI_LINELENGTH, line);
		if (textlen > lineLength)
			return;
	}

	auto text2FindW = pHighlightView->getSelectedTextToWChar(false);
	if (text2FindW.empty())
		return;

	highlightViewWithWord(pHighlightView, text2FindW);

	if (nppGUI._smartHiliteOnAnotherView && unfocusView && unfocusView->isVisible())
	{
		// Clear the indicator only when the view is not a clone
		if (unfocusView->getCurrentBufferID() != pHighlightView->getCurrentBufferID())
		{
			unfocusView->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_SMART);
		}

		highlightViewWithWord(unfocusView, text2FindW);
	}
}
