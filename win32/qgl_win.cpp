//!! make this file multi-platform (and move from /win32 to renderer directory + rename file + header)

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN		// exclude rarely-used services from windown headers
#	include <windows.h>				// need this include, because have wgl and GDI functions in gl.h
#endif

#include "../ref_gl/OpenGLDrv.h"

#ifdef _WIN32
#	include "gl_win.h"				// gl_hDC
#endif

#include "OutputDeviceFile.h"


extern bool ActiveApp;	//!!! change


namespace OpenGLDrv {


static HINSTANCE libGL;		//!! win32 (type)

qgl_t			qgl;
static qgl_t	lib;

#if !NO_GL_LOG
static COutputDeviceFile *LogFile;
#endif

#include "../ref_gl/qgl_impl.h"


/*-----------------------------------------------------------------------------
	Init (loading OpenGL library and retreiving function pointers)
-----------------------------------------------------------------------------*/

bool QGL_Init (const char *libName)
{
	if (!(libGL = LoadLibrary (libName)))	//!! win32
	{
		appWPrintf ("QGL_Init: LoadLibrary(%s) failed\n", libName);
		return false;
	}

	for (int i = 0; i < NUM_GLFUNCS; i++)
	{
		if (!(lib.funcs[i] = (dummyFunc_t) (GetProcAddress (libGL, qglNames[i]))))	//!! win32
		{
			// free library
			FreeLibrary (libGL);	//!! win32
			libGL = NULL;

			appWPrintf ("QGL_Init: inconsistent OpenGL library %s: function %s is not found\n", libName, qglNames[i]);
			return false;
		}
	}
#ifdef _WIN32
	// detect minidriver OpenGL version
	if (!gl_driver->IsDefault())			// compare with opengl32.dll
		appPrintf ("...minidriver detected\n");
	else
	{
		// replace some wgl function with equivalent GDI ones
		lib.ChoosePixelFormat = ChoosePixelFormat;
		lib.DescribePixelFormat = DescribePixelFormat;
		// lib.GetPixelFormat = GetPixelFormat;
		lib.SetPixelFormat = SetPixelFormat;
		lib.SwapBuffers = SwapBuffers;
	}
#endif

	qgl = lib;

	// allow init-time logging
	QGL_EnableLogging (gl_logFile->integer != 0);

	return true;
}


/*-----------------------------------------------------------------------------
	Shutdown (unloading library)
-----------------------------------------------------------------------------*/

void QGL_Shutdown ()
{
#if !NO_GL_LOG
	if (LogFile)
	{
		delete LogFile;
		LogFile = NULL;
	}
#endif

	if (libGL)
	{
		FreeLibrary (libGL);	//!! win32
		libGL = NULL;
	}

	memset (&qgl, 0, sizeof(qgl));
}


/*-----------------------------------------------------------------------------
	Extensions support
-----------------------------------------------------------------------------*/

static bool ExtensionNameSupported (const char *name, const char *extString)
{
	if (!extString) return false;

	int len = strlen (name);
	const char *s = extString;
	while (true)
	{
		while (s[0] == ' ') s++;				// skip spaces
		if (!s[0]) break;						// end of extension string

		if (!memcmp (s, name, len) && (s[len] == 0 || s[len] == ' ')) return true;
		while (s[0] != ' ' && s[0] != 0) s++;	// skip name
	}
	return false;
}


static bool ExtensionSupported (extInfo_t *ext, const char *extStr1, const char *extStr2)
{
	const char *s = ext->name = ext->names;		// 1st alias
	if (ExtensionNameSupported (s, extStr1) || ExtensionNameSupported (s, extStr2))
		return true;
	while (true)
	{
		s = strchr (s, 0) + 1;
		if (!s[0]) return false;	// no another aliases
		//?? change notify string
		Com_DPrintf ("%s not found - try alias %s\n", ext->names, s);
		if (ExtensionNameSupported (s, extStr1) || ExtensionNameSupported (s, extStr2))
		{
			ext->name = s;
			return true;
		}
	}
}


void QGL_InitExtensions ()
{
	int		i, j;
	extInfo_t *ext;
	const char *ext1, *ext2;

	guard(QGL_InitExtensions);

	// init cvars for controlling extensions
	for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
		if (ext->cvar) Cvar_Get (ext->cvar, "1", CVAR_ARCHIVE);

	gl_config.extensionMask = 0;
	unsigned notFoundExt = 0;
	gl_config.disabledExt = gl_config.ignoredExt = 0;
	gl_config.extensions = ext1 = (char*)glGetString (GL_EXTENSIONS);

	ext2 = NULL;
#ifdef _WIN32
	{
		//?? from wglext.h -- later, when use this header, remove line
		typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
		PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
			(PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress ("wglGetExtensionsStringARB");
		if (wglGetExtensionsStringARB)
			ext2 = wglGetExtensionsStringARB (gl_hDC);
	}
#else
	use glXQueryExtensionsString() ?
#endif
	gl_config.extensions2 = ext2;

	for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
	{
		bool enable = false;
		if (ExtensionSupported (ext, ext1, ext2))
		{
			if (!ext->cvar || Cvar_VariableInt (ext->cvar))
				enable = true;
			else
				gl_config.disabledExt |= 1 << i;
		}
		else
			notFoundExt |= 1 << i;

		if (enable)
		{
			gl_config.extensionMask |= 1 << i;
			for (j = ext->first; j < ext->first + ext->count; j++)
			{
				dummyFunc_t func = qgl.funcs[j] = lib.funcs[j] = (dummyFunc_t) (wglGetProcAddress (qglNames[j]));	//!! win32
				if (!func)
				{
					appWPrintf ("Inconsistent extension %s: function %s is not found\n", ext->name, qglNames[j]);
					enable = false;
					break;
				}
			}
		}

		// (theoretically) can get "enable == false" in previous block
		if (!enable)
		{
			gl_config.extensionMask &= ~(1 << i);
			for (j = ext->first; j < ext->first + ext->count; j++)
				qgl.funcs[j] = NULL;
		}
	}

	/*-------------- check requirements -------------*/
	for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
	{
		if (!(gl_config.extensionMask & (1 << i)))
			continue;

		if ((gl_config.extensionMask & ext->require) != ext->require)
		{
			if (gl_config.disabledExt & ext->require)	// require disabled extension
				gl_config.disabledExt |= 1 << i;		// mark this extension as disabled too
			else
			{
				unsigned tmp = (gl_config.extensionMask ^ ext->require) & ext->require;
				// display error
				for (j = 0; j < NUM_EXTENSIONS; j++)
				{
					if ((1 << j) & tmp)
						appWPrintf ("%s required for %s\n", extInfo[j].names, ext->name);
				}
//				// disable extension
//				gl_config.ignoredExt |= 1 << i;
			}
			gl_config.extensionMask &= ~(1 << i);
		}
	}

	/*-------- choose preferred extensions ----------*/
	if (gl_config.extensionMask)
	{
		appPrintf ("...used extensions:\n");
		for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
		{
			if (!(gl_config.extensionMask & (1 << i)))
				continue;

			unsigned tmp = gl_config.extensionMask & ext->deprecate;
			if (tmp)
			{
				// display error
				for (j = 0; j < NUM_EXTENSIONS; j++)
				{
					if ((1 << j) & tmp)
						Com_DPrintf ("   %s ignored in favor of %s\n", ext->name, extInfo[j].name);
				}
				// disable extension
				gl_config.extensionMask &= ~(1 << i);
				gl_config.ignoredExt |= 1 << i;
			}
			else
				appPrintf ("   %s\n", ext->name);
		}
	}
	else
		appWPrintf ("...no extensions was found\n");

	/*---------- notify disabled extensions ---------*/
	if (gl_config.disabledExt)
	{
		appPrintf (S_CYAN"...disabled extensions:\n");
		for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
			if (gl_config.disabledExt & (1 << i))
				appPrintf (S_CYAN"   %s\n", ext->name);
	}

	/*----------- notify extension absence ----------*/
	if (notFoundExt)
	{
		appPrintf ("...undetected extensions:\n");
		for (i = 0, ext = extInfo; i < NUM_EXTENSIONS; i++, ext++)
		{
			if (!(notFoundExt & (1 << i))) continue;
			unsigned tmp = gl_config.extensionMask & ext->deprecate;
			if (tmp)
			{
				// display error
				for (j = 0; j < NUM_EXTENSIONS; j++)
				{
					if ((1 << j) & tmp)
					{
						Com_DPrintf ("   %s is covered by %s\n", ext->name, extInfo[j].name);
						break;		// ignore other exteensions
					}
				}
			}
			else
				appPrintf ("   %s\n", ext->name);
		}
		appPrintf ("\n");
	}

	unguard;
}


void QGL_PrintExtensionsString (const char *label, const char *str, const char *mask)
{
	char	name[256];

	// display label
	appPrintf (S_RED"%s extensions: ", label);
	// display extension names
	int i = 0;
	while (true)
	{
		char c = *str++;
		if ((c == ' ' || c == 0) && i)
		{
			name[i] = 0;
			i = 0;
			// name[] now contains current extension name
			// check display mask
			if (mask && !appMatchWildcard (name, mask, true)) continue;
			// determine color for name display
			const char *color = NULL;
			int j;
			unsigned m;
			extInfo_t *ext;
			for (j = 0, m = 1, ext = extInfo; j < NUM_EXTENSIONS; j++, ext++, m <<= 1)
			{
				const char *s = ext->names;
				while (s[0])	// while aliases present
				{
					if (!strcmp (s, name)) break;
					s = strchr (s, 0) + 1;
				}
				if (!s[0]) continue;
				// here: current name is one of aliases of extInfo[j]
				if (gl_config.disabledExt & m)
					color = S_CYAN;			// disabled by cvar
				else if (gl_config.ignoredExt & m)
					color = S_BLUE;			// ignored in a favor of different extension
				else if (gl_config.extensionMask & m)
					color = S_GREEN;		// used
				else
					color = S_RED;			// switched off by another reason
				appPrintf ("%s%s "S_WHITE, color, name);
				break;
			}
			if (!color) appPrintf ("%s ", name);		// unsupported extension
		}
		else
		{
			if (i >= sizeof(name)-1)
			{
				appWPrintf ("Bad extension string\n");
				return;
			}
			name[i++] = c;
		}
		if (!c) break;
	}
	// EOLN
	appPrintf ("\n");
}


/*-----------------------------------------------------------------------------
	Logging OpenGL function calls
-----------------------------------------------------------------------------*/

#if !NO_GL_LOG

void QGL_EnableLogging (bool enable)
{
	if (enable)
	{
		if (!LogFile)
		{
			LogFile = new COutputDeviceFile ("gl.log");
			if (!LogFile->IsOpened ())
			{
//				delete LogFile; -- if enable this, logfile will be tried to create every frame ...
//				LogFile = NULL;
				appWPrintf ("QGL_EnableLogging: unable to create file\n");
				return;
			}
			LogFile->Printf ("\n---------------------------\n%s\n---------------------------\n", appTimestamp ());
		}

		qgl = logFuncs;
	}
	else
	{
		if (LogFile)
		{
			delete LogFile;
			LogFile = NULL;
		}

		qgl = lib;
	}
}


void QGL_LogMessage (const char *text)
{
	if (!LogFile) return;
	LogFile->Printf (text);
}

#endif


/*-----------------------------------------------------------------------------
	Miscellaneous
-----------------------------------------------------------------------------*/

void QGL_SwapBuffers ()
{
	LOG_STRING("// QGL_SwapBuffers()\n");
#ifdef _WIN32
	// swapinterval stuff
	if (gl_swapinterval->modified)
	{
		gl_swapinterval->modified = false;
		if (GL_SUPPORT(QWGL_EXT_SWAP_CONTROL))
			wglSwapIntervalEXT (gl_swapinterval->integer);
	}

	static int error = 0;
	if (wglSwapBuffers (gl_hDC))
		error = 0;
	else
	{
		if (++error > 3 && ActiveApp)	// make fatal error only when 4 errors one by one
			Com_FatalError ("wglSwapBuffers() failed");
		else
			appWPrintf ("wglSwapBuffers() failed\n");
	}
#else
	glXSwapBuffers() ?
#endif
}


} // namespace
