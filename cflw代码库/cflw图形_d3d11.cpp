#include <array>
#include <wrl.h>
#include <D3Dcompiler.h>
#include "cflw图形_d3d11.h"
#include "cflw辅助_win.h"
#include "cflw视窗.h"
#include "cflw图形_d3d纹理.h"
using Microsoft::WRL::ComPtr;
namespace cflw::图形::d3d11 {
//=============================================================================
// 图形引擎
//=============================================================================
C三维::~C三维() {
	f销毁();
}
void C三维::f销毁() {
	//普通对象
	m着色器工厂.reset();
	m缓冲工厂.reset();
	m纹理工厂.reset();
	m渲染状态.reset();
	m渲染控制.reset();
	//com对象
	m交换链.Reset();
	m渲染目标视图.Reset();
	m深度模板.Reset();
	m深度模板视图.Reset();
	m上下文.Reset();
	if (c调试 && m设备) {
		ComPtr<ID3D11Debug> v调试;
		HRESULT hr = m设备->QueryInterface(IID_PPV_ARGS(&v调试));
		if (SUCCEEDED(hr)) {
			hr = v调试->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}
	}
	m设备.Reset();
}
void C三维::f初始化窗口(HWND a) {
	assert(a);
	m窗口 = a;
	视窗::S客户区尺寸 v尺寸 = 视窗::S客户区尺寸::fc窗口(m窗口);
	m窗口大小[0] = v尺寸.fg宽();
	m窗口大小[1] = v尺寸.fg高();
}
HRESULT C三维::f初始化设备() {
	m创建设备 = std::make_unique<C创建设备>();
	//检查兼容性
	m标记[e调试] = c调试;
	HRESULT hr = m创建设备->f检查兼容性();
	if (FAILED(hr)) {
		if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING) {
			m创建设备->fs调试标记(false);
			m标记[e调试] = false;
		} else {
			return hr;
		}
	}
	//枚举适配器
	ComPtr<IDXGIAdapter1> v显卡;
	hr = m创建设备->f取显卡(&v显卡);
	if (hr == DXGI_ERROR_UNSUPPORTED) {//0x887A0004 硬件或驱动不支持
		hr = m创建设备->f创建软件设备(&m设备, &m上下文);
		if (FAILED(hr)) {
			return hr;
		}
		m标记[e软件设备] = true;
	} else {
		hr = m创建设备->f创建设备(v显卡.Get(), &m设备, &m上下文);
		if (FAILED(hr)) {
			return hr;
		}
	}
	//检查抗锯齿等级
	m设备->CheckMultisampleQualityLevels(c交换链格式, 4, &m抗锯齿等级);
	if (m当前抗锯齿等级 > m抗锯齿等级) {
		m当前抗锯齿等级 = m抗锯齿等级;
	}
	//返回
	return S_OK;
}
HRESULT C三维::f初始化交换链() {
	//交换链描述
	DXGI_SWAP_CHAIN_DESC sd;
	辅助::f清零(sd);
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m窗口大小[0];
	sd.BufferDesc.Height = m窗口大小[1];
	sd.BufferDesc.Format = c交换链格式;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m窗口;
	sd.SampleDesc = f计算抗锯齿(m当前抗锯齿等级);
	sd.Windowed = TRUE;
	//创建交换链
	ComPtr<IDXGIFactory1> v基础工厂 = m创建设备->fg工厂();
	HRESULT hr = hr = v基础工厂->CreateSwapChain(m设备.Get(), &sd, &m交换链);
	if(FAILED(hr)) {
		return hr;
	}
	return S_OK;
}
HRESULT C三维::f初始化渲染目标视图() {
	ComPtr<ID3D11Texture2D> v后台缓存;
	HRESULT hr = m交换链->GetBuffer(0, IID_PPV_ARGS(&v后台缓存));
	if(FAILED(hr)) {
		return hr;
	}
	hr = m设备->CreateRenderTargetView(v后台缓存.Get(), nullptr, &m渲染目标视图);
	return S_OK;
}
HRESULT C三维::f初始化深度模板视图() {
	// 创建深度模板
	D3D11_TEXTURE2D_DESC v纹理描述;
	ZeroMemory(&v纹理描述, sizeof(v纹理描述));
	v纹理描述.Width = m窗口大小[0];
	v纹理描述.Height = m窗口大小[1];
	v纹理描述.MipLevels = 1;
	v纹理描述.ArraySize = 1;
	v纹理描述.Format = c深度模板格式;
	v纹理描述.SampleDesc = f计算抗锯齿(m当前抗锯齿等级);
	v纹理描述.Usage = D3D11_USAGE_DEFAULT;
	v纹理描述.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	v纹理描述.CPUAccessFlags = 0;
	v纹理描述.MiscFlags = 0;
	HRESULT hr = m设备->CreateTexture2D(&v纹理描述, nullptr, &m深度模板);
	if(FAILED(hr)) {
		return hr;
	}
	// 创建深度模板视图
	D3D11_DEPTH_STENCIL_VIEW_DESC v深度模板视图描述;
	ZeroMemory(&v深度模板视图描述, sizeof(v深度模板视图描述));
	v深度模板视图描述.Format = v纹理描述.Format;
	v深度模板视图描述.ViewDimension = (m当前抗锯齿等级 > 0) ? D3D11_DSV_DIMENSION_TEXTURE2DMS :  D3D11_DSV_DIMENSION_TEXTURE2D;
	v深度模板视图描述.Texture2D.MipSlice = 0;
	hr = m设备->CreateDepthStencilView(m深度模板.Get(), &v深度模板视图描述, &m深度模板视图);
	if (FAILED(hr)) {
		return hr;
	}
	m上下文->OMSetRenderTargets(1, m渲染目标视图.GetAddressOf(), m深度模板视图.Get());
	return S_OK;
}
//初始化
bool C三维::f初始化(HWND a) {
	try {
		f初始化窗口(a);
		辅助::f失败则抛出(f初始化设备());
		辅助::f失败则抛出(f初始化交换链());
		辅助::f失败则抛出(f初始化渲染目标视图());
		辅助::f失败则抛出(f初始化深度模板视图());
		m上下文->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		f设置视口();
		return true;
	} catch (HRESULT hr) {
		return false;
	}
}
HRESULT C三维::f创建输入布局(tp输入布局 &p输入布局, C顶点格式 &p顶点格式, const void *a数据, size_t a大小) {
	return m设备->CreateInputLayout(p顶点格式.m数组.data(), p顶点格式.m数组.size(), a数据, a大小, &p输入布局);
}
HRESULT C三维::f创建图形管线(tp图形管线 &a, const S图形管线参数 &p参数) {
	C着色器工厂 &v着色器工厂 = fg着色器工厂();
	HRESULT hr;
	std::shared_ptr<C图形管线> v = std::make_shared<C图形管线>();
	if (p参数.m顶点着色器) {
		hr = v着色器工厂.f创建顶点着色器(v->m顶点着色器, p参数.m顶点着色器);
		if (FAILED(hr)) {
			return hr;
		}
		if (p参数.m顶点格式) {
			hr = v着色器工厂.f创建输入布局(v->m输入布局, p参数.m顶点着色器, *p参数.m顶点格式);
			if (FAILED(hr)) {
				return hr;
			}
		}
	}
	if (p参数.m几何着色器) {
		hr = v着色器工厂.f创建几何着色器(v->m几何着色器, p参数.m几何着色器);
		if (FAILED(hr)) {
			return hr;
		}
	}
	if (p参数.m外壳着色器) {
		hr = v着色器工厂.f创建外壳着色器(v->m外壳着色器, p参数.m外壳着色器);
		if (FAILED(hr)) {
			return hr;
		}
	}
	if (p参数.m域着色器) {
		hr = v着色器工厂.f创建域着色器(v->m域着色器, p参数.m域着色器);
		if (FAILED(hr)) {
			return hr;
		}
	}
	if (p参数.m像素着色器) {
		hr = v着色器工厂.f创建像素着色器(v->m像素着色器, p参数.m像素着色器);
		if (FAILED(hr)) {
			return hr;
		}
	}
	v->m光栅化 = p参数.m光栅化;
	v->m混合 = p参数.m混合;
	v->m深度模板 = p参数.m深度模板;
	a = std::move(v);
	return S_OK;
}
void C三维::f设置视口() {
	const D3D11_VIEWPORT &v窗口视口 = fg窗口视口();
	m上下文->RSSetViewports(1, &v窗口视口);
}
//抗锯齿
HRESULT C三维::f设置抗锯齿(UINT a) {
	assert(m设备 != nullptr);
	assert(m抗锯齿等级 >= a);
	HRESULT hr;
	if (m抗锯齿等级 < a) {
		m当前抗锯齿等级 = m抗锯齿等级;
	} else {
		m当前抗锯齿等级 = a;
	}
	if (m交换链) {
		m交换链 = nullptr;
		hr = f初始化交换链();
		if(FAILED(hr)) {
			return hr;
		}
	}
	if (m深度模板) {
		hr = f初始化深度模板视图();
		if(FAILED(hr)) {
			return hr;
		}
	}
	return S_OK;
}
DXGI_SAMPLE_DESC C三维::f计算抗锯齿(UINT a) {
	DXGI_SAMPLE_DESC v;
	v.Count = (a < 1) ? 1 : 4;
	v.Quality = (a < 1) ? 0 : (a - 1);
	return v;
}
C渲染控制 &C三维::fg渲染控制() {
	if (m渲染控制 == nullptr) {
		m渲染控制 = std::make_unique<C渲染控制>();
		m渲染控制->m上下文 = m上下文;
		m渲染控制->m交换链 = m交换链;
		m渲染控制->m渲染目标视图 = m渲染目标视图;
		m渲染控制->m深度模板视图 = m深度模板视图;
	}
	return *m渲染控制;
}
C渲染状态 &C三维::fg渲染状态() {
	if (m渲染状态 == nullptr) {
		m渲染状态 = std::make_unique<C渲染状态>(m设备.Get());
	}
	return *m渲染状态;
}
ComPtr<ID3D11Device> C三维::fg设备() {
	return m设备;
}
ComPtr<ID3D11DeviceContext> C三维::fg上下文() {
	return m上下文;
}
ComPtr<IDXGISwapChain> C三维::fg交换链() {
	return m交换链;
}
D3D11_VIEWPORT C三维::fg窗口视口() const {
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m窗口大小[0];
	vp.Height = (FLOAT)m窗口大小[1];
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	return vp;
}
数学::S向量2 C三维::fg窗口大小() const {
	return {(float)m窗口大小[0], (float)m窗口大小[1]};
}
//工厂
C缓冲工厂 &C三维::fg缓冲工厂() {
	if (m缓冲工厂 == nullptr) {
		m缓冲工厂 = std::make_unique<C缓冲工厂>();
		m缓冲工厂->f初始化(m设备.Get());
	}
	return *m缓冲工厂;
}
C纹理工厂 &C三维::fg纹理工厂() {
	if (m纹理工厂 == nullptr) {
		m纹理工厂 = std::make_unique<C纹理工厂>();
		m纹理工厂->f初始化(m设备.Get());
	}
	return *m纹理工厂;
}
C着色器工厂 &C三维::fg着色器工厂() {
	if (m着色器工厂 == nullptr) {
		m着色器工厂 = std::make_unique<C着色器工厂>();
		m着色器工厂->f初始化(m设备.Get());
	}
	return *m着色器工厂;
}
//=============================================================================
// 创建设备
//=============================================================================
const D3D_FEATURE_LEVEL C创建设备::c功能级别组[3] = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0
};
const UINT C创建设备::c创建标记 = D3D11_CREATE_DEVICE_BGRA_SUPPORT | (c调试 ? D3D11_CREATE_DEVICE_DEBUG : 0);
HRESULT C创建设备::f检查兼容性() {
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, m创建标记, c功能级别组, c功能级别数, D3D11_SDK_VERSION, nullptr, &m功能级别, nullptr);
	return hr;
}
ComPtr<IDXGIFactory1> C创建设备::fg工厂() {
	if (m工厂 == nullptr) {
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m工厂));
		if (FAILED(hr)) {
			return nullptr;
		}
	}
	return m工厂;
}
HRESULT C创建设备::f取显卡(IDXGIAdapter1 **a输出) {
	HRESULT hr = S_OK;
	if (m工厂 == nullptr) {
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&m工厂));
		if (FAILED(hr)) {
			return hr;
		}
	}
	ComPtr<IDXGIAdapter1> v适配器;
	ComPtr<IDXGIAdapter1> v目标显卡;
	D3D_FEATURE_LEVEL v目标等级 = D3D_FEATURE_LEVEL_10_0;
	SIZE_T v显存 = 0;
	for (UINT i = 0; m工厂->EnumAdapters1(i, &v适配器) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC1 v描述;
		v适配器->GetDesc1(&v描述);
		D3D_FEATURE_LEVEL v支持级别;
		hr = D3D11CreateDevice(v适配器.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, m创建标记, c功能级别组, c功能级别数, D3D11_SDK_VERSION, nullptr, &v支持级别, nullptr);
		if (FAILED(hr)) {
			//0x887a002d应用程序请求的操作依赖于已缺失或不匹配的 SDK 组件
			__debugbreak();
			continue;
		}
		if (v描述.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		bool v更好 = false;
		if (v支持级别 > v目标等级) {
			if (v描述.DedicatedVideoMemory > 0) {
				v更好 = true;
			}
		} else if (v支持级别 == v目标等级) {
			if (v描述.DedicatedVideoMemory > v显存) {
				v更好 = true;
			}
		}
		if (v更好) {
			v目标显卡 = v适配器;
			v目标等级 = v支持级别;
			v显存 = v描述.DedicatedVideoMemory;
		}
	}
	*a输出 = v目标显卡.Detach();
	return hr;
}
void C创建设备::fs调试标记(bool a) {
	m创建标记 = 辅助::f位赋值<UINT>(m创建标记, D3D11_CREATE_DEVICE_DEBUG, a);
}
HRESULT C创建设备::f创建设备(IDXGIAdapter1 *p显卡, ID3D11Device **a设备, ID3D11DeviceContext **p上下文) {
	HRESULT hr = D3D11CreateDevice(p显卡, D3D_DRIVER_TYPE_UNKNOWN, nullptr, m创建标记, c功能级别组, c功能级别数, D3D11_SDK_VERSION, a设备, &m功能级别, p上下文);
	return hr;
}
HRESULT C创建设备::f创建软件设备(ID3D11Device **a设备, ID3D11DeviceContext **p上下文) {
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, m创建标记, c功能级别组, c功能级别数, D3D11_SDK_VERSION, a设备, &m功能级别, p上下文);
	return hr;
}
//=============================================================================
// 渲染控制
//=============================================================================
//清屏
C渲染控制::C渲染控制() {
}
void C渲染控制::f清屏() {
	m上下文->ClearRenderTargetView(m渲染目标视图.Get(), m清屏颜色.v);
	if (m深度模板视图) {
		m上下文->ClearDepthStencilView(m深度模板视图.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, m清屏深度, m清屏模板);
	}
}
//显示
void C渲染控制::f显示() {
	m交换链->Present(0, 0);
}
//绘制
void C渲染控制::f绘制(unsigned int p顶点数, unsigned int p开始) {
	m上下文->Draw(p顶点数, p开始);
}
void C渲染控制::f绘制索引(unsigned int p索引数, unsigned int p开始, int p开始顶点) {
	m上下文->DrawIndexed(p索引数, p开始, p开始顶点);
}
void C渲染控制::fs清屏颜色(const 数学::S颜色 &a) {
	m清屏颜色 = a;
}
void C渲染控制::fs清屏深度(float a) {
	m清屏深度 = a;
}
void C渲染控制::fs清屏模板(UINT8 a) {
	m清屏模板 = a;
}
void C渲染控制::fs图形管线(const C图形管线 &a) {
	fs顶点着色器(a.m顶点着色器.Get());
	fs外壳着色器(a.m外壳着色器.Get());
	fs域着色器(a.m域着色器.Get());
	fs几何着色器(a.m几何着色器.Get());
	fs像素着色器(a.m像素着色器.Get());
	fs输入布局(a.m输入布局.Get());
	fs混合(a.m混合.Get());
	fs深度模板(a.m深度模板.Get());
	fs光栅化(a.m光栅化.Get());
}
//设置着色器
void C渲染控制::f重置着色器() {
	m上下文->VSSetShader(nullptr, nullptr, 0);
	m上下文->HSSetShader(nullptr, nullptr, 0);
	m上下文->DSSetShader(nullptr, nullptr, 0);
	m上下文->GSSetShader(nullptr, nullptr, 0);
	m上下文->PSSetShader(nullptr, nullptr, 0);
	m上下文->CSSetShader(nullptr, nullptr, 0);
}
void C渲染控制::fs顶点着色器(ID3D11VertexShader *a) {
	m上下文->VSSetShader(a, nullptr, 0);
}
void C渲染控制::fs像素着色器(ID3D11PixelShader *a) {
	m上下文->PSSetShader(a, nullptr, 0);
}
void C渲染控制::fs几何着色器(ID3D11GeometryShader *a) {
	m上下文->GSSetShader(a, nullptr, 0);
}
void C渲染控制::fs外壳着色器(ID3D11HullShader *a) {
	m上下文->HSSetShader(a, nullptr, 0);
}
void C渲染控制::fs域着色器(ID3D11DomainShader *a) {
	m上下文->DSSetShader(a, nullptr, 0);
}
//状态
void C渲染控制::fs混合(ID3D11BlendState *p混合, const 数学::S颜色 &p颜色, UINT p掩码) {
	m上下文->OMSetBlendState(p混合, (float*)&p颜色, p掩码);	//32个采样点都有效
}
void C渲染控制::fs深度模板(ID3D11DepthStencilState *p深度模板, UINT p参考) {
	m上下文->OMSetDepthStencilState(p深度模板, p参考);
}
void C渲染控制::fs光栅化(ID3D11RasterizerState *p光栅化) {
	m上下文->RSSetState(p光栅化);
}
void C渲染控制::fs顶点缓冲(ID3D11Buffer *a, UINT p单位大小) {
	UINT v偏移 = 0;
	m上下文->IASetVertexBuffers(0, 1, &a, &p单位大小, &v偏移);
}
void C渲染控制::fs索引缓冲(ID3D11Buffer *a) {
	m上下文->IASetIndexBuffer(a, DXGI_FORMAT_R16_UINT, 0);
}
void C渲染控制::fs固定缓冲(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->VSSetConstantBuffers(p位置, 1, &p缓存);
	m上下文->PSSetConstantBuffers(p位置, 1, &p缓存);
	m上下文->HSSetConstantBuffers(p位置, 1, &p缓存);
	m上下文->DSSetConstantBuffers(p位置, 1, &p缓存);
	m上下文->GSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs固定缓冲v(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->VSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs固定缓冲p(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->PSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs固定缓冲g(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->GSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs固定缓冲h(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->HSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs固定缓冲d(UINT p位置, ID3D11Buffer *p缓存) {
	m上下文->DSSetConstantBuffers(p位置, 1, &p缓存);
}
void C渲染控制::fs纹理(UINT p位置, ID3D11ShaderResourceView *p纹理) {
	m上下文->PSSetShaderResources(p位置, 1, &p纹理);
}
void C渲染控制::fs采样器(UINT p位置, ID3D11SamplerState *p采样器) {
	m上下文->PSSetSamplers(p位置, 1, &p采样器);
}
void C渲染控制::fs输入布局(ID3D11InputLayout *p布局) {
	m上下文->IASetInputLayout(p布局);
}
void C渲染控制::fs图元拓扑(E图元拓扑 p拓扑) {
	m上下文->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)p拓扑);
}
void C渲染控制::f更新资源(ID3D11Buffer *p缓存, const void *p资源) {
	assert(p缓存);
	m上下文->UpdateSubresource(p缓存, 0, nullptr, p资源, 0, 0);
}

//=============================================================================
// 图形管线
//=============================================================================
C自动缓冲::C自动缓冲(C三维 &a三维) : 
	m上下文(a三维.fg上下文().Get()),
	m缓冲工厂(&a三维.fg缓冲工厂()),
	m顶点缓冲(*m上下文, *m缓冲工厂, E缓冲::e顶点),
	m索引缓冲(*m上下文, *m缓冲工厂, E缓冲::e索引) {
}
bool C自动缓冲::f刷新() {
	if (m顶点缓冲.m修改 == 0 && m索引缓冲.m修改 == 0) {
		return false;
	}
	const bool v取消映射顶点 = m顶点缓冲.f取消映射();
	const bool v取消映射索引 = m索引缓冲.f取消映射();
	const bool v刷新 = v取消映射顶点 || v取消映射索引;
	if (v刷新) {
		if (mf刷新回调) {
			mf刷新回调();
		}
		UINT v顶点偏移 = 0;
		m上下文->IASetVertexBuffers(0, 1, m顶点缓冲.m缓冲.GetAddressOf(), &m顶点缓冲.m单位大小, &v顶点偏移);
		m上下文->IASetIndexBuffer(m索引缓冲.m缓冲.Get(), DXGI_FORMAT_R16_UINT, 0);
		//↓如果出现EXECUTION WARNING #355: DEVICE_DRAW_VERTEX_BUFFER_STRIDE_TOO_SMALL，检查初始化时设置的顶点缓冲单位大小
		m上下文->DrawIndexed(m索引缓冲.m修改 / m索引缓冲.m单位大小, 0, 0);
		m顶点缓冲.m修改 = 0;
		m索引缓冲.m修改 = 0;
	}
	return v刷新;
}
void C自动缓冲::f复制(const void *a顶点, size_t a顶点大小, const void *a索引, size_t a索引大小) {
	const unsigned int v顶点目标 = m顶点缓冲.m修改 + a顶点大小;
	const unsigned int v索引目标 = m索引缓冲.m修改 + a索引大小;
	if ((v顶点目标 > m顶点缓冲.m缓冲大小) || (v索引目标 > m索引缓冲.m缓冲大小)) {
		f刷新();
	} else {
		const unsigned int v顶点数量 = m顶点缓冲.fg数量();
		if (v顶点数量 != 0) {
			const unsigned int v索引数量 = a索引大小 / sizeof(t索引);
			std::vector<t索引> va索引(v索引数量);
			const t索引 *vp索引 = (t索引*)a索引;
			for (unsigned int i = 0; i != v索引数量; ++i) {
				va索引[i] = vp索引[i] + (t索引)v顶点数量;
			}
			m顶点缓冲.f复制(a顶点, a顶点大小);
			m索引缓冲.f复制(va索引.data(), a索引大小);
			return;
		}
	}
	m顶点缓冲.f复制(a顶点, a顶点大小);
	m索引缓冲.f复制(a索引, a索引大小);
}
bool C自动缓冲::fw修改() const {
	return m顶点缓冲.m修改 != 0 || m索引缓冲.m修改 != 0;
}
//缓存
C自动缓冲::C缓冲::C缓冲(ID3D11DeviceContext &a上下文, C缓冲工厂 &a缓冲工厂, E缓冲 a标志) :
	m上下文(&a上下文), m缓冲工厂(&a缓冲工厂), m标记(a标志) {
}
void C自动缓冲::C缓冲::f初始化(size_t a单位大小, size_t a数量) {
	m单位大小 = a单位大小;
	m缓冲大小 = m单位大小 * a数量;
	m缓冲工厂->f创建缓冲(m缓冲, nullptr, m缓冲大小, m标记, e动态);
	m修改 = 0;
}
bool C自动缓冲::C缓冲::f映射() {
	assert(m缓冲 != nullptr);
	if (m映射 != nullptr) {
		return false;
	}
	D3D11_MAPPED_SUBRESOURCE v映射资源;
	m上下文->Map(m缓冲.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &v映射资源);
	m映射 = (std::byte*)v映射资源.pData;
	return true;
}
bool C自动缓冲::C缓冲::f取消映射() {
	if (m映射 == nullptr) {
		return false;
	}
	m上下文->Unmap(m缓冲.Get(), 0);
	m映射 = nullptr;
	return true;
}
bool C自动缓冲::C缓冲::f复制(const void *p指针, size_t a大小) {
	if (m映射 == nullptr) {
		f映射();
	}
	const unsigned int v目标 = m修改 + a大小;
	const bool v可用 = v目标 <= m缓冲大小;
	assert(v可用);
	if (v可用) {
		memcpy(m映射 + m修改, p指针, a大小);
		m修改 = v目标;
	}
	return v可用;
}
UINT C自动缓冲::C缓冲::fg数量() {
	return m修改 / m单位大小;
}
//=============================================================================
// 渲染状态
//=============================================================================
C渲染状态::~C渲染状态() {
}
C渲染状态::C渲染状态(ID3D11Device *a设备) {
	HRESULT hr;
	//无光栅化
	m光栅化参数.m默认 = {D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP, D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, true, FALSE};
	m光栅化.m默认 = nullptr;
	//线框渲染
	m光栅化参数.m线框渲染 = m光栅化参数.m默认;
	m光栅化参数.m线框渲染.CullMode = D3D11_CULL_BACK;
	m光栅化参数.m线框渲染.FillMode = D3D11_FILL_WIREFRAME;
	hr = a设备->CreateRasterizerState(&m光栅化参数.m线框渲染, &m光栅化.m线框渲染);
	if(FAILED(hr)) {
		return;
	}
	//取消隐藏面消除
	m光栅化参数.m显示隐藏面 = m光栅化参数.m默认;
	m光栅化参数.m显示隐藏面.CullMode = D3D11_CULL_NONE;
	m光栅化参数.m显示隐藏面.FillMode = D3D11_FILL_SOLID;
	hr = a设备->CreateRasterizerState(&m光栅化参数.m显示隐藏面, &m光栅化.m显示隐藏面);
	if(FAILED(hr)) {
		return;
	}
	//设置逆时针为正面
	m光栅化参数.m反面渲染 = m光栅化参数.m默认;
	m光栅化参数.m反面渲染.FrontCounterClockwise = true;
	hr = a设备->CreateRasterizerState(&m光栅化参数.m反面渲染, &m光栅化.m反面渲染);
	if(FAILED(hr)) {
		return;
	}
	//无混合
	m混合参数.m默认 = {FALSE, FALSE, {
		{FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL}, {0}, {0}, {0}, {0}, {0}, {0}, {0}
	}};
	m混合.m默认 = nullptr;
	//开启透明
	m混合参数.m开启透明 = m混合参数.m默认;
	m混合参数.m开启透明.RenderTarget[0].BlendEnable = true;
	m混合参数.m开启透明.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m混合参数.m开启透明.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m混合参数.m开启透明.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m混合参数.m开启透明.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m混合参数.m开启透明.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	m混合参数.m开启透明.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m混合参数.m开启透明.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = a设备->CreateBlendState(&m混合参数.m开启透明, &m混合.m开启透明);
	if(FAILED(hr)) {
		return;
	}
	//颜色叠加
	m混合参数.m颜色叠加 = m混合参数.m开启透明;
	m混合参数.m颜色叠加.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	hr = a设备->CreateBlendState(&m混合参数.m颜色叠加, &m混合.m颜色叠加);
	if(FAILED(hr)) {
		return;
	}
	//禁止写颜色
	m混合参数.m禁止写颜色 = m混合参数.m默认;
	m混合参数.m禁止写颜色.RenderTarget[0].BlendEnable = false;
	m混合参数.m禁止写颜色.RenderTarget[0].RenderTargetWriteMask = 0;
	hr = a设备->CreateBlendState(&m混合参数.m禁止写颜色, &m混合.m禁止写颜色);
	if(FAILED(hr)) {
		return;
	}
	//无深度模板
	m深度模板参数.m默认 = {FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK, {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS}, {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS}};
	m深度模板.m默认 = nullptr;
	//正常深度
	m深度模板参数.m正常深度l = m深度模板参数.m默认;
	m深度模板参数.m正常深度l.DepthEnable = true;
	m深度模板参数.m正常深度l.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m深度模板参数.m正常深度l.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	m深度模板参数.m正常深度l.StencilEnable = false;
	hr = a设备->CreateDepthStencilState(&m深度模板参数.m正常深度l, &m深度模板.m正常深度l);
	if(FAILED(hr)) {
		return;
	}
	m深度模板参数.m正常深度r = m深度模板参数.m正常深度l;
	m深度模板参数.m正常深度r.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	hr = a设备->CreateDepthStencilState(&m深度模板参数.m正常深度r, &m深度模板.m正常深度r);
	if (FAILED(hr)) {
		return;
	}
	m深度模板参数.m总是覆盖 = m深度模板参数.m正常深度l;
	m深度模板参数.m总是覆盖.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = a设备->CreateDepthStencilState(&m深度模板参数.m总是覆盖, &m深度模板.m总是覆盖);
	if (FAILED(hr)) {
		return;
	}
	//纹理
	m采样器参数.m纹理.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	m采样器参数.m纹理.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	m采样器参数.m纹理.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	m采样器参数.m纹理.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	m采样器参数.m纹理.ComparisonFunc = D3D11_COMPARISON_NEVER;
	m采样器参数.m纹理.BorderColor[0] = 0;
	m采样器参数.m纹理.BorderColor[1] = 0;
	m采样器参数.m纹理.BorderColor[2] = 0;
	m采样器参数.m纹理.BorderColor[3] = 0;
	m采样器参数.m纹理.MinLOD = 0;
	m采样器参数.m纹理.MaxLOD = D3D11_FLOAT32_MAX;
	m采样器参数.m纹理.MipLODBias = 0;
	m采样器参数.m纹理.MaxAnisotropy = 16;
	hr = a设备->CreateSamplerState(&m采样器参数.m纹理, &m采样器.m纹理);
	if(FAILED(hr)) {
		return;
	}
	//图案
	m采样器参数.m图案 = m采样器参数.m纹理;
	m采样器参数.m图案.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	m采样器参数.m图案.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	m采样器参数.m图案.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = a设备->CreateSamplerState(&m采样器参数.m图案, &m采样器.m图案);
	if(FAILED(hr)) {
		return;
	}
	//各向异性过滤
	m采样器参数.m各向异性过滤 = m采样器参数.m纹理;
	m采样器参数.m各向异性过滤.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = a设备->CreateSamplerState(&m采样器参数.m各向异性过滤, &m采样器.m各向异性过滤);
	if (FAILED(hr)) {
		return;
	}
}
//=============================================================================
// 顶点格式
//=============================================================================
void C顶点格式::f清空() {
	m数组.clear();
	m类型累计.clear();
	m字节累计 = 0;
}
void C顶点格式::f添加(E类型 a类型, int a大小) {
	static const char *const ca语义[] = {
		"POSITION",
		"NORMAL",
		"BINORMAL",
		"COLOR",
		"TEXCOORD",
		"PSIZE",
		"TANGENT"
	};
	f添加(ca语义[a类型], a大小);
}
void C顶点格式::f添加(const char *a语义, int a大小) {
	static const DXGI_FORMAT v格式[] = {
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	D3D11_INPUT_ELEMENT_DESC v;
	v.AlignedByteOffset = m字节累计;
	v.Format = v格式[a大小-1];
	m字节累计 += a大小 * 4;
	v.SemanticName = a语义;
	auto &v语义索引 = m类型累计[a语义];
	v.SemanticIndex = v语义索引;
	v语义索引 += 1;
	v.InputSlot = 0;
	v.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	v.InstanceDataStepRate = 0;
	m数组.push_back(v);
}
//=============================================================================
// 缓冲工厂
//=============================================================================
void C缓冲工厂::f初始化(ID3D11Device *a设备) {
	m设备 = a设备;
}
HRESULT C缓冲工厂::f创建缓冲(tp缓冲 &a输出, const void *a数据, UINT a大小, E缓冲 a标志, E资源用法 a用法) {
	D3D11_BUFFER_DESC v描述 = {};
	v描述.Usage = (D3D11_USAGE)a用法;
	switch (a用法) {
	case e暂存:
		v描述.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	case e动态:
		v描述.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}
	//assert(a大小 % 16 == 0);
	v描述.ByteWidth = (a大小 % 16 != 0) ? ((a大小 / 16 + 1) * 16) : a大小;	//必需是16的倍数
	v描述.BindFlags = a标志;
	HRESULT hr;
	if (a数据) {
		D3D11_SUBRESOURCE_DATA v数据 = {};
		v数据.pSysMem = a数据;
		hr = m设备->CreateBuffer(&v描述, &v数据, &a输出);
	} else {
		hr = m设备->CreateBuffer(&v描述, nullptr, &a输出);
	}
	return hr;
}
//=============================================================================
// 着色器工厂
//=============================================================================
void C着色器工厂::f初始化(ID3D11Device *a设备) {
	m设备 = a设备;
}
//编译着色器
HRESULT C着色器工厂::f编译并创建顶点着色器(tp顶点着色器 &a, const wchar_t *a文件, const char *a入口, tp输入布局 &p输入布局, const C顶点格式 &p顶点格式) {
	ComPtr<ID3DBlob> vs;
	HRESULT hr = f编译顶点着色器(vs, a文件, a入口);
	if (FAILED(hr)) {
		return hr;
	}
	// 创建顶点着色器
	tp顶点着色器 v新着色器;
	const S着色器字节代码 v代码 = S着色器字节代码::fc二进制大对象(vs.Get());
	hr = f创建顶点着色器(v新着色器, v代码);
	if (FAILED(hr)) {
		return hr;
	}
	// 创建输入布局
	hr = f创建输入布局(p输入布局, v代码, p顶点格式);
	if (FAILED(hr)) {
		return hr;
	}
	a = std::move(v新着色器);
	return S_OK;
}
HRESULT C着色器工厂::f编译并创建像素着色器(tp像素着色器 &a, const wchar_t *a文件, const char *a入口) {
	HRESULT hr;
	ComPtr<ID3DBlob> ps;
	hr = f编译像素着色器(ps, a文件, a入口);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建像素着色器(a, S着色器字节代码::fc二进制大对象(ps.Get()));
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f编译并创建几何着色器(tp几何着色器 &a, const wchar_t *a文件, const char *a入口) {
	HRESULT hr;
	ComPtr<ID3DBlob> gs;
	hr = f编译几何着色器(gs, a文件, a入口);
	if (FAILED(hr)) {
		return hr;
	}
	// 创建几何着色器
	hr = f创建几何着色器(a, S着色器字节代码::fc二进制大对象(gs.Get()));
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f编译并创建外壳着色器(tp外壳着色器 &a, const wchar_t *a文件, const char *a入口) {
	HRESULT hr;
	ComPtr<ID3DBlob> hs;
	hr = f编译外壳着色器(hs, a文件, a入口);
	if (FAILED(hr)) {
		return hr;
	}
	// 创建外壳着色器
	hr = f创建外壳着色器(a, S着色器字节代码::fc二进制大对象(hs.Get()));
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f编译并创建域着色器(tp域着色器 &a, const wchar_t *a文件, const char *a入口) {
	HRESULT hr;
	ComPtr<ID3DBlob> ds;
	hr = f编译域着色器(ds, a文件, a入口);
	if (FAILED(hr)) {
		return hr;
	}
	// 创建域着色器
	hr = f创建域着色器(a, S着色器字节代码::fc二进制大对象(ds.Get()));
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
//读取着色器
HRESULT C着色器工厂::f读取并创建顶点着色器(tp顶点着色器 &a, const wchar_t *a文件名, tp输入布局 &p输入布局, const C顶点格式 &p顶点格式) {
	HRESULT hr;
	std::unique_ptr<std::byte[]> v数据;
	DWORD v大小;
	hr = f读取着色器(a文件名, v数据, v大小);
	if (FAILED(hr)) {
		return hr;
	}
	tp顶点着色器 v新着色器;
	const S着色器字节代码 v代码 = {v数据.get(), v大小};
	hr = f创建顶点着色器(v新着色器, v代码);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建输入布局(p输入布局, v代码, p顶点格式);
	if(FAILED(hr)) {
		return hr;
	}
	a = v新着色器;
	return S_OK;
}
HRESULT C着色器工厂::f读取并创建像素着色器(tp像素着色器 &a, const wchar_t *a文件名) {
	HRESULT hr;
	std::unique_ptr<std::byte[]> v数据;
	DWORD v大小;
	hr = f读取着色器(a文件名, v数据, v大小);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建像素着色器(a, {v数据.get(), v大小});
	if (FAILED(hr)) {
		return hr;
	}
	return S_OK;
}
HRESULT C着色器工厂::f读取并创建几何着色器(tp几何着色器 &a, const wchar_t *a文件名) {
	HRESULT hr;
	std::unique_ptr<std::byte[]> v数据;
	DWORD v大小;
	hr = f读取着色器(a文件名, v数据, v大小);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建几何着色器(a, {v数据.get(), v大小});
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f读取并创建外壳着色器(tp外壳着色器 &a, const wchar_t *a文件名) {
	HRESULT hr;
	std::unique_ptr<std::byte[]> v数据;
	DWORD v大小;
	hr = f读取着色器(a文件名, v数据, v大小);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建外壳着色器(a, {v数据.get(), v大小});
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f读取并创建域着色器(tp域着色器 &a, const wchar_t *a文件名) {
	HRESULT hr;
	std::unique_ptr<std::byte[]> v数据;
	DWORD v大小;
	hr = f读取着色器(a文件名, v数据, v大小);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建域着色器(a, {v数据.get(), v大小});
	if (FAILED(hr)) {
		return hr;
	}
	return hr;
}
HRESULT C着色器工厂::f创建顶点着色器(tp顶点着色器 &a, const S着色器字节代码 &p代码) {
	return m设备->CreateVertexShader(p代码.m数据, p代码.m大小, nullptr, &a);
}
HRESULT C着色器工厂::f创建像素着色器(tp像素着色器 &a, const S着色器字节代码 &p代码) {
	return m设备->CreatePixelShader(p代码.m数据, p代码.m大小, nullptr, &a);
}
HRESULT C着色器工厂::f创建几何着色器(tp几何着色器 &a, const S着色器字节代码 &p代码) {
	return m设备->CreateGeometryShader(p代码.m数据, p代码.m大小, nullptr, &a);
}
HRESULT C着色器工厂::f创建外壳着色器(tp外壳着色器 &a, const S着色器字节代码 &p代码) {
	return m设备->CreateHullShader(p代码.m数据, p代码.m大小, nullptr, &a);
}
HRESULT C着色器工厂::f创建域着色器(tp域着色器 &a, const S着色器字节代码 &p代码) {
	return m设备->CreateDomainShader(p代码.m数据, p代码.m大小, nullptr, &a);
}
HRESULT C着色器工厂::f创建输入布局(tp输入布局 &a, const S着色器字节代码 &p代码, const C顶点格式 &p顶点格式) {
	return m设备->CreateInputLayout(p顶点格式.m数组.data(), p顶点格式.m数组.size(), p代码.m数据, p代码.m大小, &a);
}
HRESULT C着色器工厂::f编译顶点着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名, const char *a函数名) {
	return f编译着色器(a文件名, a函数名, "vs_5_0", &a);
}
HRESULT C着色器工厂::f编译像素着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名, const char *a函数名) {
	return f编译着色器(a文件名, a函数名, "ps_5_0", &a);
}
HRESULT C着色器工厂::f编译几何着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名, const char *a函数名) {
	return f编译着色器(a文件名, a函数名, "gs_5_0", &a);
}
HRESULT C着色器工厂::f编译外壳着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名, const char *a函数名) {
	return f编译着色器(a文件名, a函数名, "hs_5_0", &a);
}
HRESULT C着色器工厂::f编译域着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名, const char *a函数名) {
	return f编译着色器(a文件名, a函数名, "ds_5_0", &a);
}
HRESULT C着色器工厂::f编译着色器(const wchar_t *a文件名, const char *a入口, const char *a着色模型, ID3DBlob** a输出) {
	HRESULT hr = S_OK;
	DWORD v着色标记 = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	v着色标记 |= D3DCOMPILE_DEBUG;
#endif
	ComPtr<ID3DBlob> v错误;
	hr = D3DCompileFromFile(a文件名, nullptr, nullptr, a入口, a着色模型, v着色标记, 0, a输出, &v错误);
	if(FAILED(hr)) {
		if (v错误 != nullptr) {
			OutputDebugStringA((char*)v错误->GetBufferPointer());
		}
		return hr;
	}
	return S_OK;
}
HRESULT C着色器工厂::f读取着色器(const wchar_t *a文件名, std::unique_ptr<std::byte[]> &a数据, DWORD &a大小) {
	HANDLE v文件 = CreateFileW(a文件名, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
	if (v文件 == INVALID_HANDLE_VALUE) {
		return ERROR_FILE_NOT_FOUND;
	}
	a大小 = GetFileSize(v文件, nullptr);
	a数据 = std::make_unique<std::byte[]>(a大小);
	DWORD v读大小;
	ReadFile(v文件, a数据.get(), a大小, &v读大小, nullptr);
	CloseHandle(v文件);
	return S_OK;
}
HRESULT C着色器工厂::f读取着色器(ComPtr<ID3DBlob> &a, const wchar_t *a文件名) {
	ComPtr<ID3DBlob> v;
	HANDLE v文件 = CreateFileW(a文件名, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
	if (v文件 == INVALID_HANDLE_VALUE) {
		return ERROR_FILE_NOT_FOUND;
	}
	const DWORD v大小 = GetFileSize(v文件, nullptr);
	D3DCreateBlob(v大小, &v);
	ReadFile(v文件, v->GetBufferPointer(), v大小, nullptr, nullptr);
	CloseHandle(v文件);
	a = std::move(v);
	return S_OK;
}
//=============================================================================
// 纹理加载器
//=============================================================================
C纹理工厂::~C纹理工厂() {
	m工厂.reset();
}
HRESULT C纹理工厂::f初始化(ID3D11Device *a设备) {
	if (m工厂) {
		return S_OK;
	}
	m设备 = a设备;
	m工厂 = std::make_unique<纹理::C图像工厂>();
	return m工厂->f初始化();
}
//创建纹理
HRESULT C纹理工厂::f从文件创建纹理(tp纹理 &a输出, const wchar_t *a文件) {
	HRESULT hr;
	ComPtr<IWICBitmapFrameDecode> v图像;
	hr = m工厂->f读取图像(a文件, &v图像);
	if (FAILED(hr)) {
		return hr;
	}
	//格式转换并复制数据
	const WICBitmapPlaneDescription v描述 = m工厂->f取图像描述(v图像.Get());
	size_t v行距, v图像大小;
	std::unique_ptr<std::byte[]> v数据;
	GUID v图像格式 = v描述.Format;
	if (纹理::格式::fw通用格式(v图像格式)) {
		v数据 = m工厂->f复制像素数据(v图像.Get(), nullptr, &v行距, &v图像大小);
	} else {
		ComPtr<IWICFormatConverter> v格式转换;
		v图像格式 = 纹理::格式::f到通用格式(v图像格式);
		m工厂->f图像格式变换(v图像.Get(), v图像格式, &v格式转换);
		v数据 = m工厂->f复制像素数据(v格式转换.Get(), nullptr, &v行距, &v图像大小);
	}
	//创建纹理
	ComPtr<ID3D11Texture2D> v纹理;
	const DXGI_FORMAT v纹理格式 = 纹理::格式::f到dxgi(v图像格式);
	hr = f从内存创建纹理2(v纹理, v数据.get(), v纹理格式, v描述.Width, v描述.Height, v行距, v图像大小);
	if (FAILED(hr)) {
		return hr;
	}
	hr = f创建纹理资源视图(a输出, v纹理.Get(), v纹理格式);
	return hr;
}
HRESULT C纹理工厂::f从内存创建纹理2(tp纹理2 &a输出, const void *a数据, DXGI_FORMAT a格式, UINT a宽, UINT a高, UINT a行距, UINT a大小) {
	m纹理描述.Width = a宽;
	m纹理描述.Height = a高;
	m纹理描述.MipLevels = 1;
	m纹理描述.ArraySize = 1;
	m纹理描述.Format = a格式;
	m纹理描述.SampleDesc.Count = 1;
	m纹理描述.SampleDesc.Quality = 0;
	m纹理描述.Usage = D3D11_USAGE_DEFAULT;
	m纹理描述.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	m纹理描述.CPUAccessFlags = 0;
	m纹理描述.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA v资源描述;
	v资源描述.pSysMem = a数据;
	v资源描述.SysMemPitch = a行距;
	v资源描述.SysMemSlicePitch = a大小;
	HRESULT hr = m设备->CreateTexture2D(&m纹理描述, &v资源描述, &a输出);
	return hr;
}
HRESULT C纹理工厂::f从纹理对象创建纹理2(tp纹理2 &a输出, const 纹理::I纹理 &a纹理) {
	return f从内存创建纹理2(a输出, a纹理.fg数据(), a纹理.fg格式(), a纹理.fg宽(), a纹理.fg高(), a纹理.fg行距(), a纹理.fg图像大小());
}
HRESULT C纹理工厂::f创建纹理资源视图(tp纹理 &a输出, tp纹理2 a纹理, DXGI_FORMAT a格式) {
	D3D11_SHADER_RESOURCE_VIEW_DESC v描述 = {};
	v描述.Format = a格式;
	v描述.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	v描述.Texture2D.MipLevels = 1;
	HRESULT hr = m设备->CreateShaderResourceView(a纹理.Get(), &v描述, &a输出);
	return hr;
}
const D3D11_TEXTURE2D_DESC &C纹理工厂::fg最近纹理描述() const {
	return m纹理描述;
}
//=============================================================================
// 结构
//=============================================================================
S着色器字节代码 S着色器字节代码::fc二进制大对象(ID3DBlob *a) {
	return {a->GetBufferPointer(), a->GetBufferSize()};
}
S着色器字节代码::operator bool() const {
	return m数据 != nullptr;
}
void S图形管线参数::fs输入布局(const C顶点格式 &a) {
	m顶点格式 = &a;
}
void S图形管线参数::fs顶点着色器(const S着色器字节代码 &a) {
	m顶点着色器 = a;
}
void S图形管线参数::fs像素着色器(const S着色器字节代码 &a) {
	m像素着色器 = a;
}
void S图形管线参数::fs几何着色器(const S着色器字节代码 &a) {
	m几何着色器 = a;
}
void S图形管线参数::fs外壳着色器(const S着色器字节代码 &a) {
	m外壳着色器 = a;
}
void S图形管线参数::fs域着色器(const S着色器字节代码 &a) {
	m域着色器 = a;
}
void S图形管线参数::fs光栅化(ID3D11RasterizerState *a) {
	m光栅化 = a;
}
void S图形管线参数::fs混合(ID3D11BlendState *a) {
	m混合 = a;
}
void S图形管线参数::fs深度模板(ID3D11DepthStencilState *a) {
	m深度模板 = a;
}
}	//namespace cflw::图形::d3d11