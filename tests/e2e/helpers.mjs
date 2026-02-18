// Shared helpers for Notepad++ E2E tests using the offscreen computer-use MCP.
//
// Usage:
//   import { setup, teardown, callTool, parseText, screenshot, sleep,
//            launchNpp, assert, report } from "./helpers.mjs";

import { Client } from "@modelcontextprotocol/sdk/client/index.js";
import { StdioClientTransport } from "@modelcontextprotocol/sdk/client/stdio.js";
import { resolve, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const MCP_SERVER = resolve(__dirname, "../../tools/computer-use-mcp/dist/main.js");
const NPP_BINARY = process.env.NPP_BINARY ||
	resolve(__dirname, "../../build/notepad-plus-plus");

let client = null;
let sessionId = null;
let _passed = 0;
let _failed = 0;

// ---------------------------------------------------------------------------
// MCP lifecycle
// ---------------------------------------------------------------------------

/** Connect to the MCP server and create a virtual display session. */
export async function setup(opts = {}) {
	const width = opts.width ?? 1280;
	const height = opts.height ?? 720;

	const transport = new StdioClientTransport({
		command: "node",
		args: [MCP_SERVER],
	});
	client = new Client({ name: "npp-e2e", version: "1.0.0" });
	await client.connect(transport);

	const result = await callTool("create_session", { width, height });
	const info = parseText(result);
	sessionId = info.session_id;
	return { sessionId, display: info.display, width, height };
}

/** Destroy the virtual display session and disconnect. */
export async function teardown() {
	if (sessionId) {
		await callTool("destroy_session", { session_id: sessionId });
		sessionId = null;
	}
	if (client) {
		await client.close();
		client = null;
	}
}

// ---------------------------------------------------------------------------
// MCP helpers
// ---------------------------------------------------------------------------

export async function callTool(name, args = {}) {
	return client.callTool({ name, arguments: args });
}

export function parseText(result) {
	const c = result.content.find((c) => c.type === "text");
	return c ? JSON.parse(c.text) : null;
}

export function getScreenshotBase64(result) {
	const c = result.content.find((c) => c.type === "image");
	return c ? c.data : null;
}

export async function sleep(ms) {
	return new Promise((r) => setTimeout(r, ms));
}

// ---------------------------------------------------------------------------
// Notepad++ helpers
// ---------------------------------------------------------------------------

/**
 * Launch Notepad++ inside the virtual display, wait for it to appear,
 * and resize the window to fill the display.
 *
 * Returns { pid }.
 */
export async function launchNpp(opts = {}) {
	const extraArgs = opts.args ?? [];
	const width = opts.width ?? 1280;
	const height = opts.height ?? 720;
	const waitMs = opts.waitMs ?? 4000;

	const result = await callTool("run_in_session", {
		session_id: sessionId,
		command: NPP_BINARY,
		args: ["-m", ...extraArgs],
	});
	const info = parseText(result);

	// Wait for Qt to fully initialize
	await sleep(waitMs);

	// Resize to fill display
	await callTool("run_in_session", {
		session_id: sessionId,
		command: "wmctrl",
		args: ["-r", "Notepad++", "-e", `0,0,0,${width},${height}`],
	});
	await sleep(1000);

	return { pid: info.pid };
}

/** Take a screenshot and return the base64-encoded image data. */
export async function screenshot() {
	const result = await callTool("computer", {
		session_id: sessionId,
		action: "get_screenshot",
	});
	return getScreenshotBase64(result);
}

/** Click at the given [x, y] coordinate. */
export async function click(coordinate) {
	await callTool("computer", {
		session_id: sessionId,
		action: "left_click",
		coordinate,
	});
}

/** Press a key or key combination (e.g. "ctrl+s", "Return"). */
export async function key(text) {
	await callTool("computer", {
		session_id: sessionId,
		action: "key",
		text,
	});
}

/** Type a string of text. */
export async function type(text) {
	await callTool("computer", {
		session_id: sessionId,
		action: "type",
		text,
	});
}

/** Move cursor to [x, y]. */
export async function moveMouse(coordinate) {
	await callTool("computer", {
		session_id: sessionId,
		action: "mouse_move",
		coordinate,
	});
}

/** Find windows matching a title pattern. */
export async function findWindows(title) {
	const result = await callTool("find_windows", {
		session_id: sessionId,
		...(title ? { title } : {}),
	});
	return parseText(result);
}

// ---------------------------------------------------------------------------
// Assertions & reporting
// ---------------------------------------------------------------------------

export function assert(condition, msg) {
	if (condition) {
		_passed++;
		console.log(`  PASS: ${msg}`);
	} else {
		_failed++;
		console.error(`  FAIL: ${msg}`);
	}
}

/** Print summary and set process.exitCode if any failures. */
export function report() {
	console.log(`\n=== Results: ${_passed} passed, ${_failed} failed ===`);
	if (_failed > 0) {
		process.exitCode = 1;
	}
	return { passed: _passed, failed: _failed };
}
