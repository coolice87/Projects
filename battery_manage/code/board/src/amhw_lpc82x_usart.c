/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2015 Guangzhou ZHIYUAN Electronics Stock Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
* e-mail:      ametal.support@zlg.cn
*******************************************************************************/

/**
 * \file
 * \brief USART Ӳ�������ӿ�ʵ��
 *
 * \internal
 * \par Modification History
 * - 1.00 15-07-03  bob, first implementation.
 * \endinternal
 */
 
#include "amhw_lpc82x_usart.h"
 
/**
 * \brief �������ֵ
 */
static int32_t __err_cal (uint32_t  u_clk, 
                          uint32_t  baudrate, 
                          uint32_t *p_prev_err)
{
    uint32_t err  = u_clk - (u_clk / baudrate) * baudrate;
    
    uint32_t herr = ((u_clk / baudrate) + 1) * baudrate - u_clk;
  
    if (NULL == p_prev_err) {
        return -AM_EINVAL;
    }
    
    if (herr < err) {
        err = herr;
    }
    
    /* �����ǰ�����С�ڵ�ǰ��� */
    if (*p_prev_err <= err) {  
        return 0;
    }
    
    *p_prev_err = err;
    
    /*
     * 1: herr != err ( init err < herr)  
     * 2: herr == err ( init err > herr)
     */                           
    return (herr == err) + 1;
}

/**
 * \brief ȷ����Ƶֵ�͹�����ֵ
 */
static int __usart_div_cal (uint32_t  u_clk,
                            uint32_t *p_baudrate,
                            uint32_t *p_div,
                            uint32_t *p_oversamples)
{
    uint32_t ovr         = 0;
    uint32_t pre_err     = ~0UL; /* ��ʼ������úܴ��ֵ */
    uint32_t div         = 0;
    uint32_t oversamples = 0;
  
    if (NULL == p_baudrate ||
        NULL == p_div      ||
        NULL == p_oversamples) {
        return -AM_EINVAL;
    }
    
    oversamples = *p_oversamples;

    /* ��������ʵĹ������� */
    if (oversamples == 0) {
        for (ovr = 16; ovr > 4; ovr--) {
            int32_t tmp = __err_cal(u_clk, *p_baudrate * ovr, &pre_err);

            /* �����ǰ���û�н�һ����С������������� */
            if (!tmp) {
                continue;
            }

            div         = tmp - 1;
            oversamples = ovr;
        }
    }
    
    if (!oversamples) {             
        return -AM_EINVAL;
    }

    div += u_clk / (*p_baudrate * oversamples);
    
    if (!div) {                         
        return -AM_EINVAL;
    }

    /* ���������ʵĹ��������Ƶֵ�����Ӧ����ʵ������ */
    *p_div         = div;
    *p_baudrate    = u_clk / (div * oversamples);         
    *p_oversamples = oversamples;
    
    return AM_OK;
}

/**
 * \brief ���ڲ���������
 */
int amhw_lpc82x_usart_baudrate_set (amhw_lpc82x_usart_t *p_hw_usart,
                                    uint32_t             u_clk,
                                    uint32_t             baudrate)
{
    uint32_t div         = 0;
    uint32_t oversamples = 0;

    if (NULL == p_hw_usart) {
        return -AM_EINVAL;
    }
    
    /* ϵͳʱ��30MHz��Ƶ�õ�u_clk */
    switch (baudrate) {
    
    case 9600 :
        oversamples = 0x0F;
        div         = 0x4D;
        break;
    
    case 19200 :
        oversamples = 0x10;
        div         = 0x24;
        break;
    
    case 28800 :
        oversamples = 0x0B;
        div         = 0x23;
        break;
    
    case 38400 :
        oversamples = 0x10;
        div         = 0x12;
        break;
    
    case 57600 :
        oversamples = 0x10;
        div         = 0x0C;
        break;
    
    case 115200 :
        oversamples = 0x10;
        div         = 0x06;
        break;
    
    case 172800 :
        oversamples = 0x10;
        div         = 0x04;
        break;
    
    case 230400 :
        oversamples = 0x10;
        div         = 0x03;
        break;
    
    default :
        /* ����������С�Ĺ������ʣ���Ƶ���÷��� */
        __usart_div_cal (u_clk,
                         &baudrate,
                         &div,
                         &oversamples);
        break;
    }

    /* ���ù������ʣ�brg��Ƶ */
    amhw_lpc82x_usart_osr_set(p_hw_usart, oversamples-1);
    amhw_lpc82x_usart_brg_set(p_hw_usart, div-1);

    return baudrate;
}

/**
 * \brief ���ڲ����ʻ�ȡ
 */
uint32_t amhw_lpc82x_usart_baudrate_get (amhw_lpc82x_usart_t *p_hw_usart,
                                         uint32_t             u_clk)
{
    uint32_t div;
    uint32_t oversamples;
    uint32_t baudrate = 0;
  
    if (0 == u_clk) {
        return 0;
    }
    
    div         = (uint32_t)(p_hw_usart->brg & 0xffff) + 1;
    oversamples = (uint32_t)(p_hw_usart->osr & 0xff) + 1;
    
    if (oversamples <= 4) {
        return 0;
    }
    
    baudrate = u_clk / oversamples / div;
  
    return baudrate;
}

/**
 * \brief USART���ݷ���(��ѯģʽ)
 */
int amhw_lpc82x_usart_poll_send (amhw_lpc82x_usart_t *p_hw_usart,
                                 const uint8_t       *p_txbuf,
                                 uint32_t             nbytes)
{
    uint32_t len = nbytes;
  
    if (NULL == p_hw_usart || NULL == p_txbuf) {
        return -AM_EINVAL;
    }
    
    while (len--) {
        while ((p_hw_usart->stat & AMHW_LPC82X_USART_STAT_TXRDY) == 0);
        amhw_lpc82x_usart_txdata_write(p_hw_usart, (uint32_t)(*p_txbuf++));
    }
    
    return nbytes;
}

/**
 * \brief USART���ݽ���(��ѯģʽ)
 */
int amhw_lpc82x_usart_poll_receive (amhw_lpc82x_usart_t *p_hw_usart,
                                    uint8_t             *p_rxbuf,
                                    uint32_t             nbytes)
{
    uint32_t len = nbytes;
  
    if (NULL == p_hw_usart || NULL == p_rxbuf) {
        return -AM_EINVAL;
    }
    
    while (len--) {
        while((p_hw_usart->stat & AMHW_LPC82X_USART_STAT_RXRDY) == 0);
        *p_rxbuf++ = amhw_lpc82x_usart_rxdata_read(p_hw_usart);
    }
    
    return nbytes;
}

/* end of file */