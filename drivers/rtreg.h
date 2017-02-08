/****************************************Copyright (c)****************************************************
**                             成 都 世 纪 华 宁 科 技 有 限 公 司
**                                http://www.6lowpanworld.com
**                                http://hichard.taobao.com
**
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:           rtreg.h
** Last modified Date:  2014-12-23
** Last Version:        V1.0
** Description:         直接访问硬件寄存器的函数定义，主要用户编写设备驱动程序
** 
**--------------------------------------------------------------------------------------------------------
** Created By:          Renhaibo任海波
** Created date:        2014-12-23
** Version:             V1.0
** Descriptions:        The original version 初始版本
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#ifndef __RT_REG_H__
#define __RT_REG_H__

#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************************************************
** 头文件包含
*********************************************************************************************************/
#include "rtdef.h"

/*********************************************************************************************************
** 特殊宏定义
*********************************************************************************************************/
#define RT_MACRO_START     do {
#define RT_MACRO_END       } while (0)
    
/*********************************************************************************************************
** 访问寄存器的宏定义
*********************************************************************************************************/
/*********************************************************************************************************
** 直接将寄存器映射
*********************************************************************************************************/
#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))

/*********************************************************************************************************
** 参考ecos移植过来的，通过函数访问
*********************************************************************************************************/
//  8位访问寄存器的宏定义
#define HAL_READ_UINT8( _register_, _value_ )       \
        ((_value_) = *((volatile rt_uint8_t *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ )      \
        (*((volatile rt_uint8_t *)(_register_)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile rt_uint8_t *)(_register_))[_j_];      \
    RT_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile rt_uint8_t *)(_register_))[_j_] = (_buf_)[_i_];      \
    RT_MACRO_END

#define HAL_READ_UINT8_STRING( _register_, _buf_, _count_ )             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile rt_uint8_t *)(_register_))[_i_];      \
    RT_MACRO_END

#define HAL_WRITE_UINT8_STRING( _register_, _buf_, _count_ )            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile rt_uint8_t *)(_register_)) = (_buf_)[_i_];           \
    RT_MACRO_END

//  16位访问寄存器的宏定义
#define HAL_READ_UINT16( _register_, _value_ )      \
        ((_value_) = *((volatile rt_uint16_t *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ )     \
        (*((volatile rt_uint16_t *)(_register_)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile rt_uint16_t *)(_register_))[_j_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile rt_uint16_t *)(_register_))[_j_] = (_buf_)[_i_];     \
    RT_MACRO_END

#define HAL_READ_UINT16_STRING( _register_, _buf_, _count_)             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile rt_uint16_t *)(_register_))[_i_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT16_STRING( _register_, _buf_, _count_)            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile rt_uint16_t *)(_register_))[_i_] = (_buf_)[_i_];     \
    RT_MACRO_END

// 32位访问寄存器的宏定义
#define HAL_READ_UINT32( _register_, _value_ )      \
        ((_value_) = *((volatile rt_uint32_t *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ )     \
        (*((volatile rt_uint32_t *)(_register_)) = (_value_))

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile rt_uint32_t *)(_register_))[_j_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile rt_uint32_t *)(_register_))[_j_] = (_buf_)[_i_];     \
    RT_MACRO_END

#define HAL_READ_UINT32_STRING( _register_, _buf_, _count_)             \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile rt_uint32_t *)(_register_))[_i_];     \
    RT_MACRO_END

#define HAL_WRITE_UINT32_STRING( _register_, _buf_, _count_)            \
    RT_MACRO_START                                                      \
    rt_uint32_t _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile rt_uint32_t *)(_register_))[_i_] = (_buf_)[_i_];     \
    RT_MACRO_END

#ifdef __cplusplus
    }
#endif      // __cplusplus

#endif // endif of __RT_REG_H__
/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/
