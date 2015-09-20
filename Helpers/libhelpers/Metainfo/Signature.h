#pragma once

#include <type_traits>

template<class T>
class SigHelper{
};

template<class R, class T>
class SigHelper<R(T::*)() const>{
public:
	typedef R R;
	typedef T T;
	typedef void A1;
	typedef void A2;
};

template<class R, class T, class A1>
class SigHelper<R(T::*)(A1) const>{
public:
	typedef R R;
	typedef T T;
	typedef A1 A1;
	typedef void A2;
};

template<class R, class T, class A1, class A2>
class SigHelper<R(T::*)(A1, A2) const>{
public:
	typedef R R;
	typedef T T;
	typedef A1 A1;
	typedef A2 A2;
};

template<class T>
class Signature{
public:
	typedef typename SigHelper<decltype(&T::operator())>::R R;
	typedef typename SigHelper<decltype(&T::operator())>::A1 A1;
	typedef typename SigHelper<decltype(&T::operator())>::A2 A2;
};

template<class R>
class Signature<R()>{
public:
	typedef R R;
	typedef void A1;
	typedef void A2;
};

template<class R, class A1>
class Signature<R(A1)>{
public:
	typedef R R;
	typedef A1 A1;
	typedef void A2;
};

template<class R, class A1, class A2>
class Signature<R(A1, A2)>{
public:
	typedef R R;
	typedef A1 A1;
	typedef A2 A2;
};