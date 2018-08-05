#pragma once
#include "cflw数学.h"
namespace cflw::数学 {
//==============================================================================
//基本数学函数
//==============================================================================
template<typename t> int f取符号(const t &x) {
	return (x < 0) ? -1 : (x > 0 ? 1 : 0);
}
template<typename t> const t& f最大值(const t &a, const t &b) {
	return a > b ? a : b;
}
template<typename t> const t& f最小值(const t &a, const t &b) {
	return a < b ? a : b;
}
template<typename t> t f限制(const t &值, const t &最小值, const t &最大值) {
	return (值 > 最大值) ? 最大值 : ((值 < 最小值) ? 最小值 : 值);
}
template<typename t> t f接近(const t &源值, const t &目标值, const t &接近差) {
	return ((f绝对值(源值 - 目标值) <= 接近差) ? 目标值 : 源值);
}
template<typename t> t f倍数渐变(const t &源值, const t &目标值,
		const float &差值渐变倍数, const t &最小渐变 = (t)0, const t &最大渐变 = (t)9999) {
	if (源值 == 目标值) {
		return 源值;
	} else {
		const t v差值 = 目标值 - 源值;
		const t v渐变值 = (t)(v差值 * 差值渐变倍数);
		const t v限制渐变值 = f限制<t>(f绝对值<t>(v渐变值), f绝对值<t>(最小渐变), f绝对值<t>(最大渐变)) * f取符号<t>(v差值);
		const t v接近值 = f接近<t>(源值 + v限制渐变值, 目标值, f绝对值<t>(最小渐变));
		return v接近值;
	}
}
template<typename t> t f线性渐变(const t &源值, const t &目标值, const float &渐变值) {
	if (源值 == 目标值) {
		return 源值;
	} else {
		return f接近<t>(源值 + (t)(渐变值 * f取符号(目标值 - 源值)), 目标值, (t)渐变值);
	}
}
template<class t> t f求余(const t &p源值, const t &p除数) {
	const t v倍数 = floor(p源值 / p除数);
	return p源值 - p除数 * v倍数;
}
template<class t> bool f限制循环(t &p值, const t &p最大值) {
	//达到最大值则清零并返回真
	if (p值 >= p最大值) {
		p值 -= p最大值;
		return true;
	} else {
		return false;
	}
}
template<typename t> t f绝对值(const t &_) {
	return ((_ > 0) ? (_) : (-_));
}
template<typename t> std::tuple<t, t> sincos(t a) {
	return {sin(a), cos(a)};
}
//插值
template<typename t> t f插值(const t &a0, const t &a1, float p) {
	return a0 + (a1 - a0) * p;
}
//限
template<class t> int f限(t &a0, const t &a1) {
	const int r = floor(a0 / a1);
	a0 -= r * a1;
	return r;
}
//
template<class t> bool f同符号(const t &a1, const t &p2) {
	return (f取符号<t>(a1) == f取符号<t>(p2));
}
//平方立方
template<typename t> t f平方(const t &p) {
	return p * p;
}
template<typename t> t f立方(const t &p) {
	return p * p * p;
}
//
template<typename t> t f对齐(const t &x, const t &y) {
	const t a = x / y;
	const t b = floor(a);
	const t c = a - b;
	if (c < 0.5) {
		return y * b;
	} else {
		return y * ceil(a);
	}
}
template<typename t> t f循环(const t &p, const t &p最小, const t &p最大) {
	const t v差 = p最大 - p最小;
	const t v基本倍 = floor(p / v差);
	const t v循环倍 = ceil(p最小 / v差);
	return p - v差 * (v基本倍 - v循环倍);
}
template<typename t> t f差(const t &a, const t &b) {
	if (a >= b) {
		return a - b;
	} else {
		return b - a;
	}
}
template<typename tk, typename tv> tv f插值2(const std::pair<tk, tv> &a0, const std::pair<tk, tv> &a1, const tk &a键) {
	const tk k = (a键 - a0.first) / (a1.first - a0.first);
	return f插值<tv>(a0.second, a1.second, k);
}
template<typename tk, typename tv> tv f插值2(const tk &a键0, const tv &a值0, const tk &a键1, const tv &a值1, const tk &a键) {
	const tk k = (a键 - a键0) / (a键1 - a键0);
	return f插值<tv>(a值0, a值1, k);
}
template<typename t> t f反勾股(const t &p斜边, const t &p直角边) {
	return sqrt(p斜边*p斜边 - p直角边*p直角边);
}
template<typename t> t f四舍五入(const t &p) {
	const float v舍 = floor(p);
	return (p - v舍 < (t)0.5) ? v舍 : (v舍 + 1);
}
template<typename t> t f等腰梯形插值(const t &a小, const t &a大, float a斜宽, float x) {
	if (x < a斜宽) {
		return f插值<t>(a小, a大, x / a斜宽);
	} else if (const float v右边 = 1 - a斜宽; x > v右边) {
		return f插值<t>(a大, a小, (x - v右边) / a斜宽);
	} else {
		return a大;
	}
}
template<typename t返回, typename...t参数> t返回 f算术平均(t参数 &&...a参数) {
	return (t返回)(... + a参数) / sizeof...(t参数);
}
template<typename t返回, typename...t参数> t返回 f几何平均(t参数 &&...a参数) {
	return sqrt((t返回)(... * a参数));
}
template<typename t返回, typename...t参数> t返回 f绝对值最大值(t参数 &&...a参数) {
	t返回 v最大值 = 0;
	t返回 v原值 = 0;
	auto f = [&](const t返回 &a) {
		const t返回 v绝对值 = abs(a);
		if (v最大值 < v绝对值) {
			v最大值 = v绝对值;
			v原值 = a;
		}
	};
	(f(a参数), ...);
	return v原值;
}
//==============================================================================
// 各种计算类
//==============================================================================
template<typename t> const C角度计算<t> C角度计算<t>::c度(360);
template<typename t> const C角度计算<t> C角度计算<t>::c弧度((t)c二π);
//角度计算
template<typename t> C角度计算<t>::C角度计算(const t &p周角) :
	m周角{p周角}, m平角{p周角 / 2} {
}
template<typename t> t C角度计算<t>::f平均(const t &p角1, const t &p角2) const {
	t v角1 = f取正(p角1);
	t v角2 = f取正(p角2);
	if (v角1 == v角2) {
		return v角1;
	} else if (v角1 >= v角2) {
		std::swap(v角1, v角2);
	}
	if (abs(v角2 - v角1) >= m平角) {
		v角2 -= m周角;
	}
	return (v角1 + v角2) / 2;
}
template<typename t> t C角度计算<t>::f取正(const t &p角) const {
	return p角 - floor(p角 / m周角) * m周角;
}
template<typename t> t C角度计算<t>::f限制(const t &p角) const {
	t v角 = p角 + m平角;
	v角 -= floor(v角 / m周角) * m周角;
	v角 -= m平角;
	return v角;
}
template<typename t> t C角度计算<t>::f取半(const t &p角) const {
	return p角 - floor(p角 / m平角) * m平角;
}
template<typename t> t C角度计算<t>::f旋转渐变(const t &s, const t &d, const t &max) const {
	const t v差 = f取正(d - s);
	if (f取半(v差) <= max) {
		return d;
	} else {
		if (v差 > m周角 / 2) {
			return s - max;
		} else {
			return s + max;
		}
	}
}
template<typename t> t C角度计算<t>::f夹角(const t &p角1, const t &p角2) const {
	t v = p角1 - p角2;
	v = f取正(v);
	if (v >= m平角) {
		v = m周角 - v;
	}
	return v;
}
template<typename t> t C角度计算<t>::f方向(const t &x, const t &y) const {
	return atan2(y, x) / ((t)c二π / m周角);
}
template<typename t> t C角度计算<t>::f插值(const t &p角1, const t &p角2, const t &p插值) const {
	t v = p角2 - p角1;	//角1到角2的夹角
	v = f取正(v);
	if (v > m平角) {
		v = m周角 - v;	//获得夹角
		return p角1 - v * p插值;
	} else {
		return p角1 + v * p插值;
	}
}
}	//namespace cflw::数学 
