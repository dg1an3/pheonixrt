// Copyright (C) 2nd Messenger Systems
// $Id: ModelObject.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "stdafx.h"

// utility macros
#include "UtilMacros.h"

// the main include for the class
#include "ModelObject.h"

namespace dH
{

//////////////////////////////////////////////////////////////////////
ModelObject::ModelObject()
#pragma warning(disable: 4355)
	: m_eventChange(this)
#pragma warning(default: 4355)
	// constructs a model object with the given name
{
}

}
