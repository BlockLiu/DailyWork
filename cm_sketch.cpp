
#include <x86intrin.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 32

class cm_sketch
{
public:
	int window_sz;
	int d;
	int width;
	int idx;

	alignas(16) unsigned short clock[3][SIZE];	// width弄成8的倍数
	__m128i m2;

	cm_sketch(int wsz, int _d, int _w): window_sz(wsz), d(_d), width(_w), idx(0){
		for(int i = 0; i < 3; ++i)
			for(int j = 0; j < SIZE; ++j)
				clock[i][j] = (i * SIZE + j) % 256;
		m2 = _mm_set1_epi16(1);		// 在构造函数里面加这个
	}

	void updateClock(int clocksize)
	{

		int ct = d * width;
		// int len = (1 << clocksize) * ct / window_sz - 1;		// 这里记得改回来
		int len = 20;

		while(idx % 8 && len > 0){
			clock[idx / width][idx % width] -= clock[idx / width][idx % width] == 0 ? 0 : 1;
			idx = (idx + 1) % ct;
			len--;
		}

		while(len >= 8){
			unsigned short *ptr = &clock[idx / width][idx % width];
			__m128i m1 = _mm_load_si128((__m128i*)ptr);
			__m128i m3 = _mm_subs_epu16(m1, m2);
			_mm_store_si128((__m128i*)ptr, m3);

			idx = (idx + 8) % ct;
			len -= 8;
		}

		while(len > 0){
			clock[idx / width][idx % width] -= clock[idx / width][idx % width] == 0 ? 0 : 1;
			idx = (idx + 1) % ct;
			len--;
		}
	}

	void print()
	{
		for(int i = 0; i < d; ++i){
			printf("level-%d:", i);
			for(int j = 0; j < width; ++j)
				printf(" %.2d", clock[i][j]);
			printf("\n");
		}
		printf("\n");
	}
};


int main()
{
	cm_sketch cm(20, 3, SIZE);

	for(int i = 0; i < 6; ++i)
	{
		printf("******************** test-%d *****************\n", i);
		cm.print();
		cm.updateClock(10);
	}
	printf("******************** finished *****************\n");
	cm.print();
	return 0;
}
