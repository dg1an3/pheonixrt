//////////////////////////////////////////////////////////////////////
// UtilMacros.h: standard utility macros
//
// Copyright (C) 1999-2001
// $Id: UtilMacros.h,v 1.2 2003/04/26 20:48:50 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(UTILMACROS_H)
#define UTILMACROS_H


//////////////////////////////////////////////////////////////////////
// Macros for trace dumps
//////////////////////////////////////////////////////////////////////

#define CREATE_TAB(LENGTH) \
	CString TAB; \
	while (TAB.GetLength() < LENGTH * 2) \
		TAB += "\t\t"

#define DC_TAB(DUMP_CONTEXT) \
	DUMP_CONTEXT << TAB

#define PUSH_DUMP_DEPTH(DUMP_CONTEXT) \
	int OLD_DUMP_DEPTH = DUMP_CONTEXT.GetDepth(); \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH + 1)

#define POP_DUMP_DEPTH(DUMP_CONTEXT) \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH)


//////////////////////////////////////////////////////////////////////
// Macros for serialization
//////////////////////////////////////////////////////////////////////

#define SERIALIZE_VALUE(ar, value) \
	if (ar.IsLoading())				\
	{								\
		ar >> value;				\
	}								\
	else							\
	{								\
		ar << value;				\
	}

#define SERIALIZE_ARRAY(ar, array) \
	if (ar.IsLoading())				\
	{								\
		array.RemoveAll();			\
	}								\
	array.Serialize(ar);

#endif

#define CHECK_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	if (FAILED(hr))			\
	{						\
		return hr;			\
	}						\
}

#define CHECK_CONDITION(expr) \
if (!(expr))				\
{							\
	return E_FAIL;			\
}

#define LOG_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	ATLASSERT(SUCCEEDED(hr));	\
}

