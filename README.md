
# 20-1 SecureProgramming : 보안프로그래밍 과제
- OpenSSL을 활용하여 클라이언트-서버 간 TLS 보안 통신 직접 구현


## 개요

이 프로젝트는 클라이언트와 서버 간에 안전한 통신을 구현하기 위한 실습 과제로,  
OpenSSL을 사용하여 TLS 기반의 보안 채널을 구축하고 상호 인증(Mutual Authentication) 과정을 포함합니다.


## 주제

- 클라이언트와 서버 간의 보안 통신 구현
- 인증서 기반의 양방향 인증
- OpenSSL의 멀티스레드 환경 구성 및 활용


## 주요 파일 설명

### `server.c`  
OpenSSL을 이용해 클라이언트와 안전하게 통신하는 서버 프로그램

```c
SSL_CTX *setup_server_ctx(void)
```
- `server.pem` 인증서와 개인키 로드
- `SSLv23_method()` 기반 TLS 설정

```c
int do_server_loop(SSL *ssl)
```
- 클라이언트 요청 처리 (CLOSE, DOWNLOAD)
- `mutex`로 다중 클라이언트 동기화

---

### `client.c`  
서버와 SSL 기반 통신하는 클라이언트 프로그램

```c
SSL_CTX *setup_client_ctx(void)
```
- `client.pem` 인증서 사용

```c
SSL_write(...) // 명령어 전송
SSL_read(...)  // 서버 응답 수신
system("xdg-open <경로>"); // 파일 열기
```

```c
THREAD_CREATE(send, write_thread, ssl);
THREAD_CREATE(recv, read_thread, ssl);
```
- 쓰레드 기반 병렬 통신 (입력/출력 분리)



### `common.[c/h]`  
- OpenSSL 초기화 (`init_OpenSSL`)
- 에러 처리 (`handle_error`)
- 인증서 검증 콜백 (`verify_callback`)
- 호스트와 인증서 일치 여부 검사 (`post_connection_check`)



### `reentrant.[c/h]`  
- 멀티스레드 환경에서 OpenSSL을 안전하게 사용할 수 있도록 락(lock) 설정
- `CRYPTO_set_locking_callback` 등 내부 락 함수 등록



### `makekey.sh`  
- OpenSSL을 이용한 RSA 키 + 인증서 생성 자동화 스크립트
- 루트 인증서 → 중간 인증서 → 서버/클라이언트 인증서 체계 구성



### `makefile`  
- `client`, `server`, `client1` 실행 파일 빌드
- `make`, `make clean` 지원



## 보안 기능 요약

| 기능 | 설명 |
|------|------|
| TLS 통신 | OpenSSL을 이용한 암호화된 전송 |
| 상호 인증 | 서버-클라이언트 모두 인증서로 검증 |
| 인증서 검증 | verify_callback으로 검증 실패 로그 출력 |
| 스레드 안전성 | OpenSSL 락 등록 (`reentrant.c`) |
| 경로 전송 방식 | `xdg-open` 으로 경로 자동 실행 (보안 유의 필요) |


## 실행 방법

### 1. 인증서 생성
```bash
chmod +x makekey.sh
./makekey.sh
```

### 2. 컴파일
```bash
make
```

### 3. 실행 순서

- 터미널 1:
```bash
./server
```

- 터미널 2:
```bash
./client
```


## 명령어 예시

클라이언트 실행 후 아래 명령어 입력:

- `DOWNLOAD` : test.jpg 경로 받아서 자동 실행
- `MAN` : 명령어 설명 출력
- `CLOSE` : 연결 종료


## 개발 환경 및 의존성

- Linux / macOS 기반 OS
- GCC, Make
- OpenSSL (1.1 이상)
- `xdg-open` 유틸리티 (Linux에서 이미지 열기용)

