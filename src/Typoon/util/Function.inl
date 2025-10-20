#pragma once


template<typename ...TArgType>
void NullableCallback<TArgType...>::operator()(TArgType... args) const
{
    if (*this)
    {
        std::function<void(TArgType...)>::operator()(std::forward<TArgType>(args)...);
    }
}
