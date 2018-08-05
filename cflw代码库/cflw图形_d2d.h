#pragma once
//引用
#include <vector>
#include <string>
#include <string_view>
#include <D2D1.h>
#include <DWrite.h>
#include <DXGI.h>
#include <wrl.h>
#include "cflw辅助.h"
#include "cflw数学.h"
#include "cflw数学_图形.h"
#include "cflw数学_平面几何.h"
//链接
#ifdef _WINDOWS
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#endif
//
void GetCurveControlPoints(
	__in const std::vector<D2D1_POINT_2F>& knots,
	__out std::vector<D2D1_POINT_2F>& firstCtrlPt,
	__out std::vector<D2D1_POINT_2F>& secondCtrlPt);
HRESULT CreateBezierSpline(
	__in ID2D1Factory* pD2dFactory,
	__in const std::vector<D2D1_POINT_2F>& points,
	__out ID2D1PathGeometry** ppPathGeometry);
//命名空间
namespace cflw::图形::d2d {
//外部
using Microsoft::WRL::ComPtr;
typedef ComPtr<ID2D1RenderTarget> tp渲染目标;
typedef ComPtr<ID2D1SolidColorBrush> tp纯色画笔;
typedef ComPtr<ID2D1GradientStopCollection> tp渐变点集;
typedef ComPtr<ID2D1LinearGradientBrush> tp线性渐变画笔;
typedef ComPtr<ID2D1RadialGradientBrush> tp径向渐变画笔;
typedef ComPtr<IDWriteTextFormat> tp文本格式;
typedef ComPtr<IDWriteTextLayout> tp文本布局;
typedef ComPtr<ID2D1PathGeometry> tp路径几何;
typedef ComPtr<ID2D1GeometrySink> tp几何槽;
typedef ComPtr<ID2D1Layer> tp层;
typedef D2D1_POINT_2F t点2f;
typedef D2D_RECT_F t矩形f;
typedef D2D1_ELLIPSE t椭圆;
typedef D2D1_COLOR_F t颜色f;
typedef D2D1_ROUNDED_RECT t圆角矩形;
typedef DWRITE_TEXT_RANGE t文本范围;
typedef std::vector<D2D1_GRADIENT_STOP> ta渐变点;
//本文件
class C画图形;
class C画文本;
class C类型转换;
class C修改文本格式;
class C修改文本布局;
class C中文字号格式;
class C坐标转换;
class C修改路径几何;
class C文本工厂;
class C文本效果;
typedef std::shared_ptr<C画图形> tp画图形;
typedef std::shared_ptr<C画文本> tp画文本;
typedef std::shared_ptr<C坐标转换> tp坐标转换;
typedef ComPtr<C文本效果> tp文本效果;
//枚举
enum class E文本水平对齐 {
	e左,
	e右,
	e中,
};
enum class E文本垂直对齐 {
	e上,
	e下,
	e中,
};
//=============================================================================
// 结构
//=============================================================================
struct S渐变点 {
	float m位置;	//m位置∈[0,1]
	数学::S颜色 m颜色;
};

//=============================================================================
// 二维引擎
//=============================================================================
class C二维 {
public:
	static C二维 *g这;
	static const D2D1_COLOR_F c清屏颜色;
//函数
	C二维();
	HRESULT f初始化(HWND, float 缩放 = 0);
	HRESULT f初始化(IDXGISwapChain*, float 缩放 = 0);
	void f初始化_窗口大小(float, float);
	void f初始化_渲染目标(ID2D1RenderTarget*);
	void fs缩放(float = 1);
	//绘制控制
	void f开始();
	void f清屏();
	void f结束();
	//画图对象
	std::shared_ptr<C画图形> fc画图形(const 数学::S颜色 &颜色 = 数学::S颜色::c白, float 宽度 = 1);
	std::shared_ptr<C画文本> fc画文本(const 数学::S颜色 &颜色 = 数学::S颜色::c白);
	ComPtr<ID2D1SolidColorBrush> fc纯色画笔(const 数学::S颜色 &) const;
	ComPtr<ID2D1GradientStopCollection> fc渐变点集(const std::vector<S渐变点> &) const;
	ComPtr<ID2D1LinearGradientBrush> fc线性渐变画笔(const std::vector<S渐变点> &) const;
	ComPtr<ID2D1RadialGradientBrush> fc径向渐变画笔(const std::vector<S渐变点> &) const;
	ComPtr<ID2D1PathGeometry> fc路径几何() const;
	ComPtr<ID2D1Layer> fc层() const;
	C修改路径几何 f修改路径几何(const ComPtr<ID2D1PathGeometry> &) const;
	ComPtr<C文本效果> fc文本效果(const 数学::S颜色 &p描边 = 数学::S颜色::c白, const 数学::S颜色 &p填充 = 数学::S颜色::c黑) const;
	//其它对象
	ComPtr<ID2D1Factory> fg二维工厂();
	ComPtr<ID2D1RenderTarget> fg渲染目标() const;
	C坐标转换 &fg坐标计算() const;
	C文本工厂 &fg文本工厂();
	ComPtr<IDWriteTextFormat> fg默认文本格式();
private:
	数学::S向量2 fg每英寸点数(float 缩放 = 0) const;
	数学::S向量2 m窗口大小;
	ComPtr<ID2D1Factory> m二维工厂;
	ComPtr<ID2D1HwndRenderTarget> m窗口渲染目标;
	ComPtr<ID2D1RenderTarget> m后台渲染目标;
	ComPtr<ID2D1RenderTarget> m渲染目标;
	std::unique_ptr<C坐标转换> m坐标计算;
	std::unique_ptr<C文本工厂> m文本工厂;
	ComPtr<IDWriteTextFormat> m默认文本格式;
};
//各种画
class C画图形 {
public:
	void f初始化(ID2D1RenderTarget *, const C坐标转换 &);
	void f初始化_参数(const 数学::S颜色 &p颜色, float a宽度);
	//设置
	void fs线条宽度(float);
	void fs颜色(const 数学::S颜色 &);
	void fs透明度(float);
	//描线
	void f绘制点(const 数学::S向量2 &);
	void f绘制线条(const 数学::S线段2 &);
	void f绘制矩形(const 数学::S矩形 &);
	void f绘制旋转矩形(const 数学::S旋转矩形 &);
	void f绘制圆形(const 数学::S圆形 &);
	void f绘制椭圆(const 数学::S椭圆 &);
	void f绘制圆角矩形(const 数学::S圆角矩形 &);
	//填充
	void f填充矩形(const 数学::S矩形 &);
	void f填充圆形(const 数学::S圆形 &);
	void f填充椭圆(const 数学::S椭圆 &);
	void f填充圆角矩形(const 数学::S圆角矩形 &);
public:
	ComPtr<ID2D1RenderTarget> m渲染目标;
	ComPtr<ID2D1SolidColorBrush> m画笔;
	const C坐标转换 *m坐标计算;
	float m线条宽度;
};
class C画文本 {
public:
	void f初始化(ID2D1RenderTarget *, const C坐标转换 &);
	void f初始化_参数(const 数学::S颜色 &);
	//设置
	void fs格式(IDWriteTextFormat *);
	void fs样式(C修改文本格式 &);
	void fs颜色(const 数学::S颜色 &);
	void fs透明度(float);
	void fs区域(const 数学::S向量2 &);
	void fs区域(const 数学::S向量2 &, const 数学::S向量2 &);
	void fs区域(float, float);
	void fs区域(float, float, float, float);
	void fs区域(const 数学::S矩形 &);
	//画
	void f绘制文本(const std::wstring_view &) const;
	void f绘制文本布局(IDWriteTextLayout *) const;
	void f绘制文本布局(IDWriteTextLayout *, IDWriteTextRenderer *) const;
	void f绘制文本布局(C修改文本布局 &) const;
public:
	ComPtr<ID2D1RenderTarget> m渲染目标;
	ComPtr<ID2D1SolidColorBrush> m画笔;
	ComPtr<IDWriteTextFormat> m格式;
	const C坐标转换 *m坐标计算;
	D2D1_RECT_F m矩形;
};
//=============================================================================
// 二维结构计算&转换
//=============================================================================
class C类型转换 {
public:
	static D2D1_COLOR_F f颜色(const 数学::S颜色 &);
	static D2D1_POINT_2F f点(const 数学::S向量2 &);
	static std::vector<D2D1_GRADIENT_STOP> f渐变点(const std::vector<S渐变点> &);
};
class C坐标转换 {
public:	//输入直角坐标然后转换成窗口坐标
	void fs大小(const 数学::S向量2 &);
	float x(float) const;
	float y(float) const;
	float f百分比x(float) const;
	float f百分比y(float) const;
	D2D1_POINT_2F f点(const 数学::S向量2 &) const;
	D2D_RECT_F f矩形(const 数学::S矩形 &) const;
	D2D_RECT_F f矩形_窗口() const;
	D2D_RECT_F f矩形_中心半径(const 数学::S向量2 &, const 数学::S向量2 &) const;
	D2D_RECT_F f矩形_左上右下(const 数学::S向量2 &, const 数学::S向量2 &) const;
	D2D_RECT_F f矩形_左上右下(float, float, float, float) const;
	D2D_RECT_F f矩形_点(float, float) const;
	D2D1_ELLIPSE f圆形(const 数学::S向量2 &, float) const;
	D2D1_ELLIPSE f圆形(const 数学::S圆形 &) const;
	D2D1_ELLIPSE f椭圆(const 数学::S向量2 &, const 数学::S向量2 &) const;
	D2D1_ELLIPSE f椭圆(const 数学::S椭圆 &) const;
	D2D1_ROUNDED_RECT f圆角矩形(const 数学::S圆角矩形 &) const;
	D2D1_ROUNDED_RECT f圆角矩形(const 数学::S向量2 &, const 数学::S向量2 &, const 数学::S向量2 &) const;
private:
	数学::S向量2 m窗口大小;
};
//=============================================================================
// 路径
//=============================================================================
class C修改路径几何 {
public:
	C修改路径几何(ID2D1PathGeometry *, const C坐标转换 &);
	~C修改路径几何();
	void f点(const 数学::S向量2 &p);
	void f直线(const 数学::S向量2 &p0, const 数学::S向量2 &p1);
	void f圆弧(const 数学::S向量2 &p圆心, float p半径, float p开始, float p弧度, bool 顺时针 = true);
	void f连续直线(const std::vector<数学::S向量2> &);
	void f平滑曲线(const std::vector<数学::S向量2> &);
	void f闭合();
	void f断开();
	void f结束();
private:
	ComPtr<ID2D1GeometrySink> m几何槽;
	const C坐标转换 *m坐标计算;
	数学::S向量2 m当前点;
	bool mw开始 = false;
};
//=============================================================================
// 文本工厂
//=============================================================================
struct S文本格式参数 {
	std::wstring m字体 = L"微软雅黑";
	std::wstring m语言区域 = L"zh-cn";
	float m字号 = 36;
	DWRITE_FONT_WEIGHT m粗体 = DWRITE_FONT_WEIGHT_REGULAR;
	DWRITE_FONT_STYLE m斜体 = DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_STRETCH m拉伸 = DWRITE_FONT_STRETCH_NORMAL;
	DWRITE_TEXT_ALIGNMENT m水平对齐 = DWRITE_TEXT_ALIGNMENT_LEADING;//默认左对齐
	DWRITE_PARAGRAPH_ALIGNMENT m垂直对齐 = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	DWRITE_WORD_WRAPPING m自动换行 = DWRITE_WORD_WRAPPING_NO_WRAP;
	void fs字体(const std::wstring_view &);
	void fs语言区域(const std::wstring_view &);
	void fs字号(float);
	void fs粗体(int);
	void fs斜体(int);
	void fs拉伸(int);
	void fs自动换行(bool);
	void fs水平对齐(E文本水平对齐);
	void fs垂直对齐(E文本垂直对齐);
};
class C文本工厂 {
public:
//构造函数
	C文本工厂();
	~C文本工厂();
//函数
	HRESULT f初始化();
	void f销毁();
	tp文本格式 fc文本格式(const S文本格式参数 &) const;
	tp文本布局 fc文本布局(const std::wstring_view &文本, IDWriteTextFormat *格式 = nullptr) const;
	tp文本布局 fc文本布局(const std::wstring_view &文本, const S文本格式参数 &格式) const;
	tp文本布局 fc文本布局_小数(double, int 小数位数, IDWriteTextFormat *格式);
	C修改文本布局 f修改文本布局(const tp文本布局 &);
public:
	ComPtr<IDWriteFactory> m写字工厂;
};
class C文本格式转换 {
public:
	static DWRITE_FONT_WEIGHT f粗体(int);	//0~9
	static DWRITE_FONT_STYLE f斜体(int);	//0~2
	static DWRITE_FONT_STRETCH f拉伸(int);	//0~9
	static DWRITE_TEXT_ALIGNMENT f水平对齐(E文本水平对齐);
	static DWRITE_TEXT_ALIGNMENT f水平对齐(int);
	static DWRITE_PARAGRAPH_ALIGNMENT f垂直对齐(E文本垂直对齐);
	static DWRITE_PARAGRAPH_ALIGNMENT f垂直对齐(int);
	static DWRITE_TRIMMING f剪裁(int);	//0~2
	static DWRITE_WORD_WRAPPING f自动换行(bool);
};
//=============================================================================
// 中文字号
//=============================================================================
enum E中文字号 {
	e大特号,e特号,
	e初号,	e小初号,
	e大一号,e一号,	e小一号,
	e二号,	e小二号,
	e三号,	e小三号,
	e四号,	e小四号,
	e五号,	e小五号,
	e六号,	e小六号,
	e七号,	e小七号,
	e八号,
};
constexpr float ca中文字号[] = {
	63, 54,	//特号
	42, 36,	//初号
	31.5, 26, 24,	//一号
	22, 18,	//二号
	16, 15,	//三号
	14, 12,	//四号
	10.5, 9,	//五号
	7.5, 6.5,	//六号
	5.5, 5.25,	//七号
	5	//八号
};
//=============================================================================
// 文本样式&文本布局
//=============================================================================
class C修改文本格式 {
public:
	IDWriteTextFormat *m格式;
	C修改文本格式(IDWriteTextFormat *);
	~C修改文本格式();
	C修改文本格式 &fs水平对齐(E文本水平对齐 = E文本水平对齐::e左);
	C修改文本格式 &fs垂直对齐(E文本垂直对齐 = E文本垂直对齐::e上);
	C修改文本格式 &fs对齐(E文本水平对齐 = E文本水平对齐::e左, E文本垂直对齐 = E文本垂直对齐::e上);
	C修改文本格式 &fs剪裁(int = 0);
};
class C修改文本布局 {
public:
	IDWriteTextLayout *m布局;
	C修改文本布局(IDWriteTextLayout *);
	~C修改文本布局();
	C修改文本布局 &fs粗体(DWRITE_FONT_WEIGHT, DWRITE_TEXT_RANGE = {0, 0});
	C修改文本布局 &fs斜体(DWRITE_FONT_STYLE, DWRITE_TEXT_RANGE = {0, 0});
	C修改文本布局 &fs拉伸(DWRITE_FONT_STRETCH, DWRITE_TEXT_RANGE = {0, 0});
};
class F文本范围计算 {
public:
	UINT32 m文本长度;
	F文本范围计算(UINT32);
	DWRITE_TEXT_RANGE operator()(int = 0, int = 0) const;
};
//=============================================================================
// 画文本
//=============================================================================
class C文本效果 : public IDWriteTextRenderer {
	friend C二维;
	C文本效果() = default;
public:
	//重写的方法
	unsigned long __stdcall AddRef() override;
	unsigned long __stdcall Release() override;
	HRESULT __stdcall QueryInterface(IID const&,void**) override;
	HRESULT __stdcall IsPixelSnappingDisabled(void*, BOOL*) override;
	HRESULT __stdcall GetCurrentTransform(void*, DWRITE_MATRIX*) override;
	HRESULT __stdcall GetPixelsPerDip(void*, FLOAT*) override;
	HRESULT __stdcall DrawGlyphRun(void*, FLOAT, FLOAT, DWRITE_MEASURING_MODE, DWRITE_GLYPH_RUN const*, DWRITE_GLYPH_RUN_DESCRIPTION const*, IUnknown*) override;
	HRESULT __stdcall DrawUnderline(void*, FLOAT, FLOAT, DWRITE_UNDERLINE const*, IUnknown*) override;
	HRESULT __stdcall DrawStrikethrough(void*, FLOAT, FLOAT, DWRITE_STRIKETHROUGH const*,  IUnknown*) override;
	HRESULT __stdcall DrawInlineObject(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override;
	//画文本
	void fs填充颜色(const 数学::S颜色 &);
	void fs轮廓颜色(const 数学::S颜色 &);
	void fs轮廓宽度(float);
	void fs透明度(float);
private:
	HRESULT f绘制几何(ID2D1Geometry *, float x, float y);
	ComPtr<ID2D1Factory> m二维工厂;
	ComPtr<ID2D1RenderTarget> m渲染目标;
	ComPtr<ID2D1SolidColorBrush> m填充画笔;
	ComPtr<ID2D1SolidColorBrush> m轮廓画笔;
	float m轮廓宽度 = 2;
	unsigned long m引用计数 = 1;
};
}	//namespace cflw::图形::d2d