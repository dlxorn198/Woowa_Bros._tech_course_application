#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "aes.h"
#include <time.h>

#define MUL2(a) (a<<1)^(a&0x80?0x1b:0x00) // 곱셈 x
#define MUL3(a) (MUL2(a))^(a)	// 곱셈 x+1
#define MUL4(a) MUL2((MUL2(a))) // 곱셈 x^2
#define MUL8(a) MUL2((MUL2((MUL2(a))))) // 곱셈 x^3
#define MUL9(a) (MUL8(a))^(a)// 곱셈 x^3+1
#define MULB(a)	(MUL8(a))^(MUL2(a))^(a)// 곱셈 x^3+x+1
#define MULD(a)	(MUL8(a))^(MUL4(a))^(a)// 곱셈 x^3+x^2+1
#define MULE(a) (MUL8(a))^(MUL4(a))^(MUL2(a))// 곱셈 x^3+x^2+x

u8 MUL(u8 a, u8 b)
{
	u8 r = 0;
	u8 tmp = b;
	u32 i;
	for (i = 0; i < 8; i++)
	{
		if (a & 1) r ^= tmp; // a의 마지막 비트가 1이면 xor를 한다.
		tmp = MUL2(tmp); // 곱셈 x
		a >>= 1; //(a=a>>1)
	}
	return r;
} // 2차원의 필드에서 곱셈 구현
u8 inv(u8 a)
{
	u8 r = a;
	r = MUL(r, r); //r=a^2
	r = MUL(r, a); //r=a^3
	r = MUL(r, r);	//r=a^6
	r = MUL(r, a);	//r=a^7
	r = MUL(r, r);	//r=a^14
	r = MUL(r, a);	//r=a^15
	r = MUL(r, r);	//r=a^30
	r = MUL(r, a);	//r=a^31
	r = MUL(r, r);	//r=a^62
	r = MUL(r, a);	//r=a^63
	r = MUL(r, r);	//r=a^126
	r = MUL(r, a);	//r=a^127
	r = MUL(r, r);	//r=a^254
	return r;
} // 역원 구하는 함수

u8 GenSbox(u8 a)
{
	u8 r = 0;
	u8 tmp;
	tmp = inv(a);
	if (tmp & 1)r ^= 0x1f;
	if (tmp & 2)r ^= 0x3e;
	if (tmp & 4)r ^= 0x7c;
	if (tmp & 8)r ^= 0xf8;
	if (tmp & 16)r ^= 0xf1;
	if (tmp & 32)r ^= 0xe3;
	if (tmp & 64)r ^= 0xc7;
	if (tmp & 128)r ^= 0x8f;
	return r ^ 0x63;

}
u8 inversion_sbox(u8 a)
{
	u8 r = 0;
	u8 tmp;
	tmp = a ^ 0x63;

	if (tmp & 1)r ^= 0x4a;
	if (tmp & 2)r ^= 0x94;
	if (tmp & 4)r ^= 0x29;
	if (tmp & 8)r ^= 0x52;
	if (tmp & 16)r ^= 0xa4;
	if (tmp & 32)r ^= 0x49;
	if (tmp & 64)r ^= 0x92;
	if (tmp & 128)r ^= 0x25;
	return inv(r);
}

void AddRoundkey(u8 S[16], u8 RK[16])
{
	S[0] ^= RK[0];
	S[1] ^= RK[1];
	S[2] ^= RK[2];
	S[3] ^= RK[3];
	S[4] ^= RK[4];
	S[5] ^= RK[5];
	S[6] ^= RK[6];
	S[7] ^= RK[7];
	S[8] ^= RK[8];
	S[9] ^= RK[9];
	S[10] ^= RK[10];
	S[11] ^= RK[11];
	S[12] ^= RK[12];
	S[13] ^= RK[13];
	S[14] ^= RK[14];
	S[15] ^= RK[15];
}
void SubBytes(u8 S[16])
{
	S[0] = Sbox[S[0]];
	S[1] = Sbox[S[1]];
	S[2] = Sbox[S[2]];
	S[3] = Sbox[S[3]];
	S[4] = Sbox[S[4]];
	S[5] = Sbox[S[5]];
	S[6] = Sbox[S[6]];
	S[7] = Sbox[S[7]];
	S[8] = Sbox[S[8]];
	S[9] = Sbox[S[9]];
	S[10] = Sbox[S[10]];
	S[11] = Sbox[S[11]];
	S[12] = Sbox[S[12]];
	S[13] = Sbox[S[13]];
	S[14] = Sbox[S[14]];
	S[15] = Sbox[S[15]];
}
void ShiftRows(u8 S[16])
{
	u8 temp;
	temp = S[1]; S[1] = S[5]; S[5] = S[9]; S[9] = S[13]; S[13] = temp;
	temp = S[2]; S[2] = S[10]; S[10] = temp; temp = S[6]; S[6] = S[14]; S[14] = temp;
	temp = S[15]; S[15] = S[11]; S[11] = S[7]; S[7] = S[3]; S[3] = temp;

}
void MixColumns(u8 S[16])
{
	u8 temp[16];

	temp[0] = MUL2(S[0]) ^ MUL3(S[1]) ^ S[2] ^ S[3];
	temp[1] = S[0] ^ MUL2(S[1]) ^ MUL3(S[2]) ^ S[3];
	temp[2] = S[0] ^ S[1] ^ MUL2(S[2]) ^ MUL3(S[3]);
	temp[3] = MUL3(S[0]) ^ S[1] ^ S[2] ^ MUL2(S[3]);

	temp[4] = MUL2(S[4]) ^ MUL3(S[5]) ^ S[6] ^ S[7];
	temp[5] = S[4] ^ MUL2(S[5]) ^ MUL3(S[6]) ^ S[7];
	temp[6] = S[4] ^ S[5] ^ MUL2(S[6]) ^ MUL3(S[7]);
	temp[7] = MUL3(S[4]) ^ S[5] ^ S[6] ^ MUL2(S[7]);

	temp[8] = MUL2(S[8]) ^ MUL3(S[9]) ^ S[10] ^ S[11];
	temp[9] = S[8] ^ MUL2(S[9]) ^ MUL3(S[10]) ^ S[11];
	temp[10] = S[8] ^ S[9] ^ MUL2(S[10]) ^ MUL3(S[11]);
	temp[11] = MUL3(S[8]) ^ S[9] ^ S[10] ^ MUL2(S[11]);

	temp[12] = MUL2(S[12]) ^ MUL3(S[13]) ^ S[14] ^ S[15];
	temp[13] = S[12] ^ MUL2(S[13]) ^ MUL3(S[14]) ^ S[15];
	temp[14] = S[12] ^ S[13] ^ MUL2(S[14]) ^ MUL3(S[15]);
	temp[15] = MUL3(S[12]) ^ S[13] ^ S[14] ^ MUL2(S[15]);

	for (int i = 0; i < 16; i++) {
		S[i] = temp[i];
	} // 임시 배열을 사용하여 연산 속도를 향상시킴
}

void AES_ENC(u8 PT[], u8 RK[], u8 CT[], int keysize)
{
	int Nr = keysize / 32 + 6; //라운드 수
	int i;
	u8 temp[16];

	for (i = 0; i < 16; i++) {
		temp[i] = PT[i];
	}

	AddRoundkey(temp, RK);  //temp의 16byte를 RK의 첫 16byte와 xor하여 temp에 저장.

	for (i = 0; i < Nr - 1; i++)
	{
		SubBytes(temp);
		ShiftRows(temp);
		MixColumns(temp);
		AddRoundkey(temp, RK + 16 * (i + 1));
	}

	SubBytes(temp);
	ShiftRows(temp);
	AddRoundkey(temp, RK + 16 * (i + 1));

	for (i = 0; i < 16; i++)CT[i] = temp[i];
}
u32 u4byte_in(u8* x)
{
	return(x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3]; //x[0]|x[1]|x[2]|x[3]
}
void u4byte_out(u8* x, u32 y)
{
	x[0] = (y >> 24) & 0xff;
	x[1] = (y >> 16) & 0xff;
	x[2] = (y >> 8) & 0xff;
	x[3] = y & 0xff;
}
void AES_keyWordToByte(u32 w[], u8 RK[])
{
	int i;
	for (i = 0; i < 44; i++)
	{
		u4byte_out(RK + 4 * i, w[i]); //RK[4i]||RK[4i+1]||RK[4i+2]||RK[4i+3] <--w[i]
	}
}

u32 Rcons[10] = { 0x01000000,0x02000000,0x04000000,0x08000000 ,0x10000000,0x20000000,0x40000000,0x80000000,0x1b000000,0x36000000 };

#define RotWord(x) ((x<<8)|(x>>24))

u32 SubWord(u32 x)
{
		/*u8 a, b, c, d ;
		a=Sbox[(u8)(x >> 24)];
		b = Sbox[(u8)((x >> 16)&0xff)];
		c = Sbox[(u8)((x >> 8) & 0xff)];
		d= Sbox[(u8)(x & 0xff)];*/

	return ((u32)Sbox[(u8)(x >> 24)] << 24) | ((u32)Sbox[(u8)((x >> 16) & 0xff)] << 16) | ((u32)Sbox[(u8)((x >> 8) & 0xff)] << 8) | ((u32)Sbox[(u8)(x & 0xff)]);
}
//#define SubWord(x)									\
//	((u32)Sbox[(u8)(x>>24)]<<24)						\
//	|((u32)Sbox[(u8)((x>>16)&0xff)]<<16)				\
//	|((u32)Sbox[(u8)((x>>8)&0xff)]<<8)					\
//	|((u32)Sbox[(u8)(x&0xff)])							\

void RoundkeyGeneration128(u8 MK[], u8 RK[])
{
	u32 w[44]; //32bit
	u32 T; //32bit
	w[0] = u4byte_in(MK); //w[0] =MK[0]||MK[1]||MK[2]|| MK[3]
	w[1] = u4byte_in(MK + 4);
	w[2] = u4byte_in(MK + 8);
	w[3] = u4byte_in(MK + 12);

	for (int i = 0; i < 10; i++)
	{
		//T = G_func(w[4 * i + 3]);
		T = w[4 * i + 3];
		T = RotWord(T);
		T = SubWord(T);
		T ^= Rcons[i];

		w[4 * i + 4] = w[4 * i] ^ T;
		w[4 * i + 5] = w[4 * i + 1] ^ w[4 * i + 4];
		w[4 * i + 6] = w[4 * i + 2] ^ w[4 * i + 5];
		w[4 * i + 7] = w[4 * i + 3] ^ w[4 * i + 6];
	}
	AES_keyWordToByte(w, RK);
}
void AES_keySchedule(u8 MK[], u8 RK[], int keysize)
{
	if (keysize == 128)RoundkeyGeneration128(MK, RK);
	//if (keysize == 192)RoundkeyGeneration192(MK, RK);
	//if (keysize == 256)RoundkeyGeneration256(MK, RK);

}
///////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// MixColumns의 역함수 
void Inversion_Mixcolumns(u8 S[16]) {
	u8 temp[16];
	int i;
	for (i = 0; i < 16; i += 4)
	{
		temp[i] = MULE(S[i]) ^ MULB(S[i + 1]) ^ MULD(S[i + 2]) ^ MUL9(S[i + 3]);
		temp[i + 1] = MUL9(S[i]) ^ MULE(S[i + 1]) ^ MULB(S[i + 2]) ^ MULD(S[i + 3]);
		temp[i + 2] = MULD(S[i]) ^ MUL9(S[i + 1]) ^ MULE(S[i + 2]) ^ MULB(S[i + 3]);
		temp[i + 3] = MULB(S[i]) ^ MULD(S[i + 1]) ^ MUL9(S[i + 2]) ^ MULE(S[i + 3]);
	}
	for (int i = 0; i < 16; i++) {
		S[i] = temp[i];
	}
} //for문 사용으로 속도 향상
void Inversion_shiftlows(u8 S[16]) {
	u8 temp;

	temp = S[13]; S[13] = S[9]; S[9] = S[5]; S[5] = S[1]; S[1] = temp;
	temp = S[6]; S[6] = S[14]; S[14] = temp; temp = S[10]; S[10] = S[2]; S[2] = temp;
	temp = S[15]; S[15] = S[3]; S[3] = S[7]; S[7] = S[11]; S[11] = temp;

}
void INV_SubBytes(u8 S[16])
{
	S[0] = INV_Sbox[S[0]];
	S[1] = INV_Sbox[S[1]];
	S[2] = INV_Sbox[S[2]];
	S[3] = INV_Sbox[S[3]];
	S[4] = INV_Sbox[S[4]];
	S[5] = INV_Sbox[S[5]];
	S[6] = INV_Sbox[S[6]];
	S[7] = INV_Sbox[S[7]];
	S[8] = INV_Sbox[S[8]];
	S[9] = INV_Sbox[S[9]];
	S[10] = INV_Sbox[S[10]];
	S[11] = INV_Sbox[S[11]];
	S[12] = INV_Sbox[S[12]];
	S[13] = INV_Sbox[S[13]];
	S[14] = INV_Sbox[S[14]];
	S[15] = INV_Sbox[S[15]];
}
void INV_AddRoundkey(u8 S[16], u8 RK[16])
{
	S[0] ^= RK[0];
	S[1] ^= RK[1];
	S[2] ^= RK[2];
	S[3] ^= RK[3];
	S[4] ^= RK[4];
	S[5] ^= RK[5];
	S[6] ^= RK[6];
	S[7] ^= RK[7];
	S[8] ^= RK[8];
	S[9] ^= RK[9];
	S[10] ^= RK[10];
	S[11] ^= RK[11];
	S[12] ^= RK[12];
	S[13] ^= RK[13];
	S[14] ^= RK[14];
	S[15] ^= RK[15];
}
void AES_DEC(u8 KT[], u8 RK[], u8 CT[], int keysize) {
	int Nr = keysize / 32 + 6;
	int i;
	u8 temp[16];

	for (i = 0; i < 16; i++) {
		temp[i] = KT[i];
	}

	INV_AddRoundkey(temp, RK + 16 * (9 + 1));
	Inversion_shiftlows(temp);
	INV_SubBytes(temp);

	for (i = 8; i > -1; i--)
	{
		INV_AddRoundkey(temp, RK + 16 * (i + 1));
		Inversion_Mixcolumns(temp);
		Inversion_shiftlows(temp);
		INV_SubBytes(temp);
	}


	INV_AddRoundkey(temp, RK + 16 * (i + 1));

	for (i = 0; i < 16; i++)CT[i] = temp[i];
}
//////////////////////////////////////////////////////////////////////

void RoundkeyGeneration128_Optimization(u8 MK[], u32 W[])
{

	u32 T; //32bit
	W[0] = u4byte_in(MK); //w[0] =MK[0]||MK[1]||MK[2]|| MK[3]
	W[1] = u4byte_in(MK + 4);
	W[2] = u4byte_in(MK + 8);
	W[3] = u4byte_in(MK + 12);

	for (int i = 0; i < 10; i++)
	{
		//T = G_func(w[4 * i + 3]);
		T = W[4 * i + 3];
		T = RotWord(T);
		T = SubWord(T);
		T ^= Rcons[i];

		W[4 * i + 4] = W[4 * i] ^ T;
		W[4 * i + 5] = W[4 * i + 1] ^ W[4 * i + 4];
		W[4 * i + 6] = W[4 * i + 2] ^ W[4 * i + 5];
		W[4 * i + 7] = W[4 * i + 3] ^ W[4 * i + 6];
	}

}
void AES_keySchedule_Optimization(u8 MK[], u32 W[], int keysize)
{
	if (keysize == 128)RoundkeyGeneration128_Optimization(MK, W);
	//if (keysize == 192)RoundkeyGeneration192_Optimization(MK, W);
	//if (keysize == 256)RoundkeyGeneration256_Optimization(MK, W);

}
void AES_ENC_Optimization(u8 PT[], u32 W[], u8 CT[], int keysize)
{
	int Nr = keysize / 32 + 6; //라운드 수
	int i;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;
	//0 round  A.K
	s0 = u4byte_in(PT) ^ W[0];
	s1 = u4byte_in(PT + 4) ^ W[1];
	s2 = u4byte_in(PT + 8) ^ W[2];
	s3 = u4byte_in(PT + 12) ^ W[3];

	//1 round shift,sub,mix
	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[4];
	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[5];
	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[6];
	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[7];
	//2 round
	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[8];
	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[9];
	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[10];
	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[11];
	//3 round
	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[12];
	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[13];
	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[14];
	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[15];
	//4 round
	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[16];
	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[17];
	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[18];
	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[19];
	//5 round
	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[20];
	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[21];
	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[22];
	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[23];
	//6 round
	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[24];
	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[25];
	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[26];
	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[27];
	//7 round
	t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[28];
	t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[29];
	t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[30];
	t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[31];
	//8 round
	s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[32];
	s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[33];
	s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[34];
	s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[35];
	if (Nr == 10) {
		//9 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[36];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[37];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[38];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[39];
		//10 round
		s0 = (Te3[(t0 >> 24)] & 0xff000000) ^ (Te0[(t1 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t2 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t3) & 0xff] & 0x000000ff) ^ W[40];
		s1 = (Te2[(t1 >> 24)] & 0xff000000) ^ (Te3[(t2 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t3 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t0) & 0xff] & 0x000000ff) ^ W[41];
		s2 = (Te2[(t2 >> 24)] & 0xff000000) ^ (Te3[(t3 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t0 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t1) & 0xff] & 0x000000ff) ^ W[42];
		s3 = (Te2[(t3 >> 24)] & 0xff000000) ^ (Te3[(t0 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t1 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t2) & 0xff] & 0x000000ff) ^ W[43];
	}
	else if (Nr == 12) {
		//9 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[36];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[37];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[38];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[39];
		//10 round
		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[40];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[41];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[42];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[43];
		//11 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[44];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[45];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[46];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[47];
		//12 round
		s0 = (Te2[(t0 >> 24)] & 0xff000000) ^ (Te3[(t1 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t2 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t3) & 0xff] & 0x000000ff) ^ W[48];
		s1 = (Te2[(t1 >> 24)] & 0xff000000) ^ (Te3[(t2 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t3 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t0) & 0xff] & 0x000000ff) ^ W[49];
		s2 = (Te2[(t2 >> 24)] & 0xff000000) ^ (Te3[(t3 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t0 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t1) & 0xff] & 0x000000ff) ^ W[50];
		s3 = (Te2[(t3 >> 24)] & 0xff000000) ^ (Te3[(t0 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t1 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t2) & 0xff] & 0x000000ff) ^ W[51];
	}
	else if (Nr == 14) {
		//9 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[36];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[37];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[38];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[39];
		//10 round
		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[40];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[41];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[42];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[43];
		//11 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[44];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[45];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[46];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[47];
		//12 round
		s0 = Te0[t0 >> 24] ^ Te1[(t1 >> 16) & 0xff] ^ Te2[(t2 >> 8) & 0xff] ^ Te3[t3 & 0xff] ^ W[48];
		s1 = Te0[t1 >> 24] ^ Te1[(t2 >> 16) & 0xff] ^ Te2[(t3 >> 8) & 0xff] ^ Te3[t0 & 0xff] ^ W[49];
		s2 = Te0[t2 >> 24] ^ Te1[(t3 >> 16) & 0xff] ^ Te2[(t0 >> 8) & 0xff] ^ Te3[t1 & 0xff] ^ W[50];
		s3 = Te0[t3 >> 24] ^ Te1[(t0 >> 16) & 0xff] ^ Te2[(t1 >> 8) & 0xff] ^ Te3[t2 & 0xff] ^ W[51];
		//13 round
		t0 = Te0[s0 >> 24] ^ Te1[(s1 >> 16) & 0xff] ^ Te2[(s2 >> 8) & 0xff] ^ Te3[s3 & 0xff] ^ W[52];
		t1 = Te0[s1 >> 24] ^ Te1[(s2 >> 16) & 0xff] ^ Te2[(s3 >> 8) & 0xff] ^ Te3[s0 & 0xff] ^ W[53];
		t2 = Te0[s2 >> 24] ^ Te1[(s3 >> 16) & 0xff] ^ Te2[(s0 >> 8) & 0xff] ^ Te3[s1 & 0xff] ^ W[54];
		t3 = Te0[s3 >> 24] ^ Te1[(s0 >> 16) & 0xff] ^ Te2[(s1 >> 8) & 0xff] ^ Te3[s2 & 0xff] ^ W[55];
		//14 round
		s0 = (Te2[(t0 >> 24)] & 0xff000000) ^ (Te3[(t1 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t2 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t3) & 0xff] & 0x000000ff) ^ W[56];
		s1 = (Te2[(t1 >> 24)] & 0xff000000) ^ (Te3[(t2 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t3 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t0) & 0xff] & 0x000000ff) ^ W[57];
		s2 = (Te2[(t2 >> 24)] & 0xff000000) ^ (Te3[(t3 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t0 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t1) & 0xff] & 0x000000ff) ^ W[58];
		s3 = (Te2[(t3 >> 24)] & 0xff000000) ^ (Te3[(t0 >> 16) & 0xff] & 0x00ff0000) ^ (Te0[(t1 >> 8) & 0xff] & 0x0000ff00) ^ (Te1[(t2) & 0xff] & 0x000000ff) ^ W[59];
	}
	u4byte_out(CT, s0);
	u4byte_out(CT + 4, s1);
	u4byte_out(CT + 8, s2);
	u4byte_out(CT + 12, s3);
}
////////////////////////////////////////////////////////////////////////////////

// MixColumns의 역함수 최적화
void Inversion_Mixcolumns_Optimization(u32 W[], u32 W_mix[], int keysize) {
	int Nr = keysize / 32 + 6;

	//방법1 각 라운드의 mixcolumn을 따로 구한다.
	/*for (int q = 4; q <= 39; q++) {
		W_mix[q] = (((u8)(MULE((u8)(W[q] >> 24))) ^ (u8)(MULB((u8)((W[q] >> 16) & 0xff))) ^ (u8)(MULD((u8)((W[q] >> 8) & 0xff))) ^ (u8)(MUL9((u8)(W[q] & 0xff)))) << 24)
			| (((u8)(MUL9((u8)(W[q] >> 24))) ^ (u8)(MULE((u8)((W[q] >> 16) & 0xff))) ^ (u8)(MULB((u8)((W[q] >> 8) & 0xff))) ^ (u8)(MULD((u8)(W[q] & 0xff)))) << 16)
			| (((u8)(MULD((u8)(W[q] >> 24))) ^ (u8)(MUL9((u8)((W[q] >> 16) & 0xff))) ^ (u8)(MULE((u8)((W[q] >> 8) & 0xff))) ^ (u8)(MULB((u8)(W[q] & 0xff)))) << 8)
			| (((u8)(MULB((u8)(W[q] >> 24))) ^ (u8)(MULD((u8)((W[q] >> 16) & 0xff))) ^ (u8)(MUL9((u8)((W[q] >> 8) & 0xff))) ^ (u8)(MULE((u8)(W[q] & 0xff)))));
	}*/


	//방법2 INV_Te 테이블을 이용한다.
	for (int i = 1; i < Nr; i++) {
		W_mix[i * 4] = INV_Te0[Sbox[(W[i * 4] >> 24)]] ^ INV_Te1[Sbox[(W[i * 4] >> 16) & 0xff]] ^ INV_Te2[Sbox[(W[i * 4] >> 8) & 0xff]] ^ INV_Te3[Sbox[(W[i * 4]) & 0xff]];
		W_mix[i * 4 + 1] = INV_Te0[Sbox[(W[i * 4 + 1] >> 24)]] ^ INV_Te1[Sbox[(W[i * 4 + 1] >> 16) & 0xff]] ^ INV_Te2[Sbox[(W[i * 4 + 1] >> 8) & 0xff]] ^ INV_Te3[Sbox[(W[i * 4 + 1]) & 0xff]];
		W_mix[(i * 4) + 2] = INV_Te0[Sbox[(W[(i * 4) + 2] >> 24)]] ^ INV_Te1[Sbox[(W[(i * 4) + 2] >> 16) & 0xff]] ^ INV_Te2[Sbox[(W[(i * 4) + 2] >> 8) & 0xff]] ^ INV_Te3[Sbox[(W[(i * 4) + 2]) & 0xff]];
		W_mix[(i * 4) + 3] = INV_Te0[Sbox[(W[(i * 4) + 3] >> 24)]] ^ INV_Te1[Sbox[(W[(i * 4) + 3] >> 16) & 0xff]] ^ INV_Te2[Sbox[(W[(i * 4) + 3] >> 8) & 0xff]] ^ INV_Te3[Sbox[(W[(i * 4) + 3]) & 0xff]];
	} //INV_Te 테이블에 입력된 값이 INV_Sbox[8비트]=8비트 이므로 이것을 섞기 위해 Sbox[8비트]를 입력으로 준다. 그렇게 MixColumn을 대체할 수 있다.

}
void AES_DEC_Optimization(u8 KT[], u32 W[], u8 CT[], int keysize)
{
	int Nr = keysize / 32 + 6; //라운드 수
	int i;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;
	u32 W_mix[44] = { 0x00, };
	Inversion_Mixcolumns_Optimization(W, W_mix, keysize);
	//10 round
	s0 = u4byte_in(KT) ^ W[40];
	s1 = u4byte_in(KT + 4) ^ W[41];
	s2 = u4byte_in(KT + 8) ^ W[42];
	s3 = u4byte_in(KT + 12) ^ W[43];
	//9 round shift,sub,mix,mix_w
	t0 = INV_Te0[s0 >> 24] ^ INV_Te1[(s3 >> 16) & 0xff] ^ INV_Te2[(s2 >> 8) & 0xff] ^ INV_Te3[s1 & 0xff] ^ W_mix[36]; //_mix
	t1 = INV_Te0[s1 >> 24] ^ INV_Te1[(s0 >> 16) & 0xff] ^ INV_Te2[(s3 >> 8) & 0xff] ^ INV_Te3[s2 & 0xff] ^ W_mix[37];
	t2 = INV_Te0[s2 >> 24] ^ INV_Te1[(s1 >> 16) & 0xff] ^ INV_Te2[(s0 >> 8) & 0xff] ^ INV_Te3[s3 & 0xff] ^ W_mix[38];
	t3 = INV_Te0[s3 >> 24] ^ INV_Te1[(s2 >> 16) & 0xff] ^ INV_Te2[(s1 >> 8) & 0xff] ^ INV_Te3[s0 & 0xff] ^ W_mix[39];
	//8 round
	s0 = INV_Te0[t0 >> 24] ^ INV_Te1[(t3 >> 16) & 0xff] ^ INV_Te2[(t2 >> 8) & 0xff] ^ INV_Te3[t1 & 0xff] ^ W_mix[32];
	s1 = INV_Te0[t1 >> 24] ^ INV_Te1[(t0 >> 16) & 0xff] ^ INV_Te2[(t3 >> 8) & 0xff] ^ INV_Te3[t2 & 0xff] ^ W_mix[33];
	s2 = INV_Te0[t2 >> 24] ^ INV_Te1[(t1 >> 16) & 0xff] ^ INV_Te2[(t0 >> 8) & 0xff] ^ INV_Te3[t3 & 0xff] ^ W_mix[34];
	s3 = INV_Te0[t3 >> 24] ^ INV_Te1[(t2 >> 16) & 0xff] ^ INV_Te2[(t1 >> 8) & 0xff] ^ INV_Te3[t0 & 0xff] ^ W_mix[35];
	//7 round
	t0 = INV_Te0[s0 >> 24] ^ INV_Te1[(s3 >> 16) & 0xff] ^ INV_Te2[(s2 >> 8) & 0xff] ^ INV_Te3[s1 & 0xff] ^ W_mix[28];
	t1 = INV_Te0[s1 >> 24] ^ INV_Te1[(s0 >> 16) & 0xff] ^ INV_Te2[(s3 >> 8) & 0xff] ^ INV_Te3[s2 & 0xff] ^ W_mix[29];
	t2 = INV_Te0[s2 >> 24] ^ INV_Te1[(s1 >> 16) & 0xff] ^ INV_Te2[(s0 >> 8) & 0xff] ^ INV_Te3[s3 & 0xff] ^ W_mix[30];
	t3 = INV_Te0[s3 >> 24] ^ INV_Te1[(s2 >> 16) & 0xff] ^ INV_Te2[(s1 >> 8) & 0xff] ^ INV_Te3[s0 & 0xff] ^ W_mix[31];
	//6 round
	s0 = INV_Te0[t0 >> 24] ^ INV_Te1[(t3 >> 16) & 0xff] ^ INV_Te2[(t2 >> 8) & 0xff] ^ INV_Te3[t1 & 0xff] ^ W_mix[24];
	s1 = INV_Te0[t1 >> 24] ^ INV_Te1[(t0 >> 16) & 0xff] ^ INV_Te2[(t3 >> 8) & 0xff] ^ INV_Te3[t2 & 0xff] ^ W_mix[25];
	s2 = INV_Te0[t2 >> 24] ^ INV_Te1[(t1 >> 16) & 0xff] ^ INV_Te2[(t0 >> 8) & 0xff] ^ INV_Te3[t3 & 0xff] ^ W_mix[26];
	s3 = INV_Te0[t3 >> 24] ^ INV_Te1[(t2 >> 16) & 0xff] ^ INV_Te2[(t1 >> 8) & 0xff] ^ INV_Te3[t0 & 0xff] ^ W_mix[27];
	//5 round
	t0 = INV_Te0[s0 >> 24] ^ INV_Te1[(s3 >> 16) & 0xff] ^ INV_Te2[(s2 >> 8) & 0xff] ^ INV_Te3[s1 & 0xff] ^ W_mix[20];
	t1 = INV_Te0[s1 >> 24] ^ INV_Te1[(s0 >> 16) & 0xff] ^ INV_Te2[(s3 >> 8) & 0xff] ^ INV_Te3[s2 & 0xff] ^ W_mix[21];
	t2 = INV_Te0[s2 >> 24] ^ INV_Te1[(s1 >> 16) & 0xff] ^ INV_Te2[(s0 >> 8) & 0xff] ^ INV_Te3[s3 & 0xff] ^ W_mix[22];
	t3 = INV_Te0[s3 >> 24] ^ INV_Te1[(s2 >> 16) & 0xff] ^ INV_Te2[(s1 >> 8) & 0xff] ^ INV_Te3[s0 & 0xff] ^ W_mix[23];
	//4 round
	s0 = INV_Te0[t0 >> 24] ^ INV_Te1[(t3 >> 16) & 0xff] ^ INV_Te2[(t2 >> 8) & 0xff] ^ INV_Te3[t1 & 0xff] ^ W_mix[16];
	s1 = INV_Te0[t1 >> 24] ^ INV_Te1[(t0 >> 16) & 0xff] ^ INV_Te2[(t3 >> 8) & 0xff] ^ INV_Te3[t2 & 0xff] ^ W_mix[17];
	s2 = INV_Te0[t2 >> 24] ^ INV_Te1[(t1 >> 16) & 0xff] ^ INV_Te2[(t0 >> 8) & 0xff] ^ INV_Te3[t3 & 0xff] ^ W_mix[18];
	s3 = INV_Te0[t3 >> 24] ^ INV_Te1[(t2 >> 16) & 0xff] ^ INV_Te2[(t1 >> 8) & 0xff] ^ INV_Te3[t0 & 0xff] ^ W_mix[19];
	//3 round
	t0 = INV_Te0[s0 >> 24] ^ INV_Te1[(s3 >> 16) & 0xff] ^ INV_Te2[(s2 >> 8) & 0xff] ^ INV_Te3[s1 & 0xff] ^ W_mix[12];
	t1 = INV_Te0[s1 >> 24] ^ INV_Te1[(s0 >> 16) & 0xff] ^ INV_Te2[(s3 >> 8) & 0xff] ^ INV_Te3[s2 & 0xff] ^ W_mix[13];
	t2 = INV_Te0[s2 >> 24] ^ INV_Te1[(s1 >> 16) & 0xff] ^ INV_Te2[(s0 >> 8) & 0xff] ^ INV_Te3[s3 & 0xff] ^ W_mix[14];
	t3 = INV_Te0[s3 >> 24] ^ INV_Te1[(s2 >> 16) & 0xff] ^ INV_Te2[(s1 >> 8) & 0xff] ^ INV_Te3[s0 & 0xff] ^ W_mix[15];
	//2 round
	s0 = INV_Te0[t0 >> 24] ^ INV_Te1[(t3 >> 16) & 0xff] ^ INV_Te2[(t2 >> 8) & 0xff] ^ INV_Te3[t1 & 0xff] ^ W_mix[8];
	s1 = INV_Te0[t1 >> 24] ^ INV_Te1[(t0 >> 16) & 0xff] ^ INV_Te2[(t3 >> 8) & 0xff] ^ INV_Te3[t2 & 0xff] ^ W_mix[9];
	s2 = INV_Te0[t2 >> 24] ^ INV_Te1[(t1 >> 16) & 0xff] ^ INV_Te2[(t0 >> 8) & 0xff] ^ INV_Te3[t3 & 0xff] ^ W_mix[10];
	s3 = INV_Te0[t3 >> 24] ^ INV_Te1[(t2 >> 16) & 0xff] ^ INV_Te2[(t1 >> 8) & 0xff] ^ INV_Te3[t0 & 0xff] ^ W_mix[11];
	//1 round
	t0 = INV_Te0[s0 >> 24] ^ INV_Te1[(s3 >> 16) & 0xff] ^ INV_Te2[(s2 >> 8) & 0xff] ^ INV_Te3[s1 & 0xff] ^ W_mix[4];
	t1 = INV_Te0[s1 >> 24] ^ INV_Te1[(s0 >> 16) & 0xff] ^ INV_Te2[(s3 >> 8) & 0xff] ^ INV_Te3[s2 & 0xff] ^ W_mix[5];
	t2 = INV_Te0[s2 >> 24] ^ INV_Te1[(s1 >> 16) & 0xff] ^ INV_Te2[(s0 >> 8) & 0xff] ^ INV_Te3[s3 & 0xff] ^ W_mix[6];
	t3 = INV_Te0[s3 >> 24] ^ INV_Te1[(s2 >> 16) & 0xff] ^ INV_Te2[(s1 >> 8) & 0xff] ^ INV_Te3[s0 & 0xff] ^ W_mix[7];
	//0 round sub + shift + addround
	s0 = (INV_Sbox[t0 >> 24] << 24) ^ (INV_Sbox[(t3 >> 16) & 0xff] << 16) ^ (INV_Sbox[(t2 >> 8) & 0xff] << 8) ^ (INV_Sbox[t1 & 0xff]) ^ W[0];
	s1 = (INV_Sbox[t1 >> 24] << 24) ^ (INV_Sbox[(t0 >> 16) & 0xff] << 16) ^ (INV_Sbox[(t3 >> 8) & 0xff] << 8) ^ (INV_Sbox[t2 & 0xff]) ^ W[1];
	s2 = (INV_Sbox[t2 >> 24] << 24) ^ (INV_Sbox[(t1 >> 16) & 0xff] << 16) ^ (INV_Sbox[(t0 >> 8) & 0xff] << 8) ^ (INV_Sbox[t3 & 0xff]) ^ W[2];
	s3 = (INV_Sbox[t3 >> 24] << 24) ^ (INV_Sbox[(t2 >> 16) & 0xff] << 16) ^ (INV_Sbox[(t1 >> 8) & 0xff] << 8) ^ (INV_Sbox[t0 & 0xff]) ^ W[3];

	u4byte_out(CT, s0);
	u4byte_out(CT + 4, s1);
	u4byte_out(CT + 8, s2);
	u4byte_out(CT + 12, s3);

}

////////////////////////////////////////////////////////////////////////////////
void ECB_Encryption(char* inputfile, char* outputfile, u32 W[]) {
	FILE* rfp, * wfp;
	u8* inputbuf, * outputbuf, r;
	u32 DataLen, i;



	fopen_s(&rfp, inputfile, "rb");
	if (rfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	fseek(rfp, 0, SEEK_END);
	DataLen = ftell(rfp);	//inputfile의 크기(바이트 단위)
	fseek(rfp, 0, SEEK_SET);

	r = DataLen % 16;
	r = 16 - r; //PKCS #7 패딩에 맞춰야 하는 바이트 수

	inputbuf = calloc(DataLen + r, sizeof(u8));
	outputbuf = calloc(DataLen + r, sizeof(u8));
	fread(inputbuf, 1, DataLen, rfp);
	fclose(rfp);
	memset(inputbuf + DataLen, r, r);

	for (i = 0; i < (DataLen + r) / 16; i++)
	{
		AES_ENC_Optimization(inputbuf + 16 * i, W, outputbuf + 16 * i, 128);
	}
	fopen_s(&wfp, outputfile, "wb");
	if (wfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	fwrite(outputbuf, 1, DataLen + r, wfp);

	fclose(wfp);
}
void XOR16Bytes(u8 S[16], u8 RK[16]) //S=S xor RK;
{
	S[0] ^= RK[0];
	S[1] ^= RK[1];
	S[2] ^= RK[2];
	S[3] ^= RK[3];
	S[4] ^= RK[4];
	S[5] ^= RK[5];
	S[6] ^= RK[6];
	S[7] ^= RK[7];
	S[8] ^= RK[8];
	S[9] ^= RK[9];
	S[10] ^= RK[10];
	S[11] ^= RK[11];
	S[12] ^= RK[12];
	S[13] ^= RK[13];
	S[14] ^= RK[14];
	S[15] ^= RK[15];
}
void CBC_Encryption(char* inputfile, char* outputfile, u32 W[]) {
	FILE* rfp, * wfp;
	u8* inputbuf, * outputbuf;
	u8 IV[16] = { 0x00, };
	u32 DataLen, i,r;



	fopen_s(&rfp, inputfile, "rb");
	if (rfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	fseek(rfp, 0, SEEK_END);
	DataLen = ftell(rfp);	//inputfile의 크기(바이트 단위)
	fseek(rfp, 0, SEEK_SET);

	r = DataLen % 16;
	r = 16 - r; //PKCS #7 패딩에 맞춰야 하는 바이트 수

	inputbuf = calloc(DataLen + r, sizeof(u8));
	outputbuf = calloc(DataLen + r, sizeof(u8));
	fread(inputbuf, 1, DataLen, rfp);
	fclose(rfp);
	memset(inputbuf + DataLen, r, r);

	XOR16Bytes(inputbuf, IV);
	AES_ENC_Optimization(inputbuf, W, outputbuf, 128);

	for (i = 1; i < (DataLen + r) / 16; i++)
	{
		XOR16Bytes(inputbuf + 16 * i, outputbuf + 16 * (i - 1));
		AES_ENC_Optimization(inputbuf + 16 * i, W, outputbuf + 16 * i, 128);
	}


	fopen_s(&wfp, outputfile, "wb");
	if (wfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	fwrite(outputbuf, 1, DataLen + r, wfp);

	fclose(wfp);
}
void CBC_Decryption(char* inputfile, char* outputfile, u32 W[])
{
	FILE* rfp, * wfp;
	u8* inputbuf, * outputbuf,r;
	u8 IV[16] = { 0x00, };
	u32 DataLen, i;
	

	fopen_s(&rfp, inputfile, "rb");//read binary mode
	if (rfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	fseek(rfp, 0, SEEK_END); // 파일의 끝으로 이동
	DataLen = ftell(rfp);	//inputfile의 크기(바이트 단위)
	fseek(rfp, 0, SEEK_SET);// 파일의 처음으로 이동

	inputbuf = calloc(DataLen, sizeof(u8));//sizeof(u8)크기만큼 DataLen의 공간을 할당할 수 있는 공간 할당
	outputbuf = calloc(DataLen, sizeof(u8));
	fread(inputbuf, 1, DataLen, rfp);//파일 포인터가 가리키는 위치부터 1byte씩 DataLen만큼 읽어 inputbuf에 저장
	fclose(rfp);


	AES_DEC_Optimization(inputbuf, W, outputbuf, 128);//복호화 루틴의 첫16바이트 복호화
	XOR16Bytes(outputbuf, IV);//초기벡터 xor연산
	for (i = 1; i < (DataLen) / 16; i++)
	{
		AES_DEC_Optimization(inputbuf + 16 * i, W, outputbuf + 16 * i, 128);
		XOR16Bytes(outputbuf + 16 * i, inputbuf + 16 * (i-1));
	}

	r = outputbuf[DataLen - 1]; //패딩제거 


	fopen_s(&wfp, outputfile, "wb");//write binary mode
	if (wfp == NULL) {
		perror("fopen_s 실패!!\n");
	}
	
	fwrite(outputbuf, 1, DataLen-r, wfp); //outputbuf 배열에 있는 데이터를 wfp파일 포인터로 1byte씩 DataLen-r만큼 씀

	fclose(wfp);

}


int main(int argc, char* argv[])
{
	/*u8 a, b, c;*/
	u8 PT[16] = { 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff };//평문
	u8 MK[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };																								
	u8 CT[16] = { 0x00, };//암호문,초기화되어있음
	u32 W[44] = { 0x00, };
	
	int keysize = 128;//키사이즈
	u8 RK[176] = { 0x00, };////라운드키
	int i;
	clock_t start, finish;
	 if (strcmp(argv[1], "cbc") == 0) {
		AES_keySchedule_Optimization(MK, W, keysize);
		start = clock();
		CBC_Decryption(argv[2], argv[3], W);
		finish = clock();
		printf("Computation time: %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);
	}
	
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Inversion 암호화 테이블 생성 코드
		/*u8 temp;
		printf("u32 Te0[256]={\n");
		for (i = 0; i < 256; i++)
		{
			temp =INV_Sbox[i];
			printf("0x%02x%02x%02x%02x, ", (u8)MULE(temp), (u8)MUL9(temp), (u8)MULD(temp), (u8)(MULB(temp)));
			if (i % 8 == 7)printf("\n");
		}
		printf("u32 INV_Te1[256]={\n");
		for (i = 0; i < 256; i++)
		{
			temp = INV_Sbox[i];
			printf("0x%02x%02x%02x%02x, ", (u8)(MULB(temp)), (u8)(MULE(temp)), (u8)(MUL9(temp)), (u8)(MULD(temp)));
			if (i % 8 == 7)printf("\n");
		}
		printf("u32 INV_Te2[256]={\n");
		for (i = 0; i < 256; i++)
		{
			temp = INV_Sbox[i];
			printf("0x%02x%02x%02x%02x, ", (u8)(MULD(temp)), (u8)(MULB(temp)), (u8)(MULE(temp)), (u8)(MUL9(temp)));
			if (i % 8 == 7)printf("\n");
		}
		printf("u32 INV_Te3[256]={\n");
		for (i = 0; i < 256; i++)
		{
			temp = INV_Sbox[i];
			printf("0x%02x%02x%02x%02x, ", (u8)(MUL9(temp)), (u8)(MULD(temp)), (u8)(MULB(temp)), (u8)(MULE(temp)));
			if (i % 8 == 7)printf("\n");
		}*/
	return 0;
}
