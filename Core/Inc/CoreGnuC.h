/*=============================================================================

	Core definitions for GCC compiler

=============================================================================*/


/*-----------------------------------------------------------------------------
	Type defines
-----------------------------------------------------------------------------*/

#if !__i386__
#error x86-only header (adaptation required)
#endif

typedef unsigned char		byte;		//?? change name to BYTE etc
//??typedef unsigned short	word;
//??typedef unsigned int	dword;
typedef unsigned long long	int64;		//?? INT64

typedef unsigned int		address_t;


// determing byte order (using std defines from Unix headers; mingw32 supports this too)
#if BYTE_ORDER == LITTLE_ENDIAN
#  undef LITTLE_ENDIAN
#  undef BIG_ENDIAN
#  define LITTLE_ENDIAN		1
#elif BYTE_ORDER == BIG_ENDIAN
#  undef LITTLE_ENDIAN
#  undef BIG_ENDIAN
#  define BIG_ENDIAN		1
#else
#  error unknown BYTE_OEDER
#endif


/*-----------------------------------------------------------------------------
	Miscellaneous
-----------------------------------------------------------------------------*/

#if __GNUC__ >= 4
// nothing exported by default
#pragma GCC visibility push(hidden)
#endif


#define DLL_IMPORT						// nothing?
#define DLL_EXPORT		__attribute__((visibility("default")))
#define NORETURN		__attribute__((noreturn))
#define PRINTF(n,m)		__attribute__((format(__printf__,n,m)))

#define GCC_PACK		__attribute__((__packed__))

#define	GET_RETADDR(firstarg)	((address_t)__builtin_return_address(0))

// link-time static assertion (linker will exit with error "undefined reference to 'assert_[name]'"
#define staticAssert(expr,name)			\
	{									\
		extern int *assert_##name;		\
		if (!(expr)) assert_##name = 0;	\
	}


/*-----------------------------------------------------------------------------
	Some math functions
-----------------------------------------------------------------------------*/

//?? asm version here (check compiled code)
inline int appRound (float f)
{
	return lrintf (f);
}

inline int appFloor (float f)
{
	return lrintf (floorf (f));
}

inline int appCeil (float f)
{
	return lrintf (ceilf (f));
}


/*-----------------------------------------------------------------------------
	Pentium RDTSC timing
-----------------------------------------------------------------------------*/

inline unsigned appCycles ()
{
	unsigned r;
	unsigned r2;			// this value is not used, but signal to GCC, that EDX will be modified too
	__asm __volatile__
	("rdtsc" : "=a" (r), "=d" (r2));
	return r;
}

inline int64 appCycles64 ()
{
	int64 r;
	__asm __volatile__
	("rdtsc" : "=A" (r));	// "A" for x86 means 64-bit value in EDX:EAX
	return r;
}

extern CORE_API double GMSecondsPerCycle;

inline float appDeltaCyclesToMsecf (unsigned timeDelta)
{
	double v = timeDelta;
	return v * GMSecondsPerCycle;
}

inline float appDeltaCyclesToMsecf (int64 &timeDelta)
{
	double v = timeDelta;
	return v * GMSecondsPerCycle;
}


/*-----------------------------------------------------------------------------
	guard/unguard macros
-----------------------------------------------------------------------------*/

#if 0
// standard C++ try/catch block

#define guard(func)						\
	{									\
		static const char __FUNC__[] = #func; \
		try {
			// guarded code here
#define unguard							\
		} catch (...) {					\
			appUnwindThrow (__FUNC__);	\
		}								\
	}

#define unguardf(msg)					\
		} catch (...) {					\
			appUnwindPrefix (__FUNC__);	\
			appUnwindThrow msg;			\
		}								\
	}

#define TRY				try
#define CATCH			catch (...)
#define	THROW_AGAIN		throw
#define THROW			throw 1

#else

// C++ exception-driven guard/unguard more-or-less works with mingw32 and cygwin, but
// does not works under Linux ...
// Here implemented guard/unguard using setjmp/longjmp functions
// WARNING: if program uses this type of guards, it should not use longjmp() and C++
// exceptions to send control outside of guard/unguard block!

#include <setjmp.h>
#define SETJMP_GUARD	1			// used to include support functions into program

class CORE_API CGuardContext
{
public:
	jmp_buf	jmp;
	const char *text;				// == NULL when TRY/CATCH used
	bool	jumped;					// disallow some infinite loops
	CGuardContext (const char *msg);
	~CGuardContext ();
	// register context and execute code
	bool CGuardContext::Exec (const char *msg);
};

CORE_API NORETURN void appThrowException ();

#define guard(func)						\
	{									\
		CGuardContext _Context(#func);	\
		if (setjmp (_Context.jmp) == 0) {
			// guarded code here
#define unguard							\
		} else {						\
			appUnwindThrow (_Context.text); \
		}								\
	}

#define unguardf(msg)					\
		} else {						\
			appUnwindPrefix (_Context.text); \
			appUnwindThrow msg;			\
		}								\
	}

#define TRY								\
	{									\
		CGuardContext _Context(NULL);	\
		if (setjmp (_Context.jmp) == 0)
			// guarded code here
#define CATCH							\
		else
			// exception handler here
#define END_CATCH						\
	}

#define THROW_AGAIN		appThrowException();
#define THROW			appThrowException();

#endif
