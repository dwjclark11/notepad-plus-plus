// NppParametersStub.cpp - Stub implementations for the IntegrationTests executable
//
// WHY STUBS ARE NEEDED:
// Most test code includes headers that transitively pull in Parameters.h,
// which declares NppParameters -- a ~3098-line god-object singleton that
// requires essentially the entire application (Scintilla, lexilla, platform
// layer, XML parsers, etc.) to link. Stubbing out the referenced symbols
// is far cheaper than dragging in the real implementations.
//
// DEPENDENCY CHAIN (IntegrationTests):
//   FindReplaceDlg.h
//     -> #include "ScintillaEditView.h"   (expandWordSelection)
//     -> ScintillaEditView.h #include "Parameters.h"  (NppParameters)
//   FindReplaceDlg uses ScintillaEditView::expandWordSelection() in its
//   multiSelect* methods. Linking the real ScintillaEditView would require
//   the Scintilla library plus the entire NppParameters dependency tree.
//
// WHAT THIS FILE STUBS:
//   - ScintillaEditView::expandWordSelection  (returns false)
//
// USED BY: IntegrationTests (via Tests/CMakeLists.txt target_sources)

#include "ScintillaEditView.h"

// Stub for ScintillaEditView::expandWordSelection - used by multiSelect* methods
// in FindReplaceDlg. Not exercised by our init/guard tests.
bool ScintillaEditView::expandWordSelection()
{
	return false;
}
