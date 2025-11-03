#pragma once
#include <functional>


namespace typoon::util
{

/**
 * @brief operator() 호출 시 null 체크를 할 필요가 없는 콜백 래퍼 클래스
 * @tparam TArgType 콜백 인자 타입
 */
template<typename ...TArgType>
class NullableCallback : std::function<void(TArgType...)>
{
public:
    using std::function<void(TArgType...)>::function;

    void operator()(TArgType... args) const;
};

}


#include "function.inl"
