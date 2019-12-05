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
#include "com_type.h"
#include "drv_gpio.h"

void driver_init(void)
{
	//gpio init
	gpio_init();

	//key init
	drv_key_init();
}

int main (void)
{
	

	
	
    while (1) {
		;
    }
}

/* end of file */
