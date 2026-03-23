#pragma once
#include "StoreDTO.h"
#include <string>
#include <vector>
#include "json.hpp"

// =========================================================
// [기본 조립 블록] 검색 화면을 그리기 위한 단품 구조체들
// =========================================================

// 1. 인기 검색어 (카테고리명)
struct PopularKeyword
{
    int rank;
    std::string keyword;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PopularKeyword, rank, keyword)
};

// 2. 최근 검색어 (historyId: PK, searchDate: 검색 날짜)
struct RecentSearch
{
    int historyId;
    std::string keyword;
    std::string searchDate; // 예: "03.21"
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RecentSearch, historyId, keyword, searchDate)
};

// 3. 매장 검색 결과
// ⚠️ StoreDTO.h의 TopStoreInfo와 이름 충돌 → 주석 처리
// struct TopStoreInfo { ... };

// =========================================================
// [프로토콜 매칭 DTO] 클라이언트 <-> 서버 통신용 구조체
// =========================================================

// [2108] 매장 검색 위젯 요청
struct ReqResearchWidgetDTO
{
    std::string userId;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReqResearchWidgetDTO, userId)
};

// [2109] 매장 검색 위젯 응답
struct ResResearchWidgetDTO
{
    int status;
    std::vector<PopularKeyword> popularKeywords;
    std::vector<RecentSearch>   recentSearches;
    std::string searchDate;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResResearchWidgetDTO, status, popularKeywords, recentSearches, searchDate)
};

// [2110] 개별 검색어 삭제 요청
struct ReqResearchDeleteDTO
{
    std::string userId;
    int historyId;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReqResearchDeleteDTO, userId, historyId)
};

// [2111] 개별 검색어 삭제 응답
struct ResResearchDeleteDTO
{
    int status;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResResearchDeleteDTO, status)
};

// [2112] 검색어 추가 요청
struct ReqResearchAddDTO
{
    std::string userId;
    std::string keyword;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReqResearchAddDTO, userId, keyword)
};

// [2113] 검색어 추가 응답
struct ResResearchAddDTO
{
    int status;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResResearchAddDTO, status)
};

// [2114] 검색어 전체 삭제 요청
struct ReqResearchDelAllDTO
{
    std::string userId;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReqResearchDelAllDTO, userId)
};

// [2115] 검색어 전체 삭제 응답
struct ResResearchDelAllDTO
{
    int status;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResResearchDelAllDTO, status)
};

// [2116] 실시간 매장 검색 요청
struct ReqSearchStoreDTO
{
    std::string keyword;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ReqSearchStoreDTO, keyword)
};

// [2117] 실시간 매장 검색 응답
struct ResSearchStoreDTO
{
    int status;
    std::vector<TopStoreInfo> storeList;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResSearchStoreDTO, status, storeList)
};
