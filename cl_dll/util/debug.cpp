#include "cl_dll.h"
#include "mathlib.h"
#include "eiface.h"

#include <stdarg.h>
#include <stdio.h>

// pm_shared is also compiled into the client but does not include the client
// no-op inline declaration from enginecallback.h.
void DEBUG_MSG( ALERT_TYPE, const char *format, ... )
{
	char message[1024];
	va_list arguments;
	va_start( arguments, format );
	vsnprintf( message, sizeof( message ), format, arguments );
	va_end( arguments );
	gEngfuncs.Con_DPrintf( "%s", message );
}
