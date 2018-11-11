﻿#pragma once
#include <memory>
#include <vector>
#include <map>
#include <bitset>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl.h>
#include "cflw数学_图形.h"
#include "cflw图形_d3d纹理.h"
#ifdef _WINDOWS
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#endif

namespace cflw::图形::d3d12 {
using Microsoft::WRL::ComPtr;
typedef ComPtr<IDXGIFactory4> tp基础工厂;
typedef ComPtr<IDXGIAdapter3> tp显卡;
typedef ComPtr<IDXGISwapChain3> tp交换链;
typedef ComPtr<ID3D12Device> tp设备;
typedef ComPtr<ID3D12PipelineState> tp图形管线;
typedef ComPtr<ID3D12GraphicsCommandList> tp命令列表;
typedef ComPtr<ID3D12Resource> tp资源;
typedef ComPtr<ID3DBlob> tp着色器;
typedef ComPtr<ID3D12RootSignature> tp根签名;
typedef ComPtr<ID3D12DescriptorHeap> tp描述符堆;
class C创建设备;
class C渲染控制;
class C渲染状态;
class C缓冲工厂;
class C纹理工厂;
class C渲染目标管理;
class C深度模板管理;
class C顶点格式;
class C根签名参数;
class C顶点缓冲;
class C索引缓冲;
class C固定缓冲;
class C纹理资源;
using tp顶点 = std::shared_ptr<C顶点缓冲>;
using tp索引 = std::shared_ptr<C索引缓冲>;
using tp固定 = std::shared_ptr<C固定缓冲>;
using tp纹理 = std::shared_ptr<C纹理资源>;
//==============================================================================
// 常量&标志
//==============================================================================
enum class E图元拓扑 {
	e列表点 = D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
	e列表线段 = D3D_PRIMITIVE_TOPOLOGY_LINELIST,
	e连续线段 = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
	e列表三角形 = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	e连续三角形 = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};
enum class E图元拓扑类型 {
	e点 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
	e线段 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
	e三角形 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	e补丁 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
};
enum class E着色器 {
	e无 = 0,
	e全部 = 0,
	e顶点着色器,
	e外壳着色器,
	e域着色器,
	e几何着色器,
	e像素着色器,
	e计算着色器
};
enum class E根签名标志 {
	e无 = D3D12_ROOT_SIGNATURE_FLAG_NONE,
	e允许输入集成输入层 = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	e阻止顶点着色器根访问 = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS,
	e阻止外壳着色器根访问 = D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS,
	e阻止域着色器根访问 = D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS,
	e阻止几何着色器根访问 = D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS,
	e阻止像素着色器根访问 = D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS,
	e允许流输出 = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT,
};
constexpr D3D12_ROOT_SIGNATURE_FLAGS fc根签名标志(bool a允许输入集成输入层 = false, bool a阻止顶点着色器根访问 = false, bool a阻止外壳着色器根访问 = false, bool a阻止域着色器根访问 = false, bool a阻止几何着色器根访问 = false, bool a阻止像素着色器根访问 = false, bool a允许流输出 = false) {
	return (a允许输入集成输入层 ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a阻止顶点着色器根访问 ? D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a阻止外壳着色器根访问 ? D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a阻止域着色器根访问 ? D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a阻止几何着色器根访问 ? D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a阻止像素着色器根访问 ? D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS : D3D12_ROOT_SIGNATURE_FLAG_NONE) |
		(a允许流输出 ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT : D3D12_ROOT_SIGNATURE_FLAG_NONE);
}
constexpr DXGI_FORMAT c交换链格式 = DXGI_FORMAT_R16G16B16A16_FLOAT;
constexpr DXGI_FORMAT c深度模板格式 = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
constexpr float c清屏深度l = 1;
constexpr float c清屏深度r = 0;
constexpr UINT c描述符范围默认偏移 = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
//==============================================================================
// 结构
//==============================================================================
struct S着色器字节代码 : public D3D12_SHADER_BYTECODE {
	static S着色器字节代码 fc二进制大对象(ID3DBlob *);
};
struct S资源描述 : public D3D12_RESOURCE_DESC {
	static S资源描述 fc缓存(UINT64);
	static S资源描述 fc纹理2(UINT64 宽, UINT 高, DXGI_FORMAT 格式, D3D12_RESOURCE_FLAGS 标志 = D3D12_RESOURCE_FLAG_NONE);
};
struct S图形管线参数 : public D3D12_GRAPHICS_PIPELINE_STATE_DESC {
	S图形管线参数();
	void fs输入布局(const C顶点格式 &);
	void fs根签名(ID3D12RootSignature *);
	void fs顶点着色器(ID3DBlob *);
	void fs像素着色器(ID3DBlob *);
	void fs几何着色器(ID3DBlob *);
	void fs外壳着色器(ID3DBlob *);
	void fs域着色器(ID3DBlob *);
	void fs光栅化(const D3D12_RASTERIZER_DESC &);
	void fs混合(const D3D12_BLEND_DESC &);
	void fs深度模板(const D3D12_DEPTH_STENCIL_DESC &);
	void f关闭深度模板();
	void fs图元拓扑类型(E图元拓扑类型);
};
struct S资源栅栏 : public D3D12_RESOURCE_BARRIER {
	static S资源栅栏 fc变换(ID3D12Resource* 资源, D3D12_RESOURCE_STATES 前, D3D12_RESOURCE_STATES 后, UINT 子资源 = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS 标志 = D3D12_RESOURCE_BARRIER_FLAG_NONE);
};
struct S堆属性 : public D3D12_HEAP_PROPERTIES {
	static S堆属性 fc类型(D3D12_HEAP_TYPE 类型, UINT 创建节点掩码 = 1, UINT 可见节点掩码 = 1);
};
struct S纹理复制区 : public D3D12_TEXTURE_COPY_LOCATION {
	static S纹理复制区 fc脚印(ID3D12Resource*, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &);
	static S纹理复制区 fc索引(ID3D12Resource*, UINT);
};
struct S清除值 : public D3D12_CLEAR_VALUE {
	static S清除值 fc颜色(DXGI_FORMAT, const 数学::S颜色 &);
	static S清除值 fc深度模板(DXGI_FORMAT, float, UINT8);
};
//==============================================================================
// 图形引擎
//==============================================================================
class C三维 {
public:
	static constexpr UINT c帧数 = 2;
	enum E标志 {
		e调试,
		e软件设备
	};
	~C三维();
	//手动初始化
	void f初始化窗口(HWND);
	HRESULT f初始化设备();
	HRESULT f初始化命令队列();
	HRESULT f初始化交换链();
	HRESULT f初始化渲染目标视图();
	HRESULT f初始化深度模板视图();
	HRESULT f初始化根签名();
	//
	HRESULT f创建图形管线(tp图形管线 &, const D3D12_GRAPHICS_PIPELINE_STATE_DESC &);
	HRESULT f创建命令列表(tp命令列表 &, ID3D12PipelineState * = nullptr);
	HRESULT f创建根签名(tp根签名 &, const C根签名参数 &);
	HRESULT f创建根签名(tp根签名 &, const D3D12_ROOT_SIGNATURE_DESC &);
	HRESULT f创建上传资源(tp资源 &, const void *, size_t);
	D3D12_VIEWPORT fg窗口视口();
	D3D12_RECT fg窗口矩形();
	数学::S向量2 fg窗口大小() const;
	//一键初始化
	bool f初始化(const HWND &);
	void f销毁();
	//对象
	C渲染控制 &fg渲染控制();
	C渲染状态 &fg渲染状态();
	C创建设备 &fg创建设备();
	C缓冲工厂 &fg缓冲工厂();
	C纹理工厂 &fg纹理工厂();
	ID3D12RootSignature *fg默认根签名();
public:
	HWND m窗口 = nullptr;
	int m窗口大小[2];
	std::bitset<32> m标志;
	ComPtr<ID3D12Device> m设备;
	ComPtr<ID3D12CommandQueue> m命令队列;
	ComPtr<IDXGISwapChain3> m交换链;
	ComPtr<ID3D12CommandAllocator> m命令分配器;
	ComPtr<ID3D12RootSignature> m根签名;
	std::unique_ptr<C渲染控制> m渲染控制;
	std::unique_ptr<C渲染状态> m渲染状态;
	std::unique_ptr<C创建设备> m创建设备;
	std::unique_ptr<C缓冲工厂> m缓冲工厂;
	std::unique_ptr<C纹理工厂> m纹理工厂;
	std::unique_ptr<C渲染目标管理> m渲染目标管理;
	std::unique_ptr<C深度模板管理> m深度模板管理;
};
class C创建设备 {
public:
	ComPtr<IDXGIFactory4> fg工厂();
	bool f开启调试层();
	HRESULT f取显卡(IDXGIAdapter3**);
	HRESULT f取软件适配器(IDXGIAdapter3**);
	HRESULT f创建设备(IDXGIAdapter3*, ID3D12Device**);
public:
	ComPtr<IDXGIFactory4> m工厂 = nullptr;
	D3D_FEATURE_LEVEL m功能级别 = D3D_FEATURE_LEVEL_11_0;
};
//==============================================================================
// 渲染控制
//==============================================================================
class C渲染控制 {
	friend C三维;
public:
	~C渲染控制();
	HRESULT f初始化围栏(ID3D12Device *);
	void f销毁();
	//设备控制
	void f开始();	//重置命令,并做一些重要设置
	void f结束();	//结束录制命令列表
	void f清屏();
	void f执行命令();
	void f执行命令并显示();
	void f绘制(UINT 顶点数, UINT 开始 = 0);
	void f绘制索引(UINT 索引数, UINT 开始索引 = 0, INT 开始顶点 = 0);
	void f等待完成帧();
	UINT f更新帧索引();
	void f更新资源(ID3D12Resource *目标, ID3D12Resource *源);
	void f更新纹理(ID3D12Resource *目标, ID3D12Resource *源, D3D12_SUBRESOURCE_DATA &);
	//设置
	void fs清屏颜色(const 数学::S颜色 &);
	void fs清屏深度(float);
	void fs清屏模板(UINT8);
	void fs图形管线(ID3D12PipelineState *);
	void fs根签名(ID3D12RootSignature *);
	void fs图元拓扑(E图元拓扑);
	void fs顶点缓冲(const D3D12_VERTEX_BUFFER_VIEW &);
	void fs索引缓冲(const D3D12_INDEX_BUFFER_VIEW &);
	void fs固定缓冲(UINT, const D3D12_GPU_VIRTUAL_ADDRESS &);
	void fs纹理(UINT, const D3D12_GPU_VIRTUAL_ADDRESS &);
	void fs描述符表(UINT, ID3D12DescriptorHeap *);
	//void fs描述符表(const std::initializer_list<ID3D12DescriptorHeap*> &);
	//其它
	D3D12_RESOURCE_BARRIER fc渲染视图栅栏变换(D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATES);
public:
	ComPtr<ID3D12CommandAllocator> m命令分配器;
	ComPtr<ID3D12CommandQueue> m命令队列;
	ComPtr<IDXGISwapChain3> m交换链;
	ComPtr<ID3D12RootSignature> m根签名;
	ComPtr<ID3D12GraphicsCommandList> m命令列表;
	ComPtr<ID3D12PipelineState> m图形管线;
	std::vector<ComPtr<ID3D12Resource>> ma更新资源;
	C渲染目标管理 *m渲染目标管理 = nullptr;
	C深度模板管理 *m深度模板管理 = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m深度模板视图;
	UINT m帧索引 = 0;
	D3D12_VIEWPORT m视口;
	D3D12_RECT m裁剪矩形;
	HANDLE m围栏事件;
	ComPtr<ID3D12Fence> m围栏;
	UINT64 m围栏值;
	数学::S颜色 m清屏颜色 = 数学::S颜色::c黑;
	FLOAT m清屏深度 = c清屏深度r;
	UINT8 m清屏模板 = 0;
};
//==============================================================================
// 管理
//==============================================================================
class C渲染目标管理 {
public:
	const D3D12_CPU_DESCRIPTOR_HANDLE &f更新视图();
	const D3D12_CPU_DESCRIPTOR_HANDLE &fg当前视图() const;
	ID3D12Resource *fg当前资源() const;
public:
	ComPtr<ID3D12Resource> m渲染目标[C三维::c帧数];
	ComPtr<ID3D12DescriptorHeap> m渲染目标堆;
	D3D12_CPU_DESCRIPTOR_HANDLE m当前渲染目标视图;
	UINT m渲染目标视图大小;
	const UINT *m帧索引;
};
class C深度模板管理 {
public:
	ComPtr<ID3D12Resource> m深度模板;
	ComPtr<ID3D12DescriptorHeap> m深度模板堆;
};
//==============================================================================
// 资源&缓冲&视图
//==============================================================================
class C固定缓冲 {
public:
	operator D3D12_GPU_VIRTUAL_ADDRESS() const {
		return m资源->GetGPUVirtualAddress();
	}
	void f复制(void *数据, size_t a大小);
	ComPtr<ID3D12Resource> m资源;
	ComPtr<ID3D12DescriptorHeap> m描述符堆;
	std::byte *m映射;
	const UINT *m帧索引;
};
class C顶点缓冲 {
public:
	operator const D3D12_VERTEX_BUFFER_VIEW &() const {
		return m视图;
	}
	ComPtr<ID3D12Resource> m资源;
	D3D12_VERTEX_BUFFER_VIEW m视图;
};
class C索引缓冲 {
public:
	operator const D3D12_INDEX_BUFFER_VIEW &() const {
		return m视图;
	}
	ComPtr<ID3D12Resource> m资源;
	D3D12_INDEX_BUFFER_VIEW m视图;
};
class C纹理资源 {
public:
	operator D3D12_GPU_VIRTUAL_ADDRESS() const {
		return m资源->GetGPUVirtualAddress();
	}
	operator ID3D12DescriptorHeap*() const {
		return m描述符堆.Get();
	}
	ComPtr<ID3D12Resource> m资源;
	ComPtr<ID3D12DescriptorHeap> m描述符堆;
};
//==============================================================================
// 一些已经定义好的渲染状态
//==============================================================================
class C渲染状态 {
public:
	struct S光栅化 {
		D3D12_RASTERIZER_DESC m默认, m线框渲染, m显示隐藏面, m反面渲染;
	} m光栅化;
	struct S混合 {
		D3D12_BLEND_DESC m默认, m开启透明;
	} m混合;
	struct S深度模板 {
		D3D12_DEPTH_STENCIL_DESC m默认, m正常深度l, m正常深度r, m总是覆盖;
	} m深度模板;
	struct S采样器 {
		D3D12_SAMPLER_DESC m纹理, m图案, m各向异性过滤;
	} m采样器;
	C渲染状态();
	~C渲染状态();
	static D3D12_SAMPLER_DESC ft采样器(const D3D12_STATIC_SAMPLER_DESC &);
	static D3D12_STATIC_SAMPLER_DESC ft静态采样器(const D3D12_SAMPLER_DESC &, UINT 寄存器 = 0, UINT 空间 = 0, D3D12_SHADER_VISIBILITY 可视性 = D3D12_SHADER_VISIBILITY_ALL);
};
//==============================================================================
// 修改
//==============================================================================
class C自动映射 {
public:
	C自动映射(ID3D12Resource*);
	~C自动映射();
	ComPtr<ID3D12Resource> m资源;
	void *m指针;
};
//==============================================================================
// 工厂
//==============================================================================
//输入布局
class C顶点格式 {
public:
	enum E类型 {
		e位置,
		e法线,
		e副法线,
		e颜色,
		e纹理,
		e点大小,
		e切线
	};
	void f清空();
	void f添加(E类型, int 大小);
	void f添加(const char *, int 大小);
	D3D12_INPUT_LAYOUT_DESC fg输入层描述() const;
private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m数组;
	std::map<std::string, int> m类型累计;
	int m字节累计 = 0;
};
//根签名
class C根签名参数 {
public:
	enum E类型 {
		e着色器资源视图,
		e无序访问视图,
		e固定缓冲视图,
		e根常量,
		e采样器,
	};
	enum E根参数类型 {
		e描述符表,
		e常量,
		e描述符,
	};
	struct S根参数索引 {
		E根参数类型 m类型;
		size_t m位置;
	};
	struct S描述符表 {
		size_t m开始位置;
		size_t m数量;
		D3D12_SHADER_VISIBILITY m可见性;
	};
	static D3D12_SHADER_VISIBILITY f计算可见性(E着色器);
	static D3D12_ROOT_PARAMETER_TYPE f计算根参数类型(E类型);
	static D3D12_DESCRIPTOR_RANGE_TYPE f计算描述范围类型(E类型);
	C根签名参数 &f添加描述符(E类型, UINT 寄存器, UINT 空间, E着色器 可见性 = E着色器::e全部);
	C根签名参数 &f添加采样器(const D3D12_SAMPLER_DESC &, UINT 寄存器, UINT 空间, E着色器 可见性 = E着色器::e全部);
	C根签名参数 &f添加范围(E类型 类型, UINT 分配数量 = 1, UINT 开始寄存器 = 0, UINT 寄存器空间 = 0, UINT 偏移 = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, E着色器 可见性 = E着色器::e全部);
	C根签名参数 &f连续添加范围(E类型 类型, UINT 分配数量 = 1, UINT 开始寄存器 = 0, UINT 寄存器空间 = 0, UINT 偏移 = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	C根签名参数 &f连续添加范围开始(E着色器 可见性 = E着色器::e全部);
	C根签名参数 &f连续添加范围结束();
	void fs标志(E根签名标志);
	void fs标志(D3D12_ROOT_SIGNATURE_FLAGS);
public:
	void f实现_添加范围(E类型 类型, UINT 分配数量 = 1, UINT 开始寄存器 = 0, UINT 寄存器空间 = 0, UINT 偏移 = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	void f实现_添加索引(E根参数类型);
	std::vector<S根参数索引> ma索引;
	std::vector<D3D12_ROOT_PARAMETER> ma参数;
	std::vector<D3D12_STATIC_SAMPLER_DESC> ma采样器;
	std::vector<D3D12_DESCRIPTOR_RANGE> ma描述范围;
	std::vector<S描述符表> ma描述符表;
	D3D12_ROOT_SIGNATURE_FLAGS m标志 = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	S描述符表 m连续添加;
	bool mi连续添加 = false;
};
//资源工厂
class C缓冲工厂 {
public:
	ComPtr<ID3D12Device> m设备;
public:
	static DXGI_FORMAT f计算索引格式(UINT 类型大小);
	void f初始化(ID3D12Device *);
	HRESULT f创建只读资源(ComPtr<ID3D12Resource> &, const void *数据, UINT 数据大小);	//创建完不能改
	HRESULT f创建顶点(tp顶点 &, const void *数据, UINT 类型大小, UINT 数据大小);
	HRESULT f创建索引(tp索引 &, const void *数据, UINT 类型大小, UINT 数据大小);
	HRESULT f创建固定(tp固定 &, const void *数据, UINT 类型大小, UINT 数据大小);
};
//着色器工厂
class C着色器工厂 {
public:
	//编译着色器
	static HRESULT f编译顶点着色器(ComPtr<ID3DBlob> &, const wchar_t *文件名, const char *函数名);
	static HRESULT f编译像素着色器(ComPtr<ID3DBlob> &, const wchar_t *文件名, const char *函数名);
	static HRESULT f编译几何着色器(ComPtr<ID3DBlob> &, const wchar_t *文件名, const char *函数名);
	static HRESULT f编译外壳着色器(ComPtr<ID3DBlob> &, const wchar_t *文件名, const char *函数名);
	static HRESULT f编译域着色器(ComPtr<ID3DBlob> &, const wchar_t *文件名, const char *函数名);
	//静态函数
	static HRESULT f编译着色器(const wchar_t *文件名, const char *入口, const char *着色模型, ID3DBlob** 输出);
	static HRESULT f读取着色器(const wchar_t *文件名, std::unique_ptr<std::byte[]> &数据, DWORD &大小);
};
//纹理工厂
class C纹理工厂 {
	friend class C三维;
public:
	~C纹理工厂();
	HRESULT f初始化();
public:	//创建纹理
	HRESULT f从文件创建纹理(tp纹理 &输出, const wchar_t *文件);
	HRESULT f从纹理对象创建纹理(tp纹理 &, const 纹理::I纹理 &);
	HRESULT f从内存创建纹理资源(tp资源 &输出, const void *数据, DXGI_FORMAT 格式, UINT 宽, UINT 高, UINT 行距, UINT 图像大小);
	HRESULT f从纹理对象创建纹理资源(tp资源 &输出, const 纹理::I纹理 &纹理);
	HRESULT f从纹理资源创建纹理视图(tp描述符堆 &输出, const tp资源 &, DXGI_FORMAT 格式);
private:
	std::unique_ptr<纹理::C图像工厂> m工厂;
	ComPtr<ID3D12Device> m设备;
	C三维 *m三维 = nullptr;
	C渲染控制 *m渲染控制 = nullptr;
};
}	//namespace cflw::图形::d3d12