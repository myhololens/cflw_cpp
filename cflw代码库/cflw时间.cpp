#include "cflw时间.h"
namespace cflw {
namespace 时间 {
//=============================================================================
// 计算时间
//=============================================================================
t时间点 C计算时间::f现在() {
	return std::chrono::system_clock::now();
}
t时间段 C计算时间::f零() {
	return t时间段::zero();
}
t时间段 C计算时间::f间隔(const t时间点 &p0, const t时间点 &p1) {
	return std::chrono::duration_cast<t时间段>(p1 - p0);
}
//=============================================================================
// 时间间隔
//=============================================================================
void C时间间隔::f重置() {
	m上次时间 = C计算时间::f现在();
}
void C时间间隔::f刷新() {
	m上次时间 = m这次时间;
	m这次时间 = C计算时间::f现在();
}
t时间段 C时间间隔::f计算() {
	f刷新();
	return fg间隔();
}
t时间段 C时间间隔::fg间隔() const {
	return C计算时间::f间隔(m上次时间, m这次时间);
}
//=============================================================================
// 计时器
//=============================================================================
void C计时器::f重置(double t) {
	m持续时间 = C计算时间::f零();
	m上次时间 = C计算时间::f现在();
	fs间隔(t);
}
void C计时器::fs间隔(double t) {
	m滴答时间 = t时间段(t);
}
bool C计时器::f滴答() {
	auto v这次时间 = C计算时间::f现在();
	if (m滴答时间.count() <= 0) {
		m上次时间 = v这次时间;
		return true;
	}
	if (v这次时间 != m上次时间) {
		auto v间隔时间 = C计算时间::f间隔(m上次时间, v这次时间);
		m上次时间 = v这次时间;
		m持续时间 += v间隔时间;
	}
	if (m持续时间 > m滴答时间) {
		if (m滴答时间.count() == 0) {
			m持续时间 = C计算时间::f零();
		} else {
			m持续时间 -= m滴答时间;
			//丢弃多余的时间
			double v次数 = floor(m持续时间.count() / m滴答时间.count());
			if (v次数 >= 2) {
				m持续时间 -= m滴答时间 * (v次数 - 1);
			}
		}
		return true;
	} else {
		return false;
	}
}
//=============================================================================
// 计帧器
//=============================================================================
void C计帧器::f重置() {
	m时间.clear();
	m时间.push_back(C计算时间::f现在());
}
double C计帧器::f计算() {
	const t时间点 v这次时间 = C计算时间::f现在();
	const double v这次间隔 = C计算时间::f间隔(m时间.back(), v这次时间).count();
	const int v保留数量 = (int)(c取数范围 / v这次间隔);
	const int v删除数量 = m时间.size() - v保留数量 - 1;
	if (v删除数量 <= 0) {
		++m删除计数;
		if (m删除计数 >= 2) {
			m时间.pop_front();
			m删除计数 = 0;
		}
	} else {
		for (int i = 0; i < v删除数量; ++i) {
			m时间.pop_front();
			m删除计数 = 0;
		}
	}
	m时间.push_back(v这次时间);
	const double v总间隔 = C计算时间::f间隔(m时间.front(), v这次时间).count();
	const double v数量 = (double)m时间.size();
	const double v帧速率 = v数量 / (v总间隔 + (1 / m帧数率));
	m帧数率 = (m帧数率 + v帧速率) / 2;
	return m帧数率;
}
//=============================================================================
// 秒表
//=============================================================================
void C秒表::f重置() {
	m开始时间 = C计算时间::f现在();
	m上次时间 = C计算时间::f现在();
}
double C秒表::f开始到现在() {
	auto v这次时间 = C计算时间::f现在();
	auto v持续时间 = C计算时间::f间隔(m开始时间, v这次时间);
	m上次时间 = v这次时间;
	return v持续时间.count();
}
double C秒表::f上次到现在() {
	auto v这次时间 = C计算时间::f现在();
	auto v持续时间 = C计算时间::f间隔(m上次时间, v这次时间);
	m上次时间 = v这次时间;
	return v持续时间.count();
}
//=============================================================================
}
}