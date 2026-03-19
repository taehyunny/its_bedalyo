# 🛵 이츠 배달료 (Its Bedalyo)
> 현재 버전: ver 0.0.9
> 서버 포트번호 : 8080
> 서버 IP : 10.10.10.123
---
MARIADB 접속 계정
---
> 리눅스 : mariadb -h 10.10.10.123  -u bedalyo -p

> 윈도우 cmd창 : mysql -h 10.10.10.123 -u bedalyo -p

> 비밀번호 : 1234
=======

---
## 🚩 진행 상황 (Status)
- [x] **[필독]** 필수 사항 Git pull 후 현재 진행 상황 확인 부탁드립니다.
- [x] **[서버]** 서버와 깃 분리 했으니 팀원분들 깃 조율 해주세요
- [x] JSON 헤더 + 바디 설계중 (현재 완료)
- [x] 로그인, 상점, 주문 , 기본 DTO 설계 완료 ( 참조 하여 설계 )
- [ ] 회원가입 및 로그인 확인 완료 ( 테이블 저장 확인해봐야함 )
- [ ] 상점 1개 추가 완료 했으니 DB 접속하시고 확인해보세요
---
## 🔄 업데이트 이력 (Release Notes)
ver 0.0.15
- DB 테이블 상점, 메뉴 에 칼럼 추가
- STORES 테이블에 누락된 상용화 디테일 컬럼 추가
ALTER TABLE STORES 
ADD image_url VARCHAR(255),                            -- 가게 썸네일 이미지 링크
ADD min_order_amount INT DEFAULT 0,                    -- 최소 주문 금액
ADD rating DECIMAL(2,1) DEFAULT 0.0,                   -- 별점 (예: 4.5, 0.0으로 초기화)
ADD review_count INT DEFAULT 0,                        -- 리뷰 개수
ADD delivery_time_range VARCHAR(50) DEFAULT '20~30분'; -- 예상 배달 시간 (기본값 세팅)

- MENUS 테이블에 누락된 상용화 디테일 컬럼 추가
ALTER TABLE MENUS 
ADD image_url VARCHAR(255),                            -- 음식 사진 링크
ADD menu_category VARCHAR(50) DEFAULT '기본 메뉴',        -- 메뉴 그룹명
ADD is_popular TINYINT(1) DEFAULT 0;                   -- 1: 인기메뉴(BEST), 0: 일반
- StoreDTO 수정 완료

ver 0.0.12
- 사용자 클라이언트 포토토콜 정의
- 가게(사장님) MFC 회원가입 부분 수정중


ver 0.0.9 (수정중)
- 서버 회원가입 로직 구현중
- DTO 설계 완료 ( 추후 추가 예정 )
- DTO #include 참조 방법 아래 확인


ver 0.0.8 (수정중)
- DB 구조 설계 진행중
- Cmake로 헤더 경로 문제 해결 
- 프로토콜 정의 예시 코드 ( 수정 부탁드립니다.)


ver 0.0.7 (수정중)
- DB 구조 설계 진행중 
- 프로토콜 정의 예시 코드 ( 수정 부탁드립니다.)

ver 0.0.6 (서버 수정중)
- 서버 폴더 및 파일 생성

ver 0.0.4 (작업 완료)
- 프로토콜 정의 예시 및 가이드 추가
- DTO & DAO 예시 추가

ver 0.0.3 (작업 완료)
- `json.hpp` 라이브러리 추가 및 환경 세팅 완료
- 프로토콜 정의 예시 및 가이드 추가

ver 0.0.1 (2026-03-15)
- 분산 시스템 대응 다중 클라이언트 폴더 구조 생성
- 프로젝트 초기 설정 및 협업 가이드라인 배치 완료

---
## 🎯 각 파트별 요구 사항 
> [공지] 클라이언트 팀원분들은 통신 시 이 파일 하나만 #include 하시면 됩니다!
> 예: #include "dto/AllDTOs.h"


📡 통신 프로토콜 규약 
 8바이트 헤더 규칙을 반드시 준수해야 합니다.
 [ 필드          크기            설명 ]
  Signature  - 2 Bytes - "우리 팀 패킷 맞지?" 확인용 예(0x4543)

   CmdID     - 2 Bytes - "어떤 기능 실행해?" 결정

   BodySize  - 4 Bytes - "뒤에 오는 JSON이 몇 글자야?"

   JSON Body -   가변  -  실제 데이터 내용
=======
> 예: #include "dto/AllDTOs.h"