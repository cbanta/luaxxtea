/*	
	This is an adaptation of http://en.wikipedia.org/wiki/XXTEA to lua	
	for very simple encryption.
	Lua part added by cbanta@gmail.com
	
	compile in gcc with
		gcc --shared -fPIC -O2 -o xxtea.so xxtea.c
	use in lua
		require'xxtea'
		str = 'something'
		encstr = xxtea.encrypt( str, 'abcd1234abcd1234' )
		decstr = xxtea.decrypt( encstr, 'abcd1234abcd1234' )
	where the key is a 128 bit hex string
*/

#include "lua.h"
#include "lauxlib.h"
#include <malloc.h>
#include <memory.h>

#include <stdint.h>
#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z)))

void btea(uint32_t *v, int n, uint32_t const k[4]) {
	uint32_t y, z, sum;
	unsigned p, rounds, e;
	if (n > 1) {          /* Coding Part */
		rounds = 8 + 52/n;
		sum = 0;
		z = v[n-1];
		do {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p=0; p<n-1; p++) {
				y = v[p+1]; 
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n-1] += MX;
		} while (--rounds);
	} else if (n < -1) {  /* Decoding Part */
		n = -n;
		rounds = 8 + 52/n;
		sum = rounds*DELTA;
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p=n-1; p>0; p--) {
				z = v[p-1];
				y = v[p] -= MX;
			}
			z = v[n-1];
			y = v[0] -= MX;
		} while ((sum -= DELTA) != 0);
	}
}

void getkey( lua_State *L, const unsigned int pos, uint32_t *k ){
	unsigned char buf[5];
	int i,j;
	size_t keyLength;
	const unsigned char *key = luaL_checklstring( L, pos, &keyLength );
	buf[4] = 0;
	for( i=0,j=0; i<(keyLength-3), j<4; i+=4,j++ ){
		buf[0] = key[i];
		buf[1] = key[i+1];
		buf[2] = key[i+2];
		buf[3] = key[i+3];
		k[j] = strtoul( buf, NULL, 16 );
	}
}

inline size_t align( size_t n, size_t a ){
	if( (n & (a-1)) == 0 ){
		return n;
	}else{
		return n + a - (n & (a-1));
	}
}

static int encrypt( lua_State *L ){
	uint32_t k[4];
	unsigned char *buf;
	size_t l;

	size_t textLength;
	const unsigned char *text = luaL_checklstring( L, 1, &textLength );
	getkey( L, 2, k );

	l = align(textLength + sizeof(textLength), 4);
	buf = malloc( l );
	memcpy( buf, (unsigned char *)&textLength, sizeof(textLength) );
	memcpy( &buf[sizeof(textLength)], text, textLength );

	btea( (uint32_t *)(buf+sizeof(textLength)), (l-sizeof(textLength))/sizeof(uint32_t), k );

	lua_pushlstring( L, buf, l );
	free( buf );
	return 1;
}

static int decrypt( lua_State *L ){
	uint32_t k[4];
	unsigned char *buf;
	size_t l, offset;
	size_t retLength;

	size_t textLength;
	const unsigned char *text = luaL_checklstring( L, 1, &textLength );
	getkey( L, 2, k );
	
	l = align(textLength, 4);
	buf = malloc( l );
	memcpy( buf, text, textLength );
	
	btea( (uint32_t *)(buf+sizeof(retLength)), -((textLength-sizeof(retLength))/sizeof(uint32_t)), k );
	
	memcpy( (unsigned char *)&retLength, buf, sizeof(retLength) );
	if( retLength > (l - sizeof(retLength)) )  retLength = l - sizeof(retLength);

	lua_pushlstring( L, &buf[sizeof(retLength)], retLength );
	free( buf );
	return 1;
}


/* table of operations */
static const struct luaL_reg xxtea [] = {
	{"encrypt", encrypt},
	{"decrypt", decrypt},
	{NULL, NULL}
};

/* register library */
LUALIB_API int luaopen_xxtea( lua_State *L ){
  luaL_openlib( L, "xxtea", xxtea, 0 );
  return 1;
}

