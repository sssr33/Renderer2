#pragma once

#include <type_traits>

template<bool enable, class T>
struct cond_add_reference{
	typedef typename std::add_reference<T>::type type;
};

template<class T>
struct cond_add_reference<false, T>{
	typedef T type;
};