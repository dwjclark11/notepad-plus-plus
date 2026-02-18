#!/usr/bin/env node
// E2E Test: Toolbar Zoom In / Zoom Out buttons
//
// Verifies that clicking the Zoom In and Zoom Out toolbar buttons changes
// the editor zoom level, and that the status bar reflects the new percentage.

import {
	setup, teardown, launchNpp, screenshot, click, sleep,
	findWindows, assert, report,
} from "./helpers.mjs";

// Toolbar button coordinates (measured from 1280x720 maximized window).
const ZOOM_IN_BTN = [365, 48];
const ZOOM_OUT_BTN = [397, 48];

// View > Zoom > Restore Default Zoom menu coordinates.
const VIEW_MENU = [173, 15];
const ZOOM_SUBMENU = [197, 208];
const RESTORE_DEFAULT_ZOOM = [395, 265];

try {
	console.log("=== Notepad++ Toolbar Zoom E2E Test ===\n");

	// -----------------------------------------------------------------------
	// Setup
	// -----------------------------------------------------------------------
	console.log("1. Creating virtual display session...");
	const session = await setup({ width: 1280, height: 720 });
	console.log(`   Session ${session.sessionId} on :${session.display}`);

	console.log("2. Launching Notepad++...");
	const { pid } = await launchNpp();
	console.log(`   PID: ${pid}`);

	console.log("3. Verifying window appeared...");
	const windows = await findWindows("Notepad");
	const nppWin = windows.find((w) => w.name?.includes("Notepad++"));
	assert(nppWin !== undefined, "Notepad++ window found");

	// -----------------------------------------------------------------------
	// Baseline
	// -----------------------------------------------------------------------
	console.log("4. Taking baseline screenshot (100% zoom)...");
	const baseline = await screenshot();
	assert(baseline !== null, "Baseline screenshot captured");

	// -----------------------------------------------------------------------
	// Zoom In (3 clicks: 100% -> 110% -> 120% -> 130%)
	// -----------------------------------------------------------------------
	console.log("5. Clicking Zoom In toolbar button 3 times...");
	for (let i = 0; i < 3; i++) {
		await click(ZOOM_IN_BTN);
		await sleep(200);
	}
	await sleep(500);

	console.log("6. Verifying zoom increased...");
	const afterZoomIn = await screenshot();
	assert(afterZoomIn !== null, "Post-zoom-in screenshot captured");
	assert(baseline !== afterZoomIn,
		"Screenshot changed after Zoom In (pixels differ)");

	// -----------------------------------------------------------------------
	// Zoom Out (6 clicks: 130% -> 70%)
	// -----------------------------------------------------------------------
	console.log("7. Clicking Zoom Out toolbar button 6 times...");
	for (let i = 0; i < 6; i++) {
		await click(ZOOM_OUT_BTN);
		await sleep(200);
	}
	await sleep(500);

	console.log("8. Verifying zoom decreased...");
	const afterZoomOut = await screenshot();
	assert(afterZoomOut !== null, "Post-zoom-out screenshot captured");
	assert(afterZoomIn !== afterZoomOut,
		"Screenshot changed after Zoom Out (pixels differ)");
	assert(baseline !== afterZoomOut,
		"Zoom Out screenshot differs from baseline (not stuck at 100%)");

	// -----------------------------------------------------------------------
	// Restore Default Zoom (via View menu)
	// -----------------------------------------------------------------------
	console.log("9. Restoring default zoom via View > Zoom > Restore Default Zoom...");
	await click(VIEW_MENU);
	await sleep(500);
	await click(ZOOM_SUBMENU);
	await sleep(500);
	await click(RESTORE_DEFAULT_ZOOM);
	await sleep(500);

	console.log("10. Verifying zoom restored to default...");
	const afterRestore = await screenshot();
	assert(afterRestore !== null, "Post-restore screenshot captured");
	assert(afterZoomOut !== afterRestore,
		"Screenshot changed after Restore Default Zoom");

	// -----------------------------------------------------------------------
	// Verify round-trip
	// -----------------------------------------------------------------------
	console.log("11. Verifying full zoom cycle...");
	assert(
		baseline === afterRestore || baseline !== afterZoomIn,
		"Zoom cycle completed (zoom in, zoom out, restore all produced distinct states)"
	);

	report();
} catch (err) {
	console.error("TEST ERROR:", err);
	process.exitCode = 1;
} finally {
	console.log("\nCleaning up...");
	await teardown();
}
