// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "PreferenceSubPageTest.h"
#include "../Common/TestUtils.h"
#include "preferenceDlg.h"

#include <QWidget>
#include <memory>

using namespace QtControls;

namespace Tests {

PreferenceSubPageTest::PreferenceSubPageTest() {}

PreferenceSubPageTest::~PreferenceSubPageTest() {}

void PreferenceSubPageTest::initTestCase()
{
	QVERIFY(TestEnvironment::getInstance().init());
}

void PreferenceSubPageTest::cleanupTestCase()
{
	TestEnvironment::getInstance().cleanup();
}

// ============================================================================
// Instantiation tests - verify each sub-page constructs without crashing
// ============================================================================

void PreferenceSubPageTest::testToolbarSubDlgInstantiation()
{
	auto dlg = std::make_unique<ToolbarSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testTabbarSubDlgInstantiation()
{
	auto dlg = std::make_unique<TabbarSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testEditing2SubDlgInstantiation()
{
	auto dlg = std::make_unique<Editing2SubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testDarkModeSubDlgInstantiation()
{
	auto dlg = std::make_unique<DarkModeSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testMarginsBorderEdgeSubDlgInstantiation()
{
	auto dlg = std::make_unique<MarginsBorderEdgeSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testFileAssocSubDlgInstantiation()
{
	auto dlg = std::make_unique<FileAssocSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testIndentationSubDlgInstantiation()
{
	auto dlg = std::make_unique<IndentationSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

void PreferenceSubPageTest::testPerformanceSubDlgInstantiation()
{
	auto dlg = std::make_unique<PerformanceSubDlg>();
	QVERIFY(dlg != nullptr);
	QVERIFY(dlg->isWidgetType());
}

// ============================================================================
// Settings round-trip tests - verify loadSettings/saveSettings don't crash
// ============================================================================

void PreferenceSubPageTest::testToolbarSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<ToolbarSubDlg>();
	// Constructor already calls loadSettings() -- verify saveSettings doesn't crash
	dlg->saveSettings();
	// Reload and verify no crash
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testTabbarSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<TabbarSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testEditing2SubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<Editing2SubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testDarkModeSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<DarkModeSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testMarginsBorderEdgeSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<MarginsBorderEdgeSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testFileAssocSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<FileAssocSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testIndentationSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<IndentationSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

void PreferenceSubPageTest::testPerformanceSubDlgSettingsRoundTrip()
{
	auto dlg = std::make_unique<PerformanceSubDlg>();
	dlg->saveSettings();
	dlg->loadSettings();
	QVERIFY(true);
}

// ============================================================================
// Verify all sub-pages are proper QWidgets with correct Qt meta-object
// ============================================================================

void PreferenceSubPageTest::testAllSubPagesAreQWidgets()
{
	// Create all 8 new sub-pages and verify they are proper QWidgets
	std::unique_ptr<QWidget> pages[] = {
		std::make_unique<ToolbarSubDlg>(),
		std::make_unique<TabbarSubDlg>(),
		std::make_unique<Editing2SubDlg>(),
		std::make_unique<DarkModeSubDlg>(),
		std::make_unique<MarginsBorderEdgeSubDlg>(),
		std::make_unique<FileAssocSubDlg>(),
		std::make_unique<IndentationSubDlg>(),
		std::make_unique<PerformanceSubDlg>(),
	};

	for (const auto& page : pages)
	{
		QVERIFY(page != nullptr);
		QVERIFY(page->isWidgetType());
		// Verify the Qt meta-object system recognizes the class name
		QVERIFY(page->metaObject() != nullptr);
		QVERIFY(QString(page->metaObject()->className()).contains("SubDlg"));
	}
}

} // namespace Tests

#include "PreferenceSubPageTest.moc"
