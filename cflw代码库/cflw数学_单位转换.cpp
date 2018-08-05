#include "cflw数学_单位转换.h"
namespace cflw {
namespace 数学 {
const C转换因子表 f角度转换{
	360,	//度
	21600,	//分
	1296000,	//秒
	6.283185307179586476925286766559	//弧度
};
const C转换因子表 f时间转换{
	1e-3,	//毫秒
	1,		//秒
	60,		//分
	3600,	//时
	86400,	//日
	2592000,	//月
	31557600,	//年，等于365.25日
	3155760000	//e世纪
};
const C转换因子表 f长度转换{
	1e-3,	//毫米
	1e-2,	//厘米
	1e-1,	//分米
	1,	//米
	1e3	//千米
};
const C转换因子表 f质量转换{
	1e-6,	//毫克
	1e-3,	//克
	0.5,	//斤
	1,	//千克
	1e3,	//吨
};
const C转换因子表 f数量转换{
	1,
	6.02214129e23,
};
//const C转换因子表 f温度转换{
//	//摄氏度
//	//华氏度
//	//开氏度
//	//兰氏度
//	//列氏度
//};
const C转换因子表 f货币转换{//更新于2015年12月20日
	1,		//人民币,
	6.4811,	//美元,
	7.0408,	//欧元,
	0.0533,	//日元
};
}}
