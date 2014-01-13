
#include "TCPIP Stack/TCPIP.h"


#define SHA_256_DATA_LENGTH 16


static DWORD ROTR(BYTE n,DWORD x) { return ((x)>>(n) | ((x)<<(32-(n)))); }
#define SHR(n,x) ((x)>>(n))

#define Choice(x,y,z)   ( (z) ^ ( (x) & ( (y) ^ (z) ) ) )
#define Majority(x,y,z) ( ((x) & (y)) ^ ((z) & ((x) ^ (y))) )

#define S0(x) (ROTR(2,(x)) ^ ROTR(13,(x)) ^ ROTR(22,(x)))
#define S1(x) (ROTR(6,(x)) ^ ROTR(11,(x)) ^ ROTR(25,(x)))

#define s0(x) (ROTR(7,(x)) ^ ROTR(18,(x)) ^ SHR(3,(x)))
#define s1(x) (ROTR(17,(x)) ^ ROTR(19,(x)) ^ SHR(10,(x)))

static rom DWORD K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
	0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
	0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
	0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
	0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
	0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
	0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
	0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
	0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
	0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
	0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
	0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
	0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL,
};

static DWORD EXPAND(DWORD *W, BYTE i) { return( ( W[(i) & 15 ] += (s1(W[((i)-2) & 15]) + W[((i)-7) & 15] + s0(W[((i)-15) & 15])) ) ); }

static DWORD T1, T2;
static void ROUND(DWORD a,DWORD b,DWORD c,DWORD d,DWORD e,DWORD f,DWORD g,DWORD h,DWORD k,DWORD data) {
	T1 = h + S1(e) + Choice(e,f,g) + k + data;
	T2 = S0(a) + Majority(a,b,c);
	T2+=T1;
}


static DWORD A, B, C, D, E, F, G, H;	/* Local vars not exactly :) */


void sha256_Chunk_1(DWORD *state, DWORD *xdata) {
	BYTE i;
	DWORD rom *k;
	DWORD *d;
	DWORD data[16];

	memcpy((void *)data,(void *)xdata,64);

	state[0]=A=0x6a09e667UL; state[1]=B=0xbb67ae85UL;
	state[2]=C=0x3c6ef372UL; state[3]=D=0xa54ff53aUL;
	state[4]=E=0x510e527fUL; state[5]=F=0x9b05688cUL;
	state[6]=G=0x1f83d9abUL; state[7]=H=0x5be0cd19UL;

	// Heavy mangling
	// First 16 subrounds that act on the original data
	
	for (i=0,k=K, d=data; i < 16; i += 8, k += 8, d += 8) {
		ROUND(A, B, C, D, E, F, G, H, k[0], d[0]);	D+=T1;	H=T2;
		ROUND(H, A, B, C, D, E, F, G, k[1], d[1]);	C+=T1;	G=T2;
		ROUND(G, H, A, B, C, D, E, F, k[2], d[2]);	B+=T1;	F=T2;
		ROUND(F, G, H, A, B, C, D, E, k[3], d[3]);	A+=T1;	E=T2;
		ROUND(E, F, G, H, A, B, C, D, k[4], d[4]);	H+=T1;	D=T2;
		ROUND(D, E, F, G, H, A, B, C, k[5], d[5]);	G+=T1;	C=T2;
		ROUND(C, D, E, F, G, H, A, B, k[6], d[6]);	F+=T1;	B=T2;
		ROUND(B, C, D, E, F, G, H, A, k[7], d[7]); 	E+=T1;	A=T2;
	}

	for (; i < 64; i += 16, k += 16) {
		ROUND(A, B, C, D, E, F, G, H, k[0], EXPAND(data, 0));  		D+=T1;	H=T2;
		ROUND(H, A, B, C, D, E, F, G, k[1], EXPAND(data, 1));  		C+=T1;	G=T2;
		ROUND(G, H, A, B, C, D, E, F, k[2], EXPAND(data, 2));  		B+=T1;	F=T2;
		ROUND(F, G, H, A, B, C, D, E, k[3], EXPAND(data, 3));  		A+=T1;	E=T2;
		ROUND(E, F, G, H, A, B, C, D, k[4], EXPAND(data, 4));  		H+=T1;	D=T2;
		ROUND(D, E, F, G, H, A, B, C, k[5], EXPAND(data, 5));  		G+=T1;	C=T2;
		ROUND(C, D, E, F, G, H, A, B, k[6], EXPAND(data, 6));  		F+=T1;	B=T2;
		ROUND(B, C, D, E, F, G, H, A, k[7], EXPAND(data, 7));  		E+=T1;	A=T2;
		ROUND(A, B, C, D, E, F, G, H, k[8], EXPAND(data, 8));  		D+=T1;	H=T2;
		ROUND(H, A, B, C, D, E, F, G, k[9], EXPAND(data, 9));  		C+=T1;	G=T2;
		ROUND(G, H, A, B, C, D, E, F, k[10], EXPAND(data, 10));		B+=T1;	F=T2;
		ROUND(F, G, H, A, B, C, D, E, k[11], EXPAND(data, 11));		A+=T1;	E=T2;
		ROUND(E, F, G, H, A, B, C, D, k[12], EXPAND(data, 12));		H+=T1;	D=T2;
		ROUND(D, E, F, G, H, A, B, C, k[13], EXPAND(data, 13));		G+=T1;	C=T2;
		ROUND(C, D, E, F, G, H, A, B, k[14], EXPAND(data, 14));		F+=T1;	B=T2;
		ROUND(B, C, D, E, F, G, H, A, k[15], EXPAND(data, 15));		E+=T1;	A=T2;
	}

	/* Update state */
	state[0] += A; state[1] += B; state[2] += C; state[3] += D;
	state[4] += E; state[5] += F; state[6] += G; state[7] += H;
}

/*
data: 00000001a8b2334e9ec3de3e8d9255ad7d8e1493ccfc3606c97f098b000003a200000000b0d8c85d7b6fd228c374baad33234688d6e8bd299e39e9d6beb6f1d8e76b1ce04f6330d81a0b328700000000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000
midstate: 18dce171595c61af498ca0242fdb4a2c2dabe52e05953edeee282dd088ae475b


data: 00000001a8b2334e9ec3de3e8d9255ad7d8e1493ccfc3606c97f098b000003a200000000ce5396178099759c3eef094300616bd384a9b2182e5ab0c0bafc0e88de3b88964f6330d81a0b328700000000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000
midstate: 4b082d09c5c3bc0a01cc0fa17e02a5e857559557867878f809dd18c7d1eeceef
          f9040003aae200a0000c0d303000020f0d0535141571c6c83b01e78d90c752e0
*/