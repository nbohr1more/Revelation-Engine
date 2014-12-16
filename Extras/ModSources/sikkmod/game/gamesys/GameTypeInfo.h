
#ifndef __GAMETYPEINFO_H__
#define __GAMETYPEINFO_H__

/*
===================================================================================

	This file has been generated with the Type Info Generator v1.0 (c) 2004 id Software

	59 constants
	8 enums
	9 classes/structs/unions
	0 templates
	0 max inheritance level for ''

===================================================================================
*/

typedef struct {
	const char * name;
	const char * type;
	const char * value;
} constantInfo_t;

typedef struct {
	const char * name;
	int value;
} enumValueInfo_t;

typedef struct {
	const char * typeName;
	const enumValueInfo_t * values;
} enumTypeInfo_t;

typedef struct {
	const char * type;
	const char * name;
	int offset;
	int size;
} classVariableInfo_t;

typedef struct {
	const char * typeName;
	const char * superType;
	int size;
	const classVariableInfo_t * variables;
} classTypeInfo_t;

static constantInfo_t constantInfo[] = {
	{ "int", "CPUID_NONE", "0" },
	{ "int", "CPUID_UNSUPPORTED", "1" },
	{ "int", "CPUID_GENERIC", "2" },
	{ "int", "CPUID_INTEL", "4" },
	{ "int", "CPUID_AMD", "8" },
	{ "int", "CPUID_MMX", "16" },
	{ "int", "CPUID_3DNOW", "32" },
	{ "int", "CPUID_SSE", "64" },
	{ "int", "CPUID_SSE2", "128" },
	{ "int", "CPUID_SSE3", "256" },
	{ "int", "CPUID_ALTIVEC", "512" },
	{ "int", "CPUID_HTT", "4096" },
	{ "int", "CPUID_CMOV", "8192" },
	{ "int", "CPUID_FTZ", "16384" },
	{ "int", "CPUID_DAZ", "32768" },
	{ "int", "AXIS_SIDE", "0" },
	{ "int", "AXIS_FORWARD", "1" },
	{ "int", "AXIS_UP", "2" },
	{ "int", "AXIS_ROLL", "3" },
	{ "int", "AXIS_YAW", "4" },
	{ "int", "AXIS_PITCH", "5" },
	{ "int", "MAX_JOYSTICK_AXIS", "6" },
	{ "int", "SE_NONE", "0" },
	{ "int", "SE_KEY", "1" },
	{ "int", "SE_CHAR", "2" },
	{ "int", "SE_MOUSE", "3" },
	{ "int", "SE_JOYSTICK_AXIS", "4" },
	{ "int", "SE_CONSOLE", "5" },
	{ "int", "M_ACTION1", "0" },
	{ "int", "M_ACTION2", "1" },
	{ "int", "M_ACTION3", "2" },
	{ "int", "M_ACTION4", "3" },
	{ "int", "M_ACTION5", "4" },
	{ "int", "M_ACTION6", "5" },
	{ "int", "M_ACTION7", "6" },
	{ "int", "M_ACTION8", "7" },
	{ "int", "M_DELTAX", "8" },
	{ "int", "M_DELTAY", "9" },
	{ "int", "M_DELTAZ", "10" },
	{ "int", "NA_BAD", "0" },
	{ "int", "NA_LOOPBACK", "1" },
	{ "int", "NA_BROADCAST", "2" },
	{ "int", "NA_IP", "3" },
	{ "int", "THREAD_LOWEST", "0" },
	{ "int", "THREAD_BELOW_NORMAL", "1" },
	{ "int", "THREAD_NORMAL", "2" },
	{ "int", "THREAD_ABOVE_NORMAL", "3" },
	{ "int", "THREAD_HIGHEST", "4" },
	{ "const int", "MAX_THREADS", "16" },
	{ "const int", "MAX_CRITICAL_SECTIONS", "4" },
	{ "int", "CRITICAL_SECTION_ZERO", "0" },
	{ "int", "CRITICAL_SECTION_ONE", "1" },
	{ "int", "CRITICAL_SECTION_TWO", "2" },
	{ "int", "CRITICAL_SECTION_THREE", "3" },
	{ "const int", "MAX_TRIGGER_EVENTS", "4" },
	{ "int", "TRIGGER_EVENT_ZERO", "0" },
	{ "int", "TRIGGER_EVENT_ONE", "1" },
	{ "int", "TRIGGER_EVENT_TWO", "2" },
	{ "int", "TRIGGER_EVENT_THREE", "3" },
	{ NULL, NULL, NULL }
};

static enumValueInfo_t cpuid_t_typeInfo[] = {
	{ "CPUID_NONE", 0 },
	{ "CPUID_UNSUPPORTED", 1 },
	{ "CPUID_GENERIC", 2 },
	{ "CPUID_INTEL", 4 },
	{ "CPUID_AMD", 8 },
	{ "CPUID_MMX", 16 },
	{ "CPUID_3DNOW", 32 },
	{ "CPUID_SSE", 64 },
	{ "CPUID_SSE2", 128 },
	{ "CPUID_SSE3", 256 },
	{ "CPUID_ALTIVEC", 512 },
	{ "CPUID_HTT", 4096 },
	{ "CPUID_CMOV", 8192 },
	{ "CPUID_FTZ", 16384 },
	{ "CPUID_DAZ", 32768 },
	{ NULL, 0 }
};

static enumValueInfo_t joystickAxis_t_typeInfo[] = {
	{ "AXIS_SIDE", 0 },
	{ "AXIS_FORWARD", 1 },
	{ "AXIS_UP", 2 },
	{ "AXIS_ROLL", 3 },
	{ "AXIS_YAW", 4 },
	{ "AXIS_PITCH", 5 },
	{ "MAX_JOYSTICK_AXIS", 6 },
	{ NULL, 0 }
};

static enumValueInfo_t sysEventType_t_typeInfo[] = {
	{ "SE_NONE", 0 },
	{ "SE_KEY", 1 },
	{ "SE_CHAR", 2 },
	{ "SE_MOUSE", 3 },
	{ "SE_JOYSTICK_AXIS", 4 },
	{ "SE_CONSOLE", 5 },
	{ NULL, 0 }
};

static enumValueInfo_t sys_mEvents_typeInfo[] = {
	{ "M_ACTION1", 0 },
	{ "M_ACTION2", 1 },
	{ "M_ACTION3", 2 },
	{ "M_ACTION4", 3 },
	{ "M_ACTION5", 4 },
	{ "M_ACTION6", 5 },
	{ "M_ACTION7", 6 },
	{ "M_ACTION8", 7 },
	{ "M_DELTAX", 8 },
	{ "M_DELTAY", 9 },
	{ "M_DELTAZ", 10 },
	{ NULL, 0 }
};

static enumValueInfo_t netadrtype_t_typeInfo[] = {
	{ "NA_BAD", 0 },
	{ "NA_LOOPBACK", 1 },
	{ "NA_BROADCAST", 2 },
	{ "NA_IP", 3 },
	{ NULL, 0 }
};

static enumValueInfo_t xthreadPriority_typeInfo[] = {
	{ "THREAD_LOWEST", 0 },
	{ "THREAD_BELOW_NORMAL", 1 },
	{ "THREAD_NORMAL", 2 },
	{ "THREAD_ABOVE_NORMAL", 3 },
	{ "THREAD_HIGHEST", 4 },
	{ NULL, 0 }
};

static enumValueInfo_t enum_6_typeInfo[] = {
	{ "CRITICAL_SECTION_ZERO", 0 },
	{ "CRITICAL_SECTION_ONE", 1 },
	{ "CRITICAL_SECTION_TWO", 2 },
	{ "CRITICAL_SECTION_THREE", 3 },
	{ NULL, 0 }
};

static enumValueInfo_t enum_7_typeInfo[] = {
	{ "TRIGGER_EVENT_ZERO", 0 },
	{ "TRIGGER_EVENT_ONE", 1 },
	{ "TRIGGER_EVENT_TWO", 2 },
	{ "TRIGGER_EVENT_THREE", 3 },
	{ NULL, 0 }
};

static enumTypeInfo_t enumTypeInfo[] = {
	{ "cpuid_t", cpuid_t_typeInfo },
	{ "joystickAxis_t", joystickAxis_t_typeInfo },
	{ "sysEventType_t", sysEventType_t_typeInfo },
	{ "sys_mEvents", sys_mEvents_typeInfo },
	{ "netadrtype_t", netadrtype_t_typeInfo },
	{ "xthreadPriority", xthreadPriority_typeInfo },
	{ "enum_6", enum_6_typeInfo },
	{ "enum_7", enum_7_typeInfo },
	{ NULL, NULL }
};

static classVariableInfo_t sysEvent_t_typeInfo[] = {
	{ "sysEventType_t", "evType", (int)(&((sysEvent_t *)0)->evType), sizeof( ((sysEvent_t *)0)->evType ) },
	{ "int", "evValue", (int)(&((sysEvent_t *)0)->evValue), sizeof( ((sysEvent_t *)0)->evValue ) },
	{ "int", "evValue2", (int)(&((sysEvent_t *)0)->evValue2), sizeof( ((sysEvent_t *)0)->evValue2 ) },
	{ "int", "evPtrLength", (int)(&((sysEvent_t *)0)->evPtrLength), sizeof( ((sysEvent_t *)0)->evPtrLength ) },
	{ "void *", "evPtr", (int)(&((sysEvent_t *)0)->evPtr), sizeof( ((sysEvent_t *)0)->evPtr ) },
	{ NULL, 0 }
};

static classVariableInfo_t sysMemoryStats_t_typeInfo[] = {
	{ "int", "memoryLoad", (int)(&((sysMemoryStats_t *)0)->memoryLoad), sizeof( ((sysMemoryStats_t *)0)->memoryLoad ) },
	{ "int", "totalPhysical", (int)(&((sysMemoryStats_t *)0)->totalPhysical), sizeof( ((sysMemoryStats_t *)0)->totalPhysical ) },
	{ "int", "availPhysical", (int)(&((sysMemoryStats_t *)0)->availPhysical), sizeof( ((sysMemoryStats_t *)0)->availPhysical ) },
	{ "int", "totalPageFile", (int)(&((sysMemoryStats_t *)0)->totalPageFile), sizeof( ((sysMemoryStats_t *)0)->totalPageFile ) },
	{ "int", "availPageFile", (int)(&((sysMemoryStats_t *)0)->availPageFile), sizeof( ((sysMemoryStats_t *)0)->availPageFile ) },
	{ "int", "totalVirtual", (int)(&((sysMemoryStats_t *)0)->totalVirtual), sizeof( ((sysMemoryStats_t *)0)->totalVirtual ) },
	{ "int", "availVirtual", (int)(&((sysMemoryStats_t *)0)->availVirtual), sizeof( ((sysMemoryStats_t *)0)->availVirtual ) },
	{ "int", "availExtendedVirtual", (int)(&((sysMemoryStats_t *)0)->availExtendedVirtual), sizeof( ((sysMemoryStats_t *)0)->availExtendedVirtual ) },
	{ NULL, 0 }
};

static classVariableInfo_t netadr_t_typeInfo[] = {
	{ "netadrtype_t", "type", (int)(&((netadr_t *)0)->type), sizeof( ((netadr_t *)0)->type ) },
	{ "unsigned char[4]", "ip", (int)(&((netadr_t *)0)->ip), sizeof( ((netadr_t *)0)->ip ) },
	{ "unsigned short", "port", (int)(&((netadr_t *)0)->port), sizeof( ((netadr_t *)0)->port ) },
	{ NULL, 0 }
};

static classVariableInfo_t idPort_typeInfo[] = {
	{ "int", "packetsRead", (int)(&((idPort *)0)->packetsRead), sizeof( ((idPort *)0)->packetsRead ) },
	{ "int", "bytesRead", (int)(&((idPort *)0)->bytesRead), sizeof( ((idPort *)0)->bytesRead ) },
	{ "int", "packetsWritten", (int)(&((idPort *)0)->packetsWritten), sizeof( ((idPort *)0)->packetsWritten ) },
	{ "int", "bytesWritten", (int)(&((idPort *)0)->bytesWritten), sizeof( ((idPort *)0)->bytesWritten ) },
	{ "netadr_t", "bound_to", (int)(&((idPort *)0)->bound_to), sizeof( ((idPort *)0)->bound_to ) },
	{ "int", "netSocket", (int)(&((idPort *)0)->netSocket), sizeof( ((idPort *)0)->netSocket ) },
	{ NULL, 0 }
};

static classVariableInfo_t idTCP_typeInfo[] = {
	{ "netadr_t", "address", (int)(&((idTCP *)0)->address), sizeof( ((idTCP *)0)->address ) },
	{ "int", "fd", (int)(&((idTCP *)0)->fd), sizeof( ((idTCP *)0)->fd ) },
	{ NULL, 0 }
};

static classVariableInfo_t xthreadInfo_typeInfo[] = {
	{ "const char *", "name", (int)(&((xthreadInfo *)0)->name), sizeof( ((xthreadInfo *)0)->name ) },
	{ "HANDLE", "threadHandle", (int)(&((xthreadInfo *)0)->threadHandle), sizeof( ((xthreadInfo *)0)->threadHandle ) },
	{ "unsigned long", "threadId", (int)(&((xthreadInfo *)0)->threadId), sizeof( ((xthreadInfo *)0)->threadId ) },
	{ NULL, 0 }
};

static classVariableInfo_t idSys_typeInfo[] = {
	{ NULL, 0 }
};

static classVariableInfo_t idLib_typeInfo[] = {
	{ NULL, 0 }
};

static classVariableInfo_t idException_typeInfo[] = {
	{ "char[1024]", "error", (int)(&((idException *)0)->error), sizeof( ((idException *)0)->error ) },
	{ NULL, 0 }
};

static classTypeInfo_t classTypeInfo[] = {
	{ "sysEvent_t", "", sizeof(sysEvent_t), sysEvent_t_typeInfo },
	{ "sysMemoryStats_t", "", sizeof(sysMemoryStats_t), sysMemoryStats_t_typeInfo },
	{ "netadr_t", "", sizeof(netadr_t), netadr_t_typeInfo },
	{ "idPort", "", sizeof(idPort), idPort_typeInfo },
	{ "idTCP", "", sizeof(idTCP), idTCP_typeInfo },
	{ "xthreadInfo", "", sizeof(xthreadInfo), xthreadInfo_typeInfo },
	{ "idSys", "", sizeof(idSys), idSys_typeInfo },
	{ "idLib", "", sizeof(idLib), idLib_typeInfo },
	{ "idException", "", sizeof(idException), idException_typeInfo },
	{ NULL, NULL, 0, NULL }
};

#endif /* !__GAMETYPEINFO_H__ */
