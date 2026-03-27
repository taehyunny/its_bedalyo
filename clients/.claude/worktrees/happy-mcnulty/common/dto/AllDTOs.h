// dto/AllDTOs.h (퍼사드 파일)
#pragma once

// include "dto/각종DTO.h" 파일들을 한 곳에서 모아서 관리하는 퍼사드 헤더 파일
// 클라이언트 팀원들은 이 파일 하나만 #include 하면 모든 통신 준비 끝!
#include "dto/BaseDTO.h"
#include "../include/Global_protocol.h"
#include "dto/AccountDTO.h"
#include "dto/StoreDTO.h"
#include "dto/OrderDTO.h"
