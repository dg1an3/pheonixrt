//////////////////////////////////////////////////////////////////////
// UtilMacros.h: standard utility macros
//
// Copyright (C) 1999-2001
// $Id: UtilMacros.h,v 1.1 2007-10-29 01:43:52 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(UTILMACROS_H)
#define UTILMACROS_H

//////////////////////////////////////////////////////////////////////
// Macros for attributes
//////////////////////////////////////////////////////////////////////

#define DECLARE_ATTRIBUTE_VAL(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	TYPE Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(TYPE value)		\
	{ m_##NAME = value; }

#define DECLARE_ATTRIBUTE(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; }

#define DeclareMember(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; }

#define DECLARE_ATTRIBUTE_EVT(NAME, TYPE, EVT) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; EVT.Fire(this); }


#define DECLARE_ATTRIBUTE_GI(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value);

#define DeclareMemberGI(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value);


#define DECLARE_ATTRIBUTE_PTR(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue)			\
	{ m_p##NAME = pValue; }


#define DeclareMemberPtr(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue)			\
	{ m_p##NAME = pValue; }


#define DECLARE_ATTRIBUTE_PTR_GI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);


#define DeclareMemberPtrGI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);


#define BeginNamespace(NAME) namespace NAME {

#define EndNamespace(NAME) }

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

#define SerializeValue(ar, value) \
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



#define CHECK_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	if (FAILED(hr))			\
	{						\
		return hr;			\
	}						\
}

// check COM result
#define CR(Cmd) \
{						\
	HRESULT hr = (Cmd); \
	if (FAILED(hr))		\
	{					\
		TRACE("COM Command Failed -- Error %x\n", hr); \
		return hr;		\
	}					\
}

#define CHECK_CONDITION(expr) \
if (!(expr))				\
{							\
	return E_FAIL;			\
}


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

// prevent re-definition of these
#ifndef LOG_MACROS_DEFINED
#define LOG_MACROS_DEFINED

#define USES_FMT

#define LOG_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	ATLASSERT(SUCCEEDED(hr));	\
}

// log file utilities
#define BeginLogSection(section_name) { \
	LPCTSTR __section_name = section_name; \
	CString __formatMessage; \
	__formatMessage.Format(_T("<log_section name=\"%s\">"), __section_name); \
	OutputDebugString(__formatMessage.GetBuffer());

#define EndLogSection() \
	OutputDebugString(_T("</log_section>\n")); }

#define Log OutputDebugString

#endif

// profile flag to control behavior
#define PROFILE_FLAG(Section, Key) \
	static BOOL bInit = FALSE;		\
	static BOOL bShow = FALSE;		\
	if (!bInit)						\
	{								\
		bShow =						\
			(BOOL) ::AfxGetApp()->GetProfileInt(Section, Key, 0);	\
		bInit = TRUE;				\
	}								\
	if (bShow)				


#endif
