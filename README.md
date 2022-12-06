# RSA-Algorithm
## 문제
키의길이가64비트인미니RSA알고리즘을구현한다.공개키암호방식의국제표준알고리즘인RSA는  
현재키의길이가최소2048비트가되어야안전하다.이과제에서는실세계에서활용하기에는안전하지  
않지만 RSA의 기본 원리를 이해하기에 충분하고, 구현이 까다롭지 않은 미니 RSA를 선택하였다.  
## RSA 키의 길이
RSA 키의 길이는 RSA 모률러스 𝑛의 길이를 의미한다. 구체적으로는 n 값을 이진수로 표현하기 위한  
최소 비트의 수를 뜻한다. 예를 들어, 𝑛이 15이면 4 비트로 15를 표현할 수 있으므로 길이가 4 비트가  
된다. 따라서 RSA 키의 길이가 64 비트이면 𝑛 값이 2^63 ≤ 𝑛 < 2^64 사이에 있어야 한다는 뜻이다.  
## 카마이클 함수𝜆(𝑛)
표준시스템에서는RSA키를생성할때오일러함수 𝜙(𝑛) 대신에계산양을줄여주는카마이클함수𝜆(𝑛)  
을 사용한다. 미니 RSA도𝜆(𝑛)을 사용하여 키를 생성한다. 𝜆(𝑛)은 다음과 같이 정의된다.  

𝜆(𝑛) = lcm(𝑝 − 1, 𝑞 − 1) = (𝑝 − 1) (𝑞 − 1) / gcd(𝑝 − 1, 𝑞 − 1)
## 함수 구현
외부에서보이는전역함수를아래열거한프로토타입을사용하여구현한다.각함수에대한요구사항은  
다음과 같다.  

• void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n) –길이가32비트내  
외인 임의의 두 소수 𝑝와 𝑞를 생성한 다음, 키의 길이가 64 바이트인 RSA 공개키 (𝑒, 𝑛)과 개인키  
(𝑑, 𝑛)을 생성한다. RSA 모듈러스 𝑛 값은 2^63 ≤ 𝑛 < 2^64을 만족해야 한다. 두 소수 𝑝와 𝑞의 길이가  
비슷할수록 더 안전하다는 점을 참고한다.  

• int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n) – 𝑚 ← 𝑚𝑘 mod 𝑛을 계산한다.  
계산 중 오류가 발생하면 0이 아닌 값을 넘겨주고, 없으면 0을 넘겨준다. 𝑚 ≥ 𝑛이면 𝑚이 값의  
범위를 넘었으므로 오류로 처리해야 한다.  
## 지역 함수
내부에서만사용하는지역함수는지난과제에서구현한것을각자필요에맞게사용한다.다음에열거한  
것은 이번 과제에 필요한 함수 목록이다.  

• static uint64_t gcd(uint64_t a, uint64_t b);  

• static uint64_t umul_inv(uint64_t a, uint64_t m);  

• static uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m);  

• static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m);  

• static uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m);  

• static int miller_rabin(uint64_t n);
## arc4random 함수
arc4random 계열의 함수는 암호학적으로 안전한 의사난수를 생성하기 위해 개발되었다. 기존 라이브러  
리에 있는 rand() 함수는 안전을 고려하지 않고 설계되었기 때문에 과제에서는 사용하지 않는다. 많이  
쓰이는 몇 가지 함수에 대한 용법은 아래와 같다.  

• uint32_t arc4random(void) – 32비트 난수를 생성하여 넘겨준다.  

• void arc4random_buf(void *buf, size_t nbytes) – 크기가 nbytes인 난수를 생성하여  
buf에 담아 넘겨준다.  

• uint32_t arc4random_uniform(uint32_t upper_bound) – 0과 upper_bound-1 사이의 32  
비트 난수를 넘겨준다.
## 골격 파일
구현이 필요한 골격파일 mRSA.skeleton.c와 함께 헤더파일 mRSA.h, 프로그램을 검증할 수 있는  
test.c,그리고Makefile을제공한다.이가운데test.c를제외한나머지파일은용도에맞게자유롭게  
수정할 수 있다.  
