/*
 * This file is in the public domain.
 */

#ifndef _INTTYPES_H_
#define	_INTTYPES_H_

#ifdef __GNUC__
typedef	int __attribute__((__mode__(__DI__)))		__int64_t;
typedef	unsigned int __attribute__((__mode__(__DI__)))	__uint64_t;
#else
/* LONGLONG */
typedef	long long					__int64_t;
/* LONGLONG */
typedef	unsigned long long				__uint64_t;
#endif

typedef	__signed char	__int8_t;
typedef	unsigned char	__uint8_t;
typedef	short		__int16_t;
typedef	unsigned short	__uint16_t;
typedef	int		__int32_t;
typedef	unsigned int	__uint32_t;

typedef	int		__intptr_t;
typedef	unsigned int	__uintptr_t;


typedef	__int8_t	int8_t;
typedef	__int16_t	int16_t;
typedef	__int32_t	int32_t;
typedef	__int64_t	int64_t;

typedef	__uint8_t	uint8_t;
typedef	__uint16_t	uint16_t;
typedef	__uint32_t	uint32_t;
typedef	__uint64_t	uint64_t;

typedef	__intptr_t	intptr_t;
typedef	__uintptr_t	uintptr_t;

#endif /* !_INTTYPES_H_ */

