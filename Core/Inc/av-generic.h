#pragma once
#include <stdarg.h>
#include <inttypes.h>

#ifndef CTASSERT                /* Allow lint to override */
#define CTASSERT(x)             _CTASSERT((x), __LINE__)
#define _CTASSERT(x, y)         __CTASSERT(x, y)
#define __CTASSERT(x, y)        typedef char __assert ## y[(x) ? 1 : -1]
#endif

/*
 * BIT Field operations
 */
#define _FIELD_MASK(_pos, _bits) (((1 << _bits) - 1) << _pos)
#define _SET_BITS(_x, _pos, _bits, _val) \
    do {\
        _x = (_x & ~_FIELD_MASK(_pos, _bits)) | ((_val & ((1 << _bits) - 1)) << _pos); \
    } while(0)

#define _FIELD_VAL(_pos, _bits, _val) \
    (((_val) & ((1 << (_bits)) - 1)) << (_pos))

#define _GET_BITS(_x, _pos, _bits) \
    (((_x) >> (_pos)) & ((1 << (_bits)) - 1))

#define _TST_BITS(_x, _pos, _bits) \
    ((_x) & _FIELD_MASK(_pos, _bits))

#define FIELD_MASK(_field) _FIELD_MASK(_field)
#define FIELD_VAL(_field, _val) _FIELD_VAL(_field, _val)
#define FIELD_SET(_x, _field, _val) _SET_BITS(_x, _field, _val)
#define FIELD_CLR(_x, _field) _SET_BITS(_x, _field, 0)
#define FIELD_GET(_x, _field) _GET_BITS(_x, _field)
#define FIELD_TST(_x, _field) _TST_BITS(_x, _field)

static void __attribute__ ((unused)) unreferenced_vaargs(int __attribute__ ((unused)) x, ...) { }
#define UNREFERENCED_PARAMETER(P) unreferenced_vaargs(0, P)

#define MAC_IS_EMPTY(_mac_addr) ((_mac_addr)[0] == 0xFF)

#if 0
//        (mac_addr[0] == 0xFF && mac_addr[1] == 0xFF && mac_addr[2] == 0xFF
//         mac_addr[3] == 0xFF && mac_addr[4] == 0xFF && mac_addr[5] == 0xFF)
#endif

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define PRINTF_MAC_FORMAT "%02X:%02X:%02X:%02X:%02X:%02X"
#define PRINTF_MAC_VALUE(_addr)  (_addr)[0], (_addr)[1], (_addr)[2], (_addr)[3], (_addr)[4], (_addr)[5]

#define DBG_LEN 64
extern uint32_t g_dbg[DBG_LEN];
extern int g_dbg_idx;

/* DBG_TRACE_DEF must be specified once somewhere in the global context */
#define DBG_TRACE_DEF \
		uint32_t g_dbg[DBG_LEN] = {0};\
		int g_dbg_idx = 0

#define DBG_TRACE(x) \
	do {\
		g_dbg[g_dbg_idx] = x;\
		g_dbg_idx ++;\
		if (g_dbg_idx == DBG_LEN) g_dbg_idx = 0;\
		g_dbg[g_dbg_idx] = 0xDEADBEEF;\
	} while (0)

