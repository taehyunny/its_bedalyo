#pragma once
#include <QString>
#include <QLocale>

// ============================================================
// StoreUtils - 가게 카드 UI에서 공통으로 사용하는 유틸 함수 모음
//
// 기존에 homewidget.cpp / searchresultwidget.cpp / menucategori.cpp
// 3곳에 중복 정의되어 있던 함수들을 한 곳으로 통합
//
// 사용법:
//   #include "storeutils.h"
//   StoreUtils::categoryEmoji("한식")  → "🍚"
// ============================================================
namespace StoreUtils
{
    inline QString formatWon(int amount)
    {
        return QLocale(QLocale::Korean).toString(amount) + "원";
    }

    inline QString formatDeliveryFee(int fee)
    {
        return (fee == 0)
            ? "무료배달"
            : "배달비 " + QLocale(QLocale::Korean).toString(fee) + "원";
    }

    inline QString categoryEmoji(const QString &cat)
    {
        if (cat == "한식")     return "🍚";
        if (cat == "중식")     return "🥟";
        if (cat == "일식")     return "🍱";
        if (cat == "치킨")     return "🍗";
        if (cat == "양식")     return "🍕";
        if (cat == "분식")     return "🍢";
        if (cat == "카페")     return "☕";
        if (cat == "베이커리") return "🥐";
        if (cat == "돈까스")   return "🥩";
        if (cat == "햄버거")   return "🍔";
        if (cat == "족발/보쌈") return "🍖";
        if (cat == "초밥/회")  return "🍣";
        if (cat == "도시락")   return "🍱";
        return "🍽";
    }

    inline QString placeholderColor(const QString &cat)
    {
        if (cat == "한식")     return "#d8fde4";
        if (cat == "중식")     return "#fde8d8";
        if (cat == "일식")     return "#d8eafd";
        if (cat == "치킨")     return "#fdf5d8";
        if (cat == "양식")     return "#fdd8d8";
        if (cat == "분식")     return "#fde8f0";
        if (cat == "카페")     return "#ede8fd";
        if (cat == "베이커리") return "#fdf0d8";
        if (cat == "돈까스")   return "#fff3d8";
        if (cat == "햄버거")   return "#fdf0e0";
        if (cat == "족발/보쌈") return "#e8fde8";
        if (cat == "초밥/회")  return "#d8eafd";
        if (cat == "도시락")   return "#fdf0d8";
        return "#f0f0f0";
    }
}
