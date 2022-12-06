/*
 * Copyright 2020-2022. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로
 * 제작되었다.
 */
#ifdef __linux__
#include <bsd/stdlib.h>
#elif __APPLE__
#include <stdlib.h>
#else
#include <stdlib.h>
#endif
#include "mRSA.h"

/*
 * mod_add() - computes a + b mod m
 */
static uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m) {
  // a와 b가 m보다 작다는 가정으로 시작한다.
  if (a < m && b < m) {
    /* a+b >= m이면 결과에서 m을 빼줘야 하므로 오버플로가
    발생하지 않도록 a-(m-b)를 계산한다. */
    if (a >= m - b)
      return a - (m - b);
    // 그렇지 않으면 그냥 a+b를 계산한다.
    else
      return a + b;
  }
  // 가정이 틀리다면 느리더라도 나머지 연산을 실행한다.
  else
    return ((a % m) + (b % m)) % m;
}

/*
 * mod_sub() - computes a - b mod m
 */
static uint64_t mod_sub(uint64_t a, uint64_t b, uint64_t m) {
  // a와 b가 m보다 작다는 가정으로 시작한다.
  if (a < m && b < m) {
    // 만일 a < b이면 결과가 음수가 되므로 m을 더해서 양수로 만든다.
    if (a < b)
      return a + (m - b);
    // 그렇지 않으면 그냥 a-b를 계산한다.
    else
      return a - b;
  }
  // 가정이 틀리다면 느리더라도 나머지 연산을 실행한다.
  else {
    // 만일 a < b이면 결과가 음수가 되므로 m을 더해서 양수로 만든다.
    if (a < b)
      return (a + (m - b)) % m;
    // 그렇지 않으면 그냥 a-b를 계산한다.
    else
      return (a - b) % m;
  }
}

/*
 * mod_mul() - computes a * b mod m
 */
static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m) {
  /* a*b에서 오버플로가 발생할 수 있기 때문에 덧셈을 사용하여
  빠르게 계산할 수 있는 "double addition" 알고리즘을 사용한다. */
  uint64_t r = 0;
  while (b > 0) {
    if (b & 1)
      r = mod_add(r, a, m);
    b = b >> 1;
    a = mod_add(a, a, m);
  }
  return r;
}

/*
 * mod_pow() - computes a^b mod m
 */
static uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m) {
  /* a^b에서 오버플로가 발생할 수 있기 때문에 곱셈을 사용하여
  빠르게 계산할 수 있는 "square multiplication"
  알고리즘을 사용한다. */
  uint64_t r = 1;
  while (b > 0) {
    if (b & 1)
      r = mod_mul(r, a, m);
    b = b >> 1;
    a = mod_mul(a, a, m);
  }
  return r;
}

/*
 * gcd() - Euclidean algorithm
 */
static uint64_t gcd(uint64_t a, uint64_t b) {
  uint64_t temp;
  /* 재귀함수 대신에 반복문을 사용 */
  while (b >= 0) {
    /* 한 쪽이 0인 경우 나머지 한 쪽이 결과 */
    if (b == 0)
      return a;
    /* gcd(b, a mod b)를 위해 swap하는 부분 */
    else {
      temp = a;
      a = b;
      b = temp % b;
    }
  }
  /* 최대공약수가 없을 경우 0을 반환 */
  return 0;
}

/*
 * mul_inv() - computes multiplicative inverse a^-1 mod m
 * It returns 0 if no inverse exist.
 */
static uint64_t mul_inv(uint64_t a, uint64_t m) {
  uint64_t d0 = a, d1 = m;
  uint64_t x0 = 1, x1 = 0, q, temp;
  while (d1 > 1) {
    q = d0 / d1;
    d0 = mod_sub(d0, mod_mul(q, d1, m), m);
    /* swap(d0, d1) */
    temp = d0;
    d0 = d1;
    d1 = temp;
    x0 = mod_sub(x0, mod_mul(q, x1, m), m);
    /* swap(x0, x1) */
    temp = x0;
    x0 = x1;
    x1 = temp;
  }
  if (d1 == 1)
    return (x1 > 0 ? x1 : x1 + m);
  else
    return 0;
}

/*
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3317044064679887385961981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
static const uint64_t a[BASELEN] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns 1 if n is prime, 0 otherwise.
 */
static int miller_rabin(uint64_t n) {
  /* a[0]의 값이 2 이므로 n <= 3에서 n - 1 = 2로
  a[0] < n - 1가 어긋나기에 n이 2, 3에서는 바로 소수로 한다. */
  if (n == 2 || n == 3)
    return PRIME;

  // n이 2가 아닌 짝수이면 소수가 아니다.
  if (n % 2 == 0)
    return COMPOSITE;

  // n-1 = 2^k * q를 알아낸다.
  uint64_t k = 0, q = n - 1;
  while (q % 2 == 0) {
    k++;
    q = q >> 1;
  }

  // a[BASELEN]를 돌리는 i, 0~k-1까지를 반복하는 j를 선언한다.
  // n이 소수일 가능성을 나타내는 inconclusive를 선언한다.
  int i, j, inconclusive;
  for (i = 0; i < BASELEN; i++) {
    if (a[i] < n - 1) {
      // 매 계산마다 가능성이 없다에서 test를 시작한다.
      inconclusive = 0;
      // test(a^q mod n == 1)에 통과하면 가능성을 1로 수정한다.
      if (mod_pow(a[i], q, n) == 1) {
        inconclusive = 1;
        continue;
      }

      /* test((a^q)^(2^j) mod n == n-1)에 통과하면
      가능성을 1로 수정한다. 이때 페르마의 정리로 (2^j)을
      mod n-1로 계산하여 속도를 향상시켰다. */
      for (j = 0; j < k; j++) {
        if (mod_pow(mod_pow(a[i], q, n), mod_pow(2, j, n - 1), n) == n - 1) {
          inconclusive = 1;
          break;
        }
      }
    }
    // 어느 test에도 통과하지 못했으면 합성수이다.
    if (!inconclusive)
      return COMPOSITE;
  }
  // 모든 test에 통과하면 소수로 판단한다.
  return PRIME;
}

/*
 * mRSA_generate_key() - generates mini RSA keys e, d and n
 *
 * Carmichael's totient function Lambda(n) is used.
 */
void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n) {
  // r1, r2는 각각 32 bits, 64 bits에서 가장 작은 수이다.
  uint32_t r1 = 0x80000000;
  uint64_t r2 = 0x8000000000000000;
  uint64_t p, q;

  // n을 우선 작게 설정하여 while문 안으로 들어가게 한다.
  *n = 100;
  // n이 64 bits가 될 때까지 검사한다.
  while (*n < r2) {
    // p와 q를 소수가 아닌 수로 설정한다.
    p = 4;
    q = 4;
    /* p와 q가 소수가 될 때까지 재설정한다.
    이때 r1을 더해주어 32 bits임을 강제한다. */
    while (!miller_rabin(p)) {
      p = r1 + arc4random_uniform(r1);
    }
    while (!miller_rabin(q)) {
      q = r1 + arc4random_uniform(r1);
    }
    *n = p * q;
  }

  // 1 < e < ø(n), e와 ø(n)은 서로소가 될 때까지 재설정한다.
  *e = arc4random_uniform((p - 1) * (q - 1));
  while (*e <= 1 || gcd(*e, (p - 1) * (q - 1)) != 1) {
    *e = arc4random_uniform((p - 1) * (q - 1));
  }
  // ed = 1 mod 人(n)임을 이용하여 e의 역수인 d를 구한다.
  *d = mul_inv(*e, (p - 1) * (q - 1) / gcd(p - 1, q - 1));
}

/*
 * mRSA_cipher() - compute m^k mod n
 *
 * If data >= n then returns 1 (error), otherwise 0 (success).
 */
int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n) {
  // m ≥ n이면 m이 값 범위를 넘었으므로 오류로 처리해야 한다.
  if (*m >= n)
    return 1;
  // m^k mod n을 계산하고 이상이 없으므로 0을 return한다.
  *m = mod_pow(*m, k, n);
  return 0;
}
