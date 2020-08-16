// CRuntime.c: C runtime forwarder

#include <DreamLifter.h>

size_t __cdecl Dlkwcslen(
	const wchar_t* str
)
{
	return wcslen(str);
}

size_t __cdecl Dlkstrlen(
	_In_z_ char const* _Str
)
{
	return strlen(_Str);
}

char* Dlkstrcpy(
	char* strDestination,
	const char* strSource
)
{
#pragma warning(disable:4996)
	return strcpy(strDestination, strSource);
#pragma warning(default:4996)
}

_Success_(return >= 0)
_Check_return_opt_
int __cdecl Dlksprintf_s(
	_Out_writes_(_BufferCount) _Always_(_Post_z_) char* const _Buffer,
	_In_                                          size_t      const _BufferCount,
	_In_z_ _Printf_format_string_                 char const* const _Format,
	...
)
{
	int ret = 0;
	va_list args;

	va_start(args, _Format);
	ret = vsprintf_s(_Buffer, _BufferCount, _Format, args);
	va_end(args);

	return ret;
}

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlksprintf(
	_Pre_notnull_ _Always_(_Post_z_) char* const _Buffer,
	_In_z_ _Printf_format_string_    char const* const _Format,
	...
)
{
	int ret = 0;
	va_list args;

#pragma warning(disable:4996)
	va_start(args, _Format);
	ret = vsprintf(_Buffer, _Format, args);
	va_end(args);
#pragma warning(default:4996)

	return ret;
}

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlk_vsnwprintf(
	_Out_writes_opt_(_BufferCount) _Post_maybez_ wchar_t* _Buffer,
	_In_                                         size_t         _BufferCount,
	_In_z_ _Printf_format_string_                wchar_t const* _Format,
	va_list        _ArgList
)
{
#pragma warning(disable:4996)
	return _vsnwprintf(_Buffer, _BufferCount, _Format, _ArgList);
#pragma warning(default:4996)
}

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlk_vsnprintf(
	_Out_writes_opt_(_BufferCount) _Post_maybez_ char* const _Buffer,
	_In_                                        size_t      const _BufferCount,
	_In_z_ _Printf_format_string_               char const* const _Format,
	va_list           _ArgList
)
{
#pragma warning(disable:4996)
	return _vsnprintf(_Buffer, _BufferCount, _Format, _ArgList);
#pragma warning(default:4996)
}
