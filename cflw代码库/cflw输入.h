﻿#pragma once
#include <memory>
#include <map>
namespace cflw::输入 {
using t数量 = unsigned short;
using t索引 = unsigned short;
//==============================================================================
// 输出的信息
//==============================================================================
//一个按键
struct S按键 {
	S按键() = default;
	S按键(bool, bool);
	operator bool() const;
	bool f刚按下() const;
	bool f刚松开() const;
	bool f按下() const;
	bool f松开() const;
	bool f持续() const;
	bool f变化() const;
	void f覆盖上次() &;
	bool m这次 = false, m上次 = false;
};
//一个方向
struct S方向 {
	S方向() = default;
	S方向(float, float, float);
	bool f左() const;
	bool f右() const;
	bool f上() const;
	bool f下() const;
	bool f前() const;
	bool f后() const;
	float x = 0, y = 0, z = 0;
};
//==============================================================================
// 管理输入设备变量
//==============================================================================
//一组按键
class C按键组 {
public:
	C按键组(t数量);
	~C按键组();
	void f重置数量(t数量);
	t数量 fg数量() const;
	S按键 f按键(t索引 i) const;
	void f覆盖上次();
	void f清空上次();
	void f清空这次();
	t数量 m分配数量 = 0, m使用数量 = 0;
	bool *m这次 = nullptr, *m上次 = nullptr;
};
//解决1帧内快速按下松开按键时无法捕获按键的问题
class C缓冲按键组 {
public:
	C缓冲按键组(t数量);
	~C缓冲按键组();
	void f重置数量(t数量);
	t数量 fg数量() const;
	void f按下(t索引);
	void f松开(t索引);
	void f复制到按键(C按键组 &);//必须是C按键::m这次
	void f清空();
	t数量 m分配数量 = 0, m使用数量 = 0;
	bool *m立即 = nullptr, *m缓冲 = nullptr;
};
class C缓冲按键 {
public:
	void f按下();
	void f松开();
	void f复制到按键(S按键 &);
	void f清空();
	bool m立即 = false, m缓冲 = false;
};
class C缓冲方向2 {
public:
	C缓冲方向2();
	S方向 f这次方向() const;
	S方向 f上次方向() const;
	S方向 f方向差() const;
	void f覆盖上次();
	void f清空这次();
	float m这次[2], m上次[2];
};
class C方向3 {
public:
	C方向3();
	S方向 f方向() const;
	float m方向[3];
};
/*
template<typename t> concept T按键组 = requires(t a) {
	{a.f按键(0)}->S按键;
}
*/

//==============================================================================
// 输入设备接口
//==============================================================================
//声明
class I输入设备;
class I键盘;
class I鼠标;
class I手柄;
class I触摸;
class I触控点;
using tp输入设备 = std::shared_ptr<I输入设备>;
using tp键盘 = std::shared_ptr<I键盘>;
using tp鼠标 = std::shared_ptr<I鼠标>;
using tp手柄 = std::shared_ptr<I手柄>;
using tp触摸 = std::shared_ptr<I触摸>;
using tp触控点 = std::shared_ptr<I触控点>;
//定义
class I输入设备 {
public:
	virtual void f更新() = 0;
};
class I键盘 : public I输入设备 {
public:
	static constexpr t数量 c按键数量 = 256;
	virtual S按键 f按键(t索引 i) const = 0;
};
class I鼠标 : public I输入设备 {
public:
	static constexpr t数量 c按键数量 = 32;
	virtual S按键 f按键(t索引 i) const = 0;
	virtual S方向 f坐标() const = 0;
	virtual S方向 f移动() const = 0;
	virtual S方向 f滚轮() const = 0;
};
class I手柄 : public I输入设备 {
public:
	static constexpr t数量 c按键数量 = 32;
	virtual S按键 f按键(t索引) const = 0;
	virtual S方向 f方向(t索引) const = 0;
	virtual float f触发(t索引) const = 0;
};
class I触摸 : public I输入设备 {
public:
	virtual t数量 f触控点数() const = 0;	//返回仍按在屏幕上的触控点数量
	virtual tp触控点 fg触控点(t索引) const = 0;	//触摸实现要保证新出现的触控点总是在最后
	virtual tp触控点 fg新触控点() = 0;	//没有新的则返回nullptr，最好在每次更新后把新触控点取完
};
class I触控点 {
public:	//I触控点 的更新由 I触摸::f更新 管理
	virtual S按键 f按键() const = 0;
	virtual S方向 f坐标() const = 0;
	virtual S方向 f移动() const = 0;
};
//==============================================================================
// 按键工具
//==============================================================================
class C按键映射 {
public:
	static const t数量 c映射数量 = 32;
	std::map<t索引, t索引> ma映射;
	C按键组 m按键{c映射数量};
	C按键映射();
	~C按键映射();
	template<typename t按键组, typename t迭代器> static void f更新顺序(C按键组 &, const t按键组 &, t迭代器 开始, t迭代器 结束);	//顺序容器,方向<-
	template<typename t按键组, typename t迭代器> static void f更新关联l(C按键组 &, const t按键组 &, t迭代器 开始, t迭代器 结束);	//关联容器,方向<-
	template<typename t按键组, typename t迭代器> static void f更新关联r(C按键组 &, const t按键组 &, t迭代器 开始, t迭代器 结束);	//关联容器,方向->
	S按键 f按键(t索引 i) const;
	void f更新(const C按键组 &);
	void f更新(const I键盘 &);
	void f更新(const I鼠标 &);
	void f更新(const I手柄 &);
	void fs映射(t索引, t索引);
};
class C方向键 {
public:
	C方向键() = default;
	C方向键(t索引 正, t索引 反);
	static float f计算(float, const S按键 &, const S按键 &);
	void f更新(const C按键组 &);
	void f更新(I键盘&);
	void fs正反键(t索引, t索引);
	float m方向 = 0;
	t索引 m正向键 = 0, m反向键 = 0;
};
class C互斥键 {
public:
	std::tuple<bool, const S按键 &> f计算(const S按键 &, const S按键 &);
private:
	bool m键 = true;
};
class C按住重复 {
public:
	C按住重复() = default;
	C按住重复(float 延迟, float 间隔);
	void fs重置时间(float 延迟, float 间隔);
	bool f滴答(const S按键 &, float 间隔);
private:
	int m次数 = 0;
	float m时间 = 0;
	float m延迟 = 0.5f;
	float m间隔 = 0.1f;
};
//==============================================================================
// 按键映射模板实现
//==============================================================================
template<typename t按键组, typename t迭代器> void C按键映射::f更新顺序(C按键组 &a输出, const t按键组 &a输入, t迭代器 a开始, t迭代器 a结束) {
	t迭代器 v迭代 = a开始;
	for (t索引 i = 0; i != a输出.m分配数量 && v迭代 != a结束; ++i, ++v迭代) {
		const S按键 &v按键 = a输入.f按键(*v迭代);
		a输出.m这次[i] |= v按键.m这次;
	}
}
template<typename t按键组, typename t迭代器> void C按键映射::f更新关联l(C按键组 &a输出, const t按键组 &a输入, t迭代器 a开始, t迭代器 a结束) {
	for (t迭代器 v迭代 = a开始; v迭代 != a结束; ++v迭代) {
		const auto &[v输出索引, v输入索引] = *v迭代;
		const S按键 &v按键 = a输入.f按键(v输入索引);
		a输出.m这次[v输出索引] |= v按键.m这次;
	}
}
template<typename t按键组, typename t迭代器> void C按键映射::f更新关联r(C按键组 &a输出, const t按键组 &a输入, t迭代器 a开始, t迭代器 a结束) {
	for (t迭代器 v迭代 = a开始; v迭代 != a结束; ++v迭代) {
		const auto &[v输入索引, v输出索引] = *v迭代;
		const S按键 &v按键 = a输入.f按键(v输入索引);
		a输出.m这次[v输出索引] |= v按键.m这次;
	}
}
}	//namespace cflw::输入