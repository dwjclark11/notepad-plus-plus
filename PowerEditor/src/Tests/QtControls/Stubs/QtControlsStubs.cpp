// QtControlsStubs.cpp - Stub implementations for the QtControlsTests executable
//
// WHY STUBS ARE NEEDED:
// Most test code includes headers that transitively pull in Parameters.h,
// which declares NppParameters -- a ~3098-line god-object singleton that
// requires essentially the entire application (Scintilla, lexilla, platform
// layer, XML parsers, etc.) to link. Stubbing out the referenced symbols
// is far cheaper than dragging in the real implementations.
//
// DEPENDENCY CHAIN (QtControlsTests):
//   RunDlg.cpp
//     -> #include "ScintillaEditView.h"   (getWordOnCaretPos)
//     -> #include "Parameters.h"           (NppParameters singleton)
//     -> #include "Notepad_plus.h"         (application class)
//   RunDlg::expandVariables() calls ScintillaEditView::getWordOnCaretPos()
//   and QtCore::Buffer::getFilePath(), which live in libraries that depend
//   on Scintilla and the full NppParameters implementation.
//
// WHAT THIS FILE STUBS:
//   - ScintillaEditView::getWordOnCaretPos  (returns empty string)
//   - QtCore::Buffer::getFilePath           (returns empty QString)
//   - NppParameters::NppParameters          (empty constructor)
//
// USED BY: QtControlsTests (via Tests/CMakeLists.txt target_sources)

#include "ScintillaEditView.h"
#include "QtCore/Buffer.h"
#include "Parameters.h"

// Stub: ScintillaEditView::getWordOnCaretPos (used by RunDlg::expandVariables)
char* ScintillaEditView::getWordOnCaretPos(char* txt, size_t size)
{
	if (txt && size > 0)
		txt[0] = '\0';
	return txt;
}

// Stub: QtCore::Buffer::getFilePath (used by RunDlg::expandVariables)
namespace QtCore {
QString Buffer::getFilePath() const
{
	return QString();
}
} // namespace QtCore

// Stub: NppParameters constructor (used by NppParameters::getInstancePointer)
NppParameters::NppParameters()
{
}
