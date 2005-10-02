/*=============================================================================

	Main core header

=============================================================================*/


#ifndef __CORE_H__
#define __CORE_H__


#ifndef CORE_API
#	define CORE_API PACKAGE_IMPORT
#endif


// static/dynamic build macros
#ifndef STATIC_BUILD
#	define PACKAGE_IMPORT	DLL_IMPORT
#	define PACKAGE_EXPORT	DLL_EXPORT
#else
#	define PACKAGE_IMPORT
#	define PACKAGE_EXPORT
#endif


// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// Build options
#include "Build.h"

// Platform/compiler-specific code
#ifdef _MSC_VER
#	include "VcWin32.h"
#else
#	error Unknown compiler
#endif


// guard/unguard/unguardf[Slow] macros
#if !DO_GUARD
#	undef guard
#	undef unguard
#	undef unguardf
#	define guard(func)
#	define unguard
#	define unguardf(msg)
#endif

#if DO_GUARD_SLOW
#	define guardSlow(func)		guard(func)
#	define unguardSlow			unguard
#	define unguardfSlow(msg)	unguardf(msg)
#else
#	define guardSlow(func)		{
#	define unguardSlow			}
#	define unguardfSlow(msg)	}
#endif


// profiling
inline void clock (unsigned &time)		{ time -= appCycles(); }
inline void unclock (unsigned &time)	{ time += appCycles(); }
inline void clock (int64 &time)			{ time -= appCycles64(); }
inline void unclock (int64 &time)		{ time += appCycles64(); }


#if STATS
#	define STAT(x)				x
#else
#	define STAT(x)
#endif


#ifndef NULL
#	define NULL ((void *)0)
#endif


/*-----------------------------------------------------------------------------
	Variables for current package
-----------------------------------------------------------------------------*/

#ifndef PACKAGE
#define PACKAGE			Main
#endif

#define MAX_PACKAGES	32
#define MAX_PKG_NAME	32

// package variables
#ifdef STATIC_BUILD
#	undef IMPLEMENT_PACKAGE
#	define IMPLEMENT_PACKAGE(version,build,date)	\
	namespace PACKAGE {								\
		const char PkgName[]    = STR(PACKAGE);		\
		const char PkgVersion[] = STR(PACKAGE) " version " STR(version) " build " STR(build) " (" date ")"; \
	}

namespace PACKAGE {
	extern const char PkgName[];
	extern const char PkgVersion[];
}

#define GPackage		PACKAGE.PkgName
#define GPkgVersion		PACKAGE.PkgVersion

#else // STATIC_BUILD

extern const char	GPackage[];
extern const char	PkgVersion[];

#endif // STATIC_BUILD


/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

class CMemoryChain;


//----- miscellaneous ?

#define BIG_NUMBER		0x1000000


#include "Macro.h"
#include "Strings.h"


/*-----------------------------------------------------------------------------
	Text output system
-----------------------------------------------------------------------------*/

// Color codes

#define C_BLACK			0
#define C_RED			1
#define C_GREEN			2
#define C_YELLOW		3
#define C_BLUE			4
#define C_MAGENTA		5
#define C_CYAN			6
#define C_WHITE			7

#define COLOR_ESCAPE	'^'			// may be used for quick location of color-processing code

#define S_BLACK			"^0"
#define S_RED			"^1"
#define S_GREEN			"^2"
#define S_YELLOW		"^3"
#define S_BLUE			"^4"
#define S_MAGENTA		"^5"
#define S_CYAN			"^6"
#define S_WHITE			"^7"


// Output device
class CORE_API COutputDevice
{
public:
	bool	NoColors;			// when true, color codes will be removed from output
	bool	FlushEveryTime;		// when true, Flush() will be called after every Write()
	COutputDevice ()
	:	NoColors(false), FlushEveryTime(false)
	{}
#if 0
	virtual ~COutputDevice ()
	{
		Unregister ();
	}
#endif
	virtual void Write (const char *str) = 0;
	virtual void Flush ()
	{ /* empty */ }
	virtual void Close ()		// may be used instead of destructor
	{ /* empty */ }
	void Printf (const char *fmt, ...);
	void Register ();
	void Unregister ();
};

// macros for hook/unhook output
#define HookOutput(device)				\
	COutputDevice *_OldHook = GLogHook;	\
	GLogHook = device;

#define HookOutputNull					\
	HookOutput(GNull)

#define UnhookOutput					\
	GLogHook = _OldHook;


CORE_API void appPrintf (const char *fmt, ...);
CORE_API void appWPrintf (const char *fmt, ...);


/*-----------------------------------------------------------------------------
	Error management
-----------------------------------------------------------------------------*/

class CErrorHandler
{
public:
	bool	swError;			// true when error was thrown by appError() call; will not dump CPU context
	bool	wasError;			// used for error history formatting
	TString<256>  Message;		// error message
	TString<2048> History;		// call history
	void	Reset ();
	// fields for non-fatal error
	bool	nonFatalError;		// when false, app can try to recover from error (and swError will be true)
};

CORE_API NORETURN void appFatalError (const char *fmt, ...);
#define appError	appFatalError
CORE_API NORETURN void appNonFatalError (const char *fmt, ...);


CORE_API void appUnwindPrefix (const char *fmt);		// not vararg (will display function name for unguardf only)
CORE_API NORETURN void appUnwindThrow (const char *fmt, ...);

// will return correct value even when log cannot be opened
CORE_API COutputDevice *appGetErrorLog ();
CORE_API void appDisplayError ();


/*-----------------------------------------------------------------------------
	Process control
-----------------------------------------------------------------------------*/

// Initialize core
CORE_API void appInit (/*?? const char *_cmdLine, COutputDevice *_log, CErrorHandler *_err */);
CORE_API void appExit ();

// System information
CORE_API const char *appPackage ();


/*-----------------------------------------------------------------------------
	Timing functions
-----------------------------------------------------------------------------*/

CORE_API double appSeconds ();
CORE_API double appMillisecondsf ();
CORE_API unsigned appMilliseconds ();
CORE_API const char *appTimestamp ();


/*-----------------------------------------------------------------------------
	Normal includes
-----------------------------------------------------------------------------*/

#include "DbgSymbols.h"						// MAY have some macros, so go as first as possible
#include "MemoryMgr.h"
//!! #include "Cvar.h"
#include "Commands.h"
#include "FileSystem.h"
#include "ScriptParser.h"
#include "TextContainer.h"

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

// output system
CORE_API extern COutputDevice	*GLogHook;				// hook appPrintf() output
CORE_API extern COutputDevice	*GLog;					// output via appPrintf()
CORE_API extern COutputDevice	*GNull;					// do not output
CORE_API extern int				GScreenWidth;			// number of characters per output line

// system information
CORE_API extern char			GMachineOS[];
CORE_API extern char			GMachineCPU[];
//??CORE_API extern TString<512>	GVersion;

CORE_API extern CErrorHandler	GErr;
CORE_API extern bool			GIsFatalError;
CORE_API extern bool			GIsRequestingExit;


#endif
