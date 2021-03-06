#include <algorithm>
#include <memory>
#include "cflw输入_win.h"
#include "cflw视窗.h"
namespace cflw::输入::win {
//==============================================================================
// 类定义
//==============================================================================
//键盘
class C键盘 : public I键盘 {
	friend class C输入;
public:
	S按键 f按键(t索引 i) const override;
	void f更新() override;
private:
	C按键组 m按键{c按键数量};
	C缓冲按键组 m缓冲{c按键数量};
};
//鼠标
class C鼠标 : public I鼠标 {
	friend class C输入;
public:
	C鼠标(HWND, float);
	S按键 f按键(t索引 i) const override;
	S方向 f坐标() const override;
	S方向 f移动() const override;
	S方向 f滚轮() const override;
	void f更新() override;
private:
	HWND m窗口;
	float m缩放 = 1;
	C缓冲按键组 m缓冲{c按键数量};
	C按键组 m按键{c按键数量};
	C缓冲方向2 m坐标, m滚轮;
};
//触摸
class C触摸 : public I触摸 {
	friend class C输入;
public:
	C触摸(HWND, float);
	void f更新() override;
	t数量 f触控点数() const override;
	tp触控点 fg触控点(t索引) const override;
	tp触控点 fg新触控点() override;
	void f接收输入(const TOUCHINPUT *, size_t);
public:
	std::vector<std::shared_ptr<C触控点>> ma触控点;
	HWND m窗口;
	float m缩放 = 1;
	t索引 m新触控点索引 = 0;
	t数量 m新触控点数 = 0;
};
class C触控点 : public I触控点 {
	friend class C输入;
public:
	C触控点(const C触摸 &, const TOUCHINPUT &);
	S按键 f按键() const override;
	S方向 f坐标() const override;
	S方向 f移动() const override;
	void f接收输入(const TOUCHINPUT &);
public:
	const C触摸 *m触摸 = nullptr;
	C缓冲按键 m缓冲;
	S按键 m按键;
	C缓冲方向2 m坐标;
	LONG m点x = 0, m点y = 0;
	DWORD m标识;
};
//==============================================================================
// 全局变量&函数
//==============================================================================
const std::tuple<int, int, int> ca对应左右键[] = {
	std::tuple<int, int, int>((int)E键盘按键::e上档, (int)E键盘按键::e左上档, (int)E键盘按键::e右上档),
	std::tuple<int, int, int>((int)E键盘按键::e控制, (int)E键盘按键::e左控制, (int)E键盘按键::e右控制),
	std::tuple<int, int, int>((int)E键盘按键::e交替, (int)E键盘按键::e左交替, (int)E键盘按键::e右交替),
};
//==============================================================================
// 输入引擎
//==============================================================================
C输入::~C输入() {
	f销毁();
}
void C输入::f初始化(HWND a窗口, float a缩放) {
	m窗口 = a窗口;
	if (a缩放 <= 0) {
		m缩放 = 视窗::C环境::fg缩放();
	} else {
		m缩放 = a缩放;
	}
}
void C输入::f销毁() {
	UnregisterTouchWindow(m窗口);
	m键盘.reset();
	m鼠标.reset();
	m触摸.reset();
	m窗口 = nullptr;
}
void C输入::f窗口消息(UINT a消息, WPARAM w, LPARAM l) {
	if (m键盘) {
		switch (a消息) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			m键盘->m缓冲.f按下(w);
			//区分左右键
			for (const auto &v : ca对应左右键) {
				if (std::get<0>(v) == w) {
					const int &v左 = std::get<1>(v);
					const int &v右 = std::get<2>(v);
					if (GetKeyState(v左) & 0x8000) {
						m键盘->m缓冲.f按下(v左);
					} 
					if (GetKeyState(v右) & 0x8000) {
						m键盘->m缓冲.f按下(v右);
					}
				}
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			m键盘->m缓冲.f松开(w);
			//区分左右键
			for (const auto &v : ca对应左右键) {
				if (std::get<0>(v) == w) {
					const int &v左 = std::get<1>(v);
					const int &v右 = std::get<2>(v);
					if (!(GetKeyState(v左) & 0x8000)) {
						m键盘->m缓冲.f松开(v左);
					}
					if (!(GetKeyState(v右) & 0x8000)) {
						m键盘->m缓冲.f松开(v右);
					}
				}
			}
			break;
		case WM_ACTIVATE:
			if (w == WA_INACTIVE) {
				m键盘->m缓冲.f清空();
			}
		}
	}
	if (m鼠标) {
		switch (a消息) {
		case WM_LBUTTONDOWN:
			m鼠标->m缓冲.f按下((int)E鼠标按键::e左键);
			break;
		case WM_RBUTTONDOWN:
			m鼠标->m缓冲.f按下((int)E鼠标按键::e右键);
			break;
		case WM_MBUTTONDOWN:
			m鼠标->m缓冲.f按下((int)E鼠标按键::e中键);
			break;
		case WM_LBUTTONUP:
			m鼠标->m缓冲.f松开((int)E鼠标按键::e左键);
			break;
		case WM_RBUTTONUP:
			m鼠标->m缓冲.f松开((int)E鼠标按键::e右键);
			break;
		case WM_MBUTTONUP:
			m鼠标->m缓冲.f松开((int)E鼠标按键::e中键);
			break;
		case WM_MOUSEWHEEL:
			m鼠标->m滚轮.m这次[0] = GET_WHEEL_DELTA_WPARAM(w) / 120.f;
			break;
		case WM_MOUSEHWHEEL:
			m鼠标->m滚轮.m这次[1] = GET_WHEEL_DELTA_WPARAM(w) / 120.f;
			break;
		}
	}
	if (m触摸) {
		if (a消息 == WM_TOUCH) {
			UINT v数量 = LOWORD(w);
			HTOUCHINPUT v触摸 = (HTOUCHINPUT)l;
			std::unique_ptr<TOUCHINPUT[]> va触摸 = std::make_unique<TOUCHINPUT[]>(v数量);
			if (GetTouchInputInfo(v触摸, v数量, va触摸.get(), sizeof(TOUCHINPUT))) {
				m触摸->f接收输入(va触摸.get(), v数量);
			}
			if (!CloseTouchInputHandle(v触摸)) {
				__debugbreak();
			}
		}
	}
}
void C输入::f更新() {
	if (m键盘) {
		m键盘->f更新();
	}
	if (m鼠标) {
		m鼠标->f更新();
	}
	if (m触摸) {
		m触摸->f更新();
	}
}
void C输入::f创建键盘(tp键盘 &a) {
	if (m键盘 == nullptr) {
		m键盘 = std::make_shared<C键盘>();
	}
	a = m键盘;
}
void C输入::f创建鼠标(tp鼠标 &a) {
	if (m鼠标 == nullptr) {
		m鼠标 = std::make_shared<C鼠标>(m窗口, m缩放);
	}
	a = m鼠标;
}
void C输入::f创建触摸(tp触摸 &a) {
	if (m触摸 == nullptr) {
		m触摸 = std::make_shared<C触摸>(m窗口, m缩放);
		RegisterTouchWindow(m窗口, 0);
	}
	a = m触摸;
}
//==============================================================================
// 键盘
//==============================================================================
S按键 C键盘::f按键(t索引 i) const {
	return m按键.f按键(i);
}
void C键盘::f更新() {
	m按键.f覆盖上次();
	m缓冲.f复制到按键(m按键);
}
//==============================================================================
// 鼠标
//==============================================================================
C鼠标::C鼠标(HWND a窗口, float a缩放):
	m窗口(a窗口), m缩放(a缩放) {
}
S按键 C鼠标::f按键(t索引 i) const {
	return m按键.f按键(i);
}
S方向 C鼠标::f坐标() const {
	return m坐标.f这次方向();
}
S方向 C鼠标::f移动() const {
	return m坐标.f方向差();
}
S方向 C鼠标::f滚轮() const {
	return m滚轮.f上次方向();
}
void C鼠标::f更新() {
	//按键
	m按键.f覆盖上次();
	m缓冲.f复制到按键(m按键);
	//坐标&移动
	m坐标.f覆盖上次();
	POINT v点;
	GetCursorPos(&v点);
	视窗::C屏幕坐标到客户区坐标计算 v计算(m窗口);
	m坐标.m这次[0] = v计算.f直角坐标xf(v点.x) / m缩放;
	m坐标.m这次[1] = v计算.f直角坐标yf(v点.y) / m缩放;
	//滚轮
	m滚轮.f覆盖上次();
	m滚轮.f清空这次();
}
//==============================================================================
// 触摸
//==============================================================================
C触摸::C触摸(HWND a窗口, float a缩放):
	m窗口(a窗口), m缩放(a缩放) {
}
void C触摸::f更新() {
	视窗::C屏幕坐标到客户区坐标计算 v计算(m窗口);
	for (const std::shared_ptr<C触控点> &v : ma触控点) {
		//按键
		v->m按键.f覆盖上次();
		v->m缓冲.f复制到按键(v->m按键);
		//坐标
		v->m坐标.f覆盖上次();
		float *v这次 = v->m坐标.m这次;
		v这次[0] = v计算.f直角坐标xf(v->m点x / 100.f) / m缩放;
		v这次[1] = v计算.f直角坐标yf(v->m点y / 100.f) / m缩放;
	}
	//清理失效触控点
	ma触控点.erase(std::remove_if(ma触控点.begin(), ma触控点.end(), [](const std::shared_ptr<C触控点> &a)->bool {return a->m按键.f松开();}), ma触控点.end());
	m新触控点索引 = f触控点数() - m新触控点数;
	m新触控点数 = 0;
}
t数量 C触摸::f触控点数() const {
	return (t数量)ma触控点.size();
}
tp触控点 C触摸::fg触控点(t索引 i) const {
	return ma触控点[i];
}
tp触控点 C触摸::fg新触控点() {
	if (m新触控点索引 < f触控点数()) {
		return fg触控点(m新触控点索引++);
	}
	return nullptr;
}
void C触摸::f接收输入(const TOUCHINPUT *a输入, size_t a数量) {
	for (size_t i = 0; i != a数量; ++i) {
		const TOUCHINPUT &v输入 = a输入[i];
		if (auto v点 = std::find_if(ma触控点.begin(), ma触控点.end(), [&](const std::shared_ptr<C触控点> &a)->bool {return a->m标识 == v输入.dwID; }); v点 != ma触控点.end()) {
			(*v点)->f接收输入(v输入);
		} else {
			ma触控点.push_back(std::make_shared<C触控点>(*this, v输入));
			++m新触控点数;
		}
	}
}
//触控点
C触控点::C触控点(const C触摸 &a触摸, const TOUCHINPUT &a输入):
	m触摸(&a触摸), m标识(a输入.dwID) {
	f接收输入(a输入);
}
S按键 C触控点::f按键() const {
	return m按键;
}
S方向 C触控点::f坐标() const {
	return m坐标.f这次方向();
}
S方向 C触控点::f移动() const {
	return m坐标.f方向差();
}
void C触控点::f接收输入(const TOUCHINPUT &a输入) {
	m点x = a输入.x;
	m点y = a输入.y;
	if (a输入.dwFlags & TOUCHEVENTF_MOVE) {
		m缓冲.f按下();
	} else if (a输入.dwFlags & TOUCHEVENTF_UP) {
		m缓冲.f松开();
	} else if (a输入.dwFlags & TOUCHEVENTF_DOWN) {
		m缓冲.f按下();
	}
}
}	//namespace cflw::输入::win