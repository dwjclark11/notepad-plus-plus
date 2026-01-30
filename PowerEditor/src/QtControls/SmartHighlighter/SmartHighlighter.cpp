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
// This is a stub implementation for the Linux port.
// Full smart highlighting functionality will be implemented in a future update.

#include "ScintillaComponent/SmartHighlighter.h"
#include "ScintillaComponent/ScintillaEditView.h"
#include "Notepad_plus.h"

SmartHighlighter::SmartHighlighter(FindReplaceDlg* pFRDlg)
    : _pFRDlg(pFRDlg)
{
    // Stub implementation
}

void SmartHighlighter::highlightView(ScintillaEditView* pHighlightView, ScintillaEditView* unfocusView)
{
    Q_UNUSED(pHighlightView);
    Q_UNUSED(unfocusView);
    // Stub implementation - smart highlighting not yet available on Linux
}

void SmartHighlighter::highlightViewWithWord(ScintillaEditView* pHighlightView, const std::wstring& word2Hilite)
{
    Q_UNUSED(pHighlightView);
    Q_UNUSED(word2Hilite);
    // Stub implementation - smart highlighting not yet available on Linux
}
