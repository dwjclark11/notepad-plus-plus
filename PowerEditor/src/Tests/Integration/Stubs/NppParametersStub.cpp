// NppParametersStub.cpp - Stub implementations for testing FindReplaceDlg
// without pulling in the full NppParameters dependency chain

// This file provides minimal stubs for symbols required by FindReplaceDlg
// that would otherwise require linking most of the application.

#include "ScintillaEditView.h"

// Stub for ScintillaEditView::expandWordSelection - used by multiSelect* methods
// in FindReplaceDlg. Not exercised by our init/guard tests.
bool ScintillaEditView::expandWordSelection()
{
	return false;
}
