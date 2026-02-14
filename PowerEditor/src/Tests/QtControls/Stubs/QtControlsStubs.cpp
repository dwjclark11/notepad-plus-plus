// QtControlsStubs.cpp - Stub implementations for QtControlsTests
// These stubs resolve pre-existing linker errors from RunDlg.cpp
// which references symbols from NppParameters, Buffer, and ScintillaEditView.

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
