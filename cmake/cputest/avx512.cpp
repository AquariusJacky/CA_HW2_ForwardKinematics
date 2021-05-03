
#include <immintrin.h>

int main() {
    __m512i zmm = _mm512_setzero_si512();
#if defined __GNUC__ && defined __x86_64__
    asm volatile("" : : : "zmm16", "zmm17", "zmm18", "zmm19");
#endif
    return 0;
}
