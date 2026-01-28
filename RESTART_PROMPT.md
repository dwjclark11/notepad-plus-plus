# Notepad++ Linux/KDE Porting - Restart Prompt

## Quick Restart Instructions

This project is porting Notepad++ to Linux/KDE Plasma while maintaining Windows compatibility.

### Current Status
- **Phase:** 1 (Foundation & Abstraction Layer)
- **Progress:** See PORTING_PROGRESS.md for detailed status
- **Git:** Fresh repository initialized, ready for commits

### Key Files
- `PORTING_PROGRESS.md` - Live progress tracker (check this first)
- `PLAN.md` - Detailed porting plan and architecture
- `RESTART_PROMPT.md` - This file

### Work Structure
The work is divided into parallel agent tasks:

1. **Platform Abstraction Layer (Phase 1)**
   - FileSystem abstraction
   - Settings abstraction
   - Process abstraction
   - FileWatcher abstraction
   - CMake build system

2. **QtControls Framework (Phase 2)**
   - Window classes
   - Dialog base classes
   - Toolbar/StatusBar/TabBar
   - Docking system

3. **Linux UI Implementation (Phase 2-3)**
   - Main window (Qt version)
   - Dialogs (Find/Replace, Preferences, etc.)
   - Panels (Function List, Project, etc.)

4. **Integration & Testing**
   - Scintilla Qt integration
   - KDE Frameworks integration
   - CI/CD for both platforms

### Before Starting Work

1. Read `PORTING_PROGRESS.md` to see what's already done
2. Check which tasks are marked IN PROGRESS or PENDING
3. Update the progress file when you start/stop work
4. Commit frequently with clear messages

### Critical Rules

1. **Windows compatibility is NEVER compromised**
   - Windows code paths remain unchanged
   - New code is additive only
   - All Windows builds must pass before any merge

2. **Progress Tracking**
   - Update PORTING_PROGRESS.md before ending session
   - Mark tasks as IN_PROGRESS when starting
   - Mark tasks as COMPLETED when done

3. **Code Organization**
   - Platform code: `PowerEditor/src/Platform/`
   - Linux UI: `PowerEditor/src/QtControls/`
   - Windows UI: `PowerEditor/src/WinControls/` (unchanged)

### Agent Launch Template

To continue work, launch agents with specific tasks from PORTING_PROGRESS.md. Each agent should:
1. Read their assigned task from the progress file
2. Update status to IN_PROGRESS
3. Do the work
4. Update status to COMPLETED
5. Commit changes

Example agent tasks:
- "Create Platform/FileSystem.h interface and Platform/Windows/FileSystem.cpp wrapper"
- "Create Platform/Linux/FileSystem.cpp using std::filesystem"
- "Update CMakeLists.txt to support both platforms"

### Repository Status

```bash
# Check current git status
git status

# View recent commits
git log --oneline -10

# See what files have been modified
git diff --stat
```

### Next Immediate Actions

Based on current progress (check PORTING_PROGRESS.md), typical next steps are:
1. Create Platform/ directory structure
2. Implement first abstraction (FileSystem recommended)
3. Set up CMake for Linux builds
4. Create QtControls directory structure

### Contact/Questions

See PLAN.md for detailed architecture decisions and design patterns.
