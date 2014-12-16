﻿/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <mapi.h>
#include <ShellAPI.h>

#ifndef __MRC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "../sys_local.h"
#include "win_local.h"
#include "../../renderer/tr_local.h"

idCVar Win32Vars_t::sys_arch("sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "");
idCVar Win32Vars_t::sys_cpustring("sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "");
idCVar Win32Vars_t::in_mouse("in_mouse", "1", CVAR_SYSTEM | CVAR_BOOL, "enable mouse input");
idCVar Win32Vars_t::win_allowAltTab("win_allowAltTab", "0", CVAR_SYSTEM | CVAR_BOOL, "allow Alt-Tab when fullscreen");
idCVar Win32Vars_t::win_notaskkeys("win_notaskkeys", "0", CVAR_SYSTEM | CVAR_INTEGER, "disable windows task keys");
idCVar Win32Vars_t::win_username("win_username", "", CVAR_SYSTEM | CVAR_INIT, "windows user name");
idCVar Win32Vars_t::win_xpos("win_xpos", "3", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "horizontal position of window");
idCVar Win32Vars_t::win_ypos("win_ypos", "22", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "vertical position of window");
idCVar Win32Vars_t::win_outputDebugString("win_outputDebugString", "0", CVAR_SYSTEM | CVAR_BOOL, "");
idCVar Win32Vars_t::win_outputEditString("win_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "");
idCVar Win32Vars_t::win_viewlog("win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "");
idCVar Win32Vars_t::win_timerUpdate("win_timerUpdate", "0", CVAR_SYSTEM | CVAR_BOOL, "allows the game to be updated while dragging the window");
idCVar Win32Vars_t::win_allowMultipleInstances("win_allowMultipleInstances", "0", CVAR_SYSTEM | CVAR_BOOL, "allow multiple instances running concurrently");

Win32Vars_t	win32;

static char				sys_cmdline[MAX_STRING_CHARS];

// not a hard limit, just what we keep track of for debugging
xthreadInfo				*g_threads[MAX_THREADS];
int						g_thread_count = 0;

static sysMemoryStats_t	exeLaunchMemoryStats;
static xthreadInfo		threadInfo;
static HANDLE			hTimer;

/*
===================
ThreadWrapper
===================
*/
static LPTHREAD_START_ROUTINE ThreadWrapper(xthread_t threadname) {
	return reinterpret_cast<LPTHREAD_START_ROUTINE>(threadname);
}

/*
==================
Sys_Createthread
==================
*/
void Sys_CreateThread(xthread_t threadname, LPVOID parms, xthreadPriority priority, xthreadInfo &info, const char *name, xthreadInfo *threads[MAX_THREADS], int *thread_count) {
	info.threadHandle = CreateThread(NULL,								// LPSECURITY_ATTRIBUTES lpsa,
		0,								// DWORD cbStack,
		ThreadWrapper(threadname),		// LPTHREAD_START_ROUTINE lpStartAddr,
		parms,							// LPVOID lpvThreadParm, (is this correct ??? according to msn it should be = numthreads)
		0,								// DWORD fdwCreate,
		&info.threadId);
	// uh oh ....
	if (!info.threadHandle) {
		idLib::common->FatalError("CreateThread() error is: %u\n", GetLastError());
	}
	// better have this set or Revelation will run very very very slow.
	if (priority == THREAD_HIGHEST) {
		SetThreadPriority(info.threadHandle, THREAD_PRIORITY_HIGHEST);	//  we better sleep enough to do this
	}
	else if (priority == THREAD_ABOVE_NORMAL) {
		SetThreadPriority(info.threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	else if (priority == THREAD_NORMAL) {
		SetThreadPriority(info.threadHandle, THREAD_PRIORITY_NORMAL);
	}
	else if (priority == THREAD_BELOW_NORMAL) {
		SetThreadPriority(info.threadHandle, THREAD_PRIORITY_BELOW_NORMAL);
	}
	else if (priority == THREAD_LOWEST) {
		SetThreadPriority(info.threadHandle, THREAD_PRIORITY_LOWEST);
	}
	else {
		// if we hit this then the programmer forgot to set a default thread priority.
		SetThreadPriority(info.threadHandle, GetThreadPriority(info.threadHandle) != THREAD_PRIORITY_ERROR_RETURN);
	}
	info.name = name;
	if (*thread_count < MAX_THREADS) {
		threads[(*thread_count)++] = &info;
	}
	else {
		common->Warning("WARNING: MAX_THREADS reached\n");
	}
}

/*
==================
Sys_DestroyThread
==================
*/
void Sys_DestroyThread(xthreadInfo &info) {
	DWORD	dwWaitResult, dwThreadCount;
	HANDLE	dwThreadHandle[MAX_THREADS];
	// no threads running so nothing to kill.
	if (!info.threadHandle) {
		return;
	}
	// get number of threads to wait for.
	for (dwThreadCount = 0; dwThreadCount < MAX_THREADS; dwThreadCount++) {
		// create an array of handles for WaitForMultipleObjects.
		dwThreadHandle[dwThreadCount] = info.threadHandle;
		// wait for the handle to be signaled.
		dwWaitResult = WaitForMultipleObjects(dwThreadCount, dwThreadHandle, TRUE, INFINITE);
		// signal handlers for WaitForMultipleObjects.
		switch (dwWaitResult) {
		case WAIT_ABANDONED_0:
			idLib::common->Printf("Mutex object was not released by the thread that owned the mutex object before the owning thread terminates...\n");
			break;
		case WAIT_OBJECT_0:
			idLib::common->Printf("The child thread state was signaled!\n");
			break;
		case WAIT_TIMEOUT:
			idLib::common->Printf("Time-out interval elapsed, and the child thread's state is nonsignaled.\n");
			break;
		case WAIT_FAILED:
			idLib::common->Printf("WaitForMultipleObjects() failed, error %u\n", GetLastError());
			ExitProcess(dwThreadCount);
		}
		// close the handle and set it back to NULL.
		if (CloseHandle(dwThreadHandle) != FALSE) {
			dwThreadHandle[dwThreadCount] = NULL;
		}
	}
}

/*
===================
Sys_SetThreadAffinity
===================
*/
static void Sys_SetThreadAffinity(bool mainthread) {
	SYSTEM_INFO		info;
	// check number of processors
	GetSystemInfo(&info);
	// single core machine so but out.
	if (info.dwNumberOfProcessors < 2) {
		return;
	}
	// set thread affinity for main thread on core 1 or 3
	if (mainthread) {
		switch (info.dwNumberOfProcessors) {
		case 1:
			SetThreadAffinityMask(GetCurrentThread(), (1 << info.dwNumberOfProcessors));
			break;
		case 3:
			SetThreadAffinityMask(GetCurrentThread(), (3 << info.dwNumberOfProcessors));
			break;
		default:
			break;
		}
	}
	else {
		// set affinity for other threads on core 2 or 4
		switch (info.dwNumberOfProcessors) {
		case 2:
			SetThreadAffinityMask(threadInfo.threadHandle, (2 << info.dwNumberOfProcessors));
			break;
		case 4:
			SetThreadAffinityMask(threadInfo.threadHandle, (4 << info.dwNumberOfProcessors));
			break;
		default:
			break;
		}
	}
}

/*
==================
Sys_GetThreadName
==================
*/
const char *Sys_GetThreadName(int *index) {
	int id = GetCurrentThreadId();
	for (int i = 0; i < g_thread_count; i++) {
		if (id == static_cast<int>(g_threads[i]->threadId)) {
			if (index) {
				*index = i;
			}
			return g_threads[i]->name;
		}
	}
	if (index) {
		*index = -1;
	}
	return "main";
}

/*
==================
Sys_EnterCriticalSection
==================
*/
void Sys_EnterCriticalSection(int index) {
	assert(index >= 0 && index < MAX_CRITICAL_SECTIONS);
	if (TryEnterCriticalSection(&win32.criticalSections[index]) == 0) {
		EnterCriticalSection(&win32.criticalSections[index]);
		Sys_DebugPrintf("busy lock '%d' in thread '%s'\n", index, Sys_GetThreadName());
	}
}

/*
==================
Sys_LeaveCriticalSection
==================
*/
void Sys_LeaveCriticalSection(int index) {
	assert(index >= 0 && index < MAX_CRITICAL_SECTIONS);
	LeaveCriticalSection(&win32.criticalSections[index]);
}

/*
==================
Sys_WaitForEvent
==================
*/
void Sys_WaitForEvent(int index) {
	assert(index == 0);
	if (!win32.backgroundDownloadSemaphore) {
		win32.backgroundDownloadSemaphore = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	WaitForSingleObject(win32.backgroundDownloadSemaphore, INFINITE);
	ResetEvent(win32.backgroundDownloadSemaphore);
}

/*
==================
Sys_TriggerEvent
==================
*/
void Sys_TriggerEvent(int index) {
	assert(index == 0);
	SetEvent(win32.backgroundDownloadSemaphore);
}

#pragma optimize( "", on )

#ifdef DEBUG

static unsigned int debug_total_alloc = 0;
static unsigned int debug_total_alloc_count = 0;
static unsigned int debug_current_alloc = 0;
static unsigned int debug_current_alloc_count = 0;
static unsigned int debug_frame_alloc = 0;
static unsigned int debug_frame_alloc_count = 0;

idCVar sys_showMallocs("sys_showMallocs", "0", CVAR_SYSTEM, "");

// _HOOK_ALLOC, _HOOK_REALLOC, _HOOK_FREE

typedef struct CrtMemBlockHeader {
	struct _CrtMemBlockHeader *pBlockHeaderNext;	// Pointer to the block allocated just before this one:
	struct _CrtMemBlockHeader *pBlockHeaderPrev;	// Pointer to the block allocated just after this one
	char		*szFileName;	// File name
	int			nLine;          // Line number
	size_t		nDataSize;		// Size of user block
	int			nBlockUse;      // Type of block
	long		lRequest;       // Allocation number
	byte		gap[4];								// Buffer just before (lower than) the user's memory:
} CrtMemBlockHeader;

#include <crtdbg.h>

/*
==================
Sys_AllocHook

called for every malloc/new/free/delete
==================
*/
int Sys_AllocHook(int nAllocType, void *pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char *szFileName, int nLine) {
	CrtMemBlockHeader	*pHead;
	byte				*temp;
	if (nBlockUse == _CRT_BLOCK) {
		return(TRUE);
	}
	// get a pointer to memory block header
	temp = (byte *)pvData;
	temp -= 32;
	pHead = (CrtMemBlockHeader *)temp;
	switch (nAllocType) {
	case	_HOOK_ALLOC:
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count++;
		debug_frame_alloc_count++;
		break;
	case	_HOOK_FREE:
		assert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);
		debug_current_alloc -= pHead->nDataSize;
		debug_current_alloc_count--;
		debug_total_alloc_count++;
		debug_frame_alloc_count++;
		break;
	case	_HOOK_REALLOC:
		assert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);
		debug_current_alloc -= pHead->nDataSize;
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count--;
		debug_frame_alloc_count++;
		break;
	}
	return(TRUE);
}

/*
==================
Sys_DebugMemory_f
==================
*/
void Sys_DebugMemory_f(void) {
	common->Printf("Total allocation %8dk in %d blocks\n", debug_total_alloc / 1024, debug_total_alloc_count);
	common->Printf("Current allocation %8dk in %d blocks\n", debug_current_alloc / 1024, debug_current_alloc_count);
}

/*
==================
Sys_MemFrame
==================
*/
void Sys_MemFrame(void) {
	if (sys_showMallocs.GetInteger()) {
		common->Printf("Frame: %8dk in %5d blocks\n", debug_frame_alloc / 1024, debug_frame_alloc_count);
	}
	debug_frame_alloc = 0;
	debug_frame_alloc_count = 0;
}

#endif

/*
==================
Sys_FlushCacheMemory

On windows, the vertex buffers are write combined, so they
don't need to be flushed from the cache
==================
*/
void Sys_FlushCacheMemory(void *base, int bytes) {
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error(const char *error, ...) {
	va_list		argptr;
	char		text[4096];
	MSG        msg;
	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);
	Conbuf_AppendText(text);
	Conbuf_AppendText("\n");
	Win_SetErrorText(text);
	Sys_ShowConsole(1, true);
	timeEndPeriod(1);
	Sys_ShutdownInput();
	GLimp_Shutdown();
	// wait for the user to quit
	while (true) {
		if (!GetMessage(&msg, NULL, 0, 0)) {
			common->Quit();
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Sys_DestroyConsole();
	exit(1);
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit(void) {
	timeEndPeriod(1);
	Sys_ShutdownInput();
	Sys_DestroyConsole();
	Sys_DestroyThread(threadInfo);
}

/*
==============
Sys_Printf
==============
*/
#define MAXPRINTMSG 4096
void Sys_Printf(const char *fmt, ...) {
	char		msg[MAXPRINTMSG];
	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';
	if (win32.win_outputDebugString.GetBool()) {
		OutputDebugString(msg);
	}
	if (win32.win_outputEditString.GetBool()) {
		Conbuf_AppendText(msg);
	}
}

/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf(const char *fmt, ...) {
	char msg[MAXPRINTMSG];
	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, argptr);
	msg[sizeof(msg) - 1] = '\0';
	va_end(argptr);
	OutputDebugString(msg);
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf(const char *fmt, va_list arg) {
	char msg[MAXPRINTMSG];
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, arg);
	msg[sizeof(msg) - 1] = '\0';
	OutputDebugString(msg);
}

/*
==============
Sys_Sleep
==============
*/
void Sys_Sleep(int msec) {
	Sleep(msec);
}

/*
==============
Sys_ShowWindow
==============
*/
void Sys_ShowWindow(bool show) {
	::ShowWindow(win32.hWnd, show ? SW_SHOW : SW_HIDE);
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir(const char *path) {
	_mkdir(path);
}

/*
=================
Sys_FileTimeStamp
=================
*/
ID_TIME_T Sys_FileTimeStamp(FILE *fp) {
	struct _stat st;
	_fstat(_fileno(fp), &st);
	return (long)st.st_mtime;
}

/*
==============
Sys_Cwd
==============
*/
const char *Sys_Cwd(void) {
	static char cwd[MAX_OSPATH];
	_getcwd(cwd, sizeof(cwd) - 1);
	cwd[MAX_OSPATH - 1] = 0;
	return cwd;
}

/*
==============
Sys_DefaultCDPath
==============
*/
const char *Sys_DefaultCDPath(void) {
	return "";
}

/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath(void) {
	return Sys_Cwd();
}

/*
==============
Sys_DefaultSavePath
==============
*/
const char *Sys_DefaultSavePath(void) {
	return cvarSystem->GetCVarString("fs_basepath");
}

/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath(void) {
	static char exe[MAX_OSPATH];
	GetModuleFileName(NULL, exe, sizeof(exe) - 1);
	return exe;
}

/*
==============
Sys_ListFiles
==============
*/
int Sys_ListFiles(const char *directory, const char *extension, idStrList &list) {
	idStr				search;
	struct _finddata_t	findinfo;
	int					findhandle;
	int					flag;
	if (!extension) {
		extension = "";
	}
	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		flag = 0;
	}
	else {
		flag = _A_SUBDIR;
	}
	sprintf(search, "%s\\*%s", directory, extension);
	// search
	list.Clear();
	findhandle = _findfirst(search, &findinfo);
	if (findhandle == -1) {
		return -1;
	}
	do {
		if (flag ^ (findinfo.attrib & _A_SUBDIR)) {
			list.Append(findinfo.name);
		}
	} while (_findnext(findhandle, &findinfo) != -1);
	_findclose(findhandle);
	return list.Num();
}

/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData(void) {
	char *data = NULL;
	char *cliptext;
	if (OpenClipboard(NULL) != 0) {
		HANDLE hClipboardData;
		if ((hClipboardData = GetClipboardData(CF_TEXT)) != 0) {
			if ((cliptext = (char *)GlobalLock(hClipboardData)) != 0) {
				data = (char *)Mem_Alloc(GlobalSize(hClipboardData) + 1);
				strcpy(data, cliptext);
				GlobalUnlock(hClipboardData);
				strtok(data, "\n\r\b");
			}
		}
		CloseClipboard();
	}
	return data;
}

/*
================
Sys_SetClipboardData
================
*/
void Sys_SetClipboardData(const char *string) {
	HGLOBAL HMem;
	char	*PMem;
	// allocate memory block
	HMem = (char *)::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(string) + 1);
	if (HMem == NULL) {
		return;
	}
	// lock allocated memory and obtain a pointer
	PMem = (char *)::GlobalLock(HMem);
	if (PMem == NULL) {
		return;
	}
	// copy text into allocated memory block
	lstrcpy(PMem, string);
	// unlock allocated memory
	::GlobalUnlock(HMem);
	// open Clipboard
	if (!OpenClipboard(0)) {
		::GlobalFree(HMem);
		return;
	}
	// remove current Clipboard contents
	EmptyClipboard();
	// supply the memory handle to the Clipboard
	SetClipboardData(CF_TEXT, HMem);
	HMem = 0;
	// close Clipboard
	CloseClipboard();
}

/*
========================================================================

DLL Loading

========================================================================
*/

/*
=====================
Sys_DLL_Load
=====================
*/
int Sys_DLL_Load(const char *dllName) {
	HINSTANCE	libHandle;
	libHandle = LoadLibrary( dllName );
	if ( libHandle ) {
		// since we can't have LoadLibrary load only from the specified path, check it did the right thing
		char loadedPath[ MAX_OSPATH ];
		GetModuleFileName( libHandle, loadedPath, sizeof( loadedPath ) - 1 );
		if ( idStr::IcmpPath( dllName, loadedPath ) ) {
			Sys_Printf( "ERROR: LoadLibrary '%s' wants to load '%s'\n", dllName, loadedPath );
			Sys_DLL_Unload( (int)libHandle );
			return 0;
		}
	}
	return (int)libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress(int dllHandle, const char *procName) {
	return GetProcAddress((HINSTANCE)dllHandle, procName);
}

/*
=====================
Sys_DLL_Unload
=====================
*/
void Sys_DLL_Unload(int dllHandle) {
	if (!dllHandle) {
		return;
	}
	if (FreeLibrary((HINSTANCE)dllHandle) == 0) {
		int lastError = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL
			);
		Sys_Error("Sys_DLL_Unload: FreeLibrary failed - %s (%d)", lpMsgBuf, lastError);
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t	eventQue[MAX_QUED_EVENTS];
int			eventHead = 0;
int			eventTail = 0;

/*
================
Sys_QueEvent

Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr) {
	sysEvent_t	*ev;
	ev = &eventQue[eventHead & MASK_QUED_EVENTS];
	if (eventHead - eventTail >= MAX_QUED_EVENTS) {
		common->Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if (ev->evPtr) {
			Mem_Free(ev->evPtr);
		}
		eventTail++;
	}
	eventHead++;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
=============
Sys_PumpEvents

This allows windows to be moved during renderbump
=============
*/
void Sys_PumpEvents(void) {
	MSG msg;
	// pump the message loop
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, NULL, 0, 0)) {
			common->Quit();
		}
		// save the msg time, because wndprocs don't have access to the timestamp
		if (win32.sysMsgTime && win32.sysMsgTime > (int)msg.time) {
			// don't ever let the event times run backwards
			common->DPrintf("Sys_PumpEvents: win32.sysMsgTime (%i) > msg.time (%i)\n", win32.sysMsgTime, msg.time);
		}
		else {
			win32.sysMsgTime = msg.time;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/*
================
Sys_GenerateEvents
================
*/
void Sys_GenerateEvents(void) {
	static int	entered = false;
	char		*s;
	if (entered) {
		return;
	}
	entered = true;
	// pump the message loop
	Sys_PumpEvents();
	// make sure mouse and joystick are only called once a frame
	IN_Frame();
	// check for console commands
	s = Sys_ConsoleInput();
	if (s) {
		char	*b;
		int		len;
		len = strlen(s) + 1;
		b = (char *)Mem_Alloc(len);
		strcpy(b, s);
		Sys_QueEvent(0, SE_CONSOLE, 0, 0, len, b);
	}
	entered = false;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents(void) {
	eventHead = eventTail = 0;
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent(void) {
	sysEvent_t	ev;
	// return if we have data
	if (eventHead > eventTail) {
		eventTail++;
		return eventQue[(eventTail - 1) & MASK_QUED_EVENTS];
	}
	// return the empty event
	memset(&ev, 0, sizeof(ev));
	return ev;
}

//================================================================

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f(const idCmdArgs &args) {
	Sys_ShutdownInput();
	Sys_InitInput();
}

/*
==================
Sys_AsyncThread
==================
*/
static void Sys_AsyncThread(void *parm) {
#if _DEBUG
	int		wakeNumber;
	int		startTime;
	startTime = Sys_Milliseconds();
	wakeNumber = 0;
#endif
	while (true) {
#ifdef WIN32
		// this will trigger 60 times a second
		int r = WaitForSingleObject(hTimer, 100);
		if (r != WAIT_OBJECT_0) {
			OutputDebugString("idPacketServer::PacketServerInterrupt: bad wait return");
		}
#endif
#if _DEBUG
		wakeNumber++;
		int		msec = Sys_Milliseconds();
		int		deltaTime = msec - startTime;
		startTime = msec;
		char	str[1024];
		sprintf(str, "%i ", deltaTime);
		OutputDebugString(str);
#endif
		common->Async();
	}
}

/*
==============
Sys_StartAsyncThread

Start the thread that will call idCommon::Async()
==============
*/
void Sys_StartAsyncThread(void) {
	// create an auto-reset event that happens 60 times a second
	hTimer = CreateWaitableTimer(NULL, false, NULL);
	if (!hTimer) {
		common->Error("idPacketServer::Spawn: CreateWaitableTimer failed");
	}
	LARGE_INTEGER	t;
	t.HighPart = t.LowPart = 0;
	SetWaitableTimer(hTimer, &t, USERCMD_MSEC, NULL, NULL, TRUE);
	if (!threadInfo.threadHandle) {
		// create Async thread handle.
		Sys_CreateThread((xthread_t)Sys_AsyncThread, NULL, THREAD_ABOVE_NORMAL, threadInfo, "Async", g_threads, &g_thread_count);
	}
	else {
		// Async thread allready running.
		Sys_DestroyThread(threadInfo);
	}
	Sys_SetThreadAffinity(false);
	if (!threadInfo.threadHandle) {
		common->Error("Sys_StartAsyncThread: failed");
	}
}

/*
================
Sys_AlreadyRunning

returns true if there is a copy of D3 running already
================
*/
bool Sys_AlreadyRunning(void) {
#ifndef DEBUG
	if (!win32.win_allowMultipleInstances.GetBool()) {
		HANDLE hMutexOneInstance = ::CreateMutex(NULL, FALSE, "Revelation");
		if (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED) {
			return true;
		}
	}
#endif
	return false;
}

/*
================
Sys_Init

The cvar system must already be setup
================
*/
#define OSR2_BUILD_NUMBER 1111
#define WIN98_BUILD_NUMBER 1998

void Sys_Init(void) {
	CoInitialize(NULL);
	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod(1);
	// get WM_TIMER messages pumped every millisecond
	//	SetTimer( NULL, 0, 100, NULL );
	cmdSystem->AddCommand("in_restart", Sys_In_Restart_f, CMD_FL_SYSTEM, "restarts the input system");
#ifdef DEBUG
	cmdSystem->AddCommand("createResourceIDs", CreateResourceIDs_f, CMD_FL_TOOL, "assigns resource IDs in _resouce.h files");
#endif
	//
	// Windows user name
	//
	win32.win_username.SetString(Sys_GetCurrentUser());
	//
	// Windows version
	//
	win32.osversion.dwOSVersionInfoSize = sizeof(win32.osversion);
	if (!GetVersionEx((LPOSVERSIONINFO)&win32.osversion)) {
		Sys_Error("Couldn't get OS info");
	}
	if (win32.osversion.dwMajorVersion < 4) {
		Sys_Error(GAME_NAME " requires Windows version 4 (NT) or greater");
	}
	if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32s) {
		Sys_Error(GAME_NAME " doesn't run on Win32s");
	}
	if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		if (win32.osversion.dwMajorVersion <= 4) {
			win32.sys_arch.SetString("WinNT (NT)");
		}
		else if (win32.osversion.dwMajorVersion == 5 && win32.osversion.dwMinorVersion == 0) {
			win32.sys_arch.SetString("Win2K (NT)");
		}
		else if (win32.osversion.dwMajorVersion == 5 && win32.osversion.dwMinorVersion == 1) {
			win32.sys_arch.SetString("WinXP (NT)");
		}
		else if (win32.osversion.dwMajorVersion == 6) {
			win32.sys_arch.SetString("Vista");
		}
		else if (win32.osversion.dwMajorVersion == 6 && win32.osversion.dwMinorVersion == 1) {
			win32.sys_arch.SetString("Windows 7");
		}
		else {
			win32.sys_arch.SetString("Unknown NT variant");
		}
	}
	else if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 0) {
			// Win95
			if (win32.osversion.szCSDVersion[1] == 'C') {
				win32.sys_arch.SetString("Win95 OSR2 (95)");
			}
			else {
				win32.sys_arch.SetString("Win95 (95)");
			}
		}
		else if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 10) {
			// Win98
			if (win32.osversion.szCSDVersion[1] == 'A') {
				win32.sys_arch.SetString("Win98SE (95)");
			}
			else {
				win32.sys_arch.SetString("Win98 (95)");
			}
		}
		else if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 90) {
			// WinMe
			win32.sys_arch.SetString("WinMe (95)");
		}
		else {
			win32.sys_arch.SetString("Unknown 95 variant");
		}
	}
	else {
		win32.sys_arch.SetString("unknown Windows variant");
	}
	//
	// CPU type
	//
	if (!idStr::Icmp(win32.sys_cpustring.GetString(), "detect")) {
		idStr string;
		common->Printf("%1.0f MHz ", Sys_ClockTicksPerSecond() / 1000000.0f);
		win32.cpuid = Sys_GetCPUId();
		string.Clear();
		if (win32.cpuid & CPUID_AMD) {
			string += "AMD CPU";
		}
		else if (win32.cpuid & CPUID_INTEL) {
			string += "Intel CPU";
		}
		else if (win32.cpuid & CPUID_UNSUPPORTED) {
			string += "unsupported CPU";
		}
		else {
			string += "generic CPU";
		}
		string += " with ";
		if (win32.cpuid & CPUID_MMX) {
			string += "MMX & ";
		}
		if (win32.cpuid & CPUID_3DNOW) {
			string += "3DNow! & ";
		}
		if (win32.cpuid & CPUID_SSE) {
			string += "SSE & ";
		}
		if (win32.cpuid & CPUID_SSE2) {
			string += "SSE2 & ";
		}
		if (win32.cpuid & CPUID_SSE3) {
			string += "SSE3 & ";
		}
		if (win32.cpuid & CPUID_HTT) {
			string += "HTT & ";
		}
		string.StripTrailing(" & ");
		string.StripTrailing(" with ");
		win32.sys_cpustring.SetString(string);
	}
	else {
		common->Printf("forcing CPU type to ");
		idLexer src(win32.sys_cpustring.GetString(), idStr::Length(win32.sys_cpustring.GetString()), "sys_cpustring");
		idToken token;
		int id = CPUID_NONE;
		while (src.ReadToken(&token)) {
			if (token.Icmp("generic") == 0) {
				id |= CPUID_GENERIC;
			}
			else if (token.Icmp("intel") == 0) {
				id |= CPUID_INTEL;
			}
			else if (token.Icmp("amd") == 0) {
				id |= CPUID_AMD;
			}
			else if (token.Icmp("mmx") == 0) {
				id |= CPUID_MMX;
			}
			else if (token.Icmp("3dnow") == 0) {
				id |= CPUID_3DNOW;
			}
			else if (token.Icmp("sse") == 0) {
				id |= CPUID_SSE;
			}
			else if (token.Icmp("sse2") == 0) {
				id |= CPUID_SSE2;
			}
			else if (token.Icmp("sse3") == 0) {
				id |= CPUID_SSE3;
			}
			else if (token.Icmp("htt") == 0) {
				id |= CPUID_HTT;
			}
		}
		if (id == CPUID_NONE) {
			common->Printf("WARNING: unknown sys_cpustring '%s'\n", win32.sys_cpustring.GetString());
			id = CPUID_GENERIC;
		}
		win32.cpuid = (cpuid_t)id;
	}
	common->Printf("%s\n", win32.sys_cpustring.GetString());
	common->Printf("%d MB System Memory\n", Sys_GetSystemRam());
	common->Printf("%d MB Video Memory\n", Sys_GetVideoRam());
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown(void) {
	CoUninitialize();
}

/*
================
Sys_GetProcessorId
================
*/
cpuid_t Sys_GetProcessorId(void) {
	return win32.cpuid;
}

/*
================
Sys_GetProcessorString
================
*/
const char *Sys_GetProcessorString(void) {
	return win32.sys_cpustring.GetString();
}

//=======================================================================

/*
====================
Win_Frame
====================
*/
void Win_Frame(void) {
	// if "viewlog" has been modified, show or hide the log console
	if (win32.win_viewlog.IsModified()) {
		if (!com_skipRenderer.GetBool() && idAsyncNetwork::serverDedicated.GetInteger() != 1) {
			Sys_ShowConsole(win32.win_viewlog.GetInteger(), false);
		}
		win32.win_viewlog.ClearModified();
	}
}

/*
==================
WinMain
==================
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	const HCURSOR	hcurSave = ::SetCursor(LoadCursor(0, IDC_WAIT));
	Sys_SetPhysicalWorkMemory(192 << 20, 1024 << 20);
	Sys_GetCurrentMemoryStatus(exeLaunchMemoryStats);
	win32.hInstance = hInstance;
	idStr::Copynz(sys_cmdline, lpCmdLine, sizeof(sys_cmdline));
	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();
	// no abort/retry/fail errors
	SetErrorMode(SEM_FAILCRITICALERRORS);
	for (int i = 0; i < MAX_CRITICAL_SECTIONS; i++) {
		InitializeCriticalSection(&win32.criticalSections[i]);
	}
	// get the initial time base
	Sys_Milliseconds();
#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag(0);
#endif
	common->Init(0, NULL, lpCmdLine);
#ifndef	ID_DEDICATED
	if (win32.win_notaskkeys.GetInteger()) {
		DisableTaskKeys(TRUE, FALSE, FALSE);
	}
#endif
	Sys_StartAsyncThread();
	// hide or show the early console as necessary
	if (win32.win_viewlog.GetInteger() || com_skipRenderer.GetBool() || idAsyncNetwork::serverDedicated.GetInteger()) {
		Sys_ShowConsole(1, true);
	}
	else {
		Sys_ShowConsole(0, false);
	}
	// give the main thread an affinity for the first or 3'rd cpu
	Sys_SetThreadAffinity(true);
	::SetCursor(hcurSave);
	::SetFocus(win32.hWnd);
	// main game loop
	while (true) {
		Win_Frame();
#ifdef DEBUG
		Sys_MemFrame();
#endif
		// run the game
		common->Frame();
	}
	// never gets here
	return 0;
}

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL(const char *url, bool doexit) {
	static bool doexit_spamguard = false;
	HWND wnd;
	if (doexit_spamguard) {
		common->DPrintf("OpenURL: already in an exit sequence, ignoring %s\n", url);
		return;
	}
	common->Printf("Open URL: %s\n", url);
	if (!ShellExecute(NULL, "open", url, NULL, NULL, SW_RESTORE)) {
		common->Error("Could not open url: '%s' ", url);
		return;
	}
	wnd = GetForegroundWindow();
	if (wnd) {
		ShowWindow(wnd, SW_MAXIMIZE);
	}
	if (doexit) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess(const char *exePath, bool doexit) {
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	strncpy(szPathOrig, exePath, _MAX_PATH);
	if (!CreateProcess(NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		common->Error("Could not start process: '%s' ", szPathOrig);
		return;
	}
	if (doexit) {
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
	}
}

/*
==================
Sys_SetFatalError
==================
*/
void Sys_SetFatalError(const char *error) {
}

/*
==================
Sys_DoPreferences
==================
*/
void Sys_DoPreferences(void) {
}