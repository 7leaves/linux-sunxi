/*
 * arch/arm/mach-sun6i/gpio/gpio_script.c
 * (C) Copyright 2010-2015
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * liugang <liugang@reuuimllatech.com>
 *
 * sun6i gpio driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "gpio_include.h"

/*
 * old gpio api(base on script) realize
 */
extern char sys_cofig_data[];
extern char sys_cofig_data_end[];

#define CSP_OSAL_MALLOC(size) 		kmalloc((size), GFP_ATOMIC)
#define CSP_OSAL_FREE(ptr) 		kfree((ptr))

typedef struct
{
	int mul_sel;
	int pull;
	int drv_level;
	int data;
}gpio_status_set_t;

typedef struct
{
	char    	gpio_name[32];
	int 		port;
	int 		port_num;
	gpio_status_set_t user_gpio_status;
	gpio_status_set_t hardware_gpio_status;
}system_gpio_set_t;

/**
 * gpio_init - script init
 *
 * return SCRIPT_PARSER_OK if success, others if failed
 */
int gpio_script_init(void)
{
	PIO_DBG("%s, line %d, init pin", __FUNCTION__, __LINE__);

	//gpio_g_pioMemBase = (u32)CSP_OSAL_PHY_2_VIRT(CSP_PIN_PHY_ADDR_BASE , CSP_PIN_PHY_ADDR_SIZE);
#ifdef FPGA_RUNTIME_ENV
	return script_parser_init((char *)(sys_cofig_data));
#else
	return script_parser_init((char *)__va(SYS_CONFIG_MEMBASE));
#endif
}
fs_initcall(gpio_script_init);

/**
 * port_to_gpio_index - gpio port to global index, port is from script
 * @port: gpio port group index, eg: 1 for PA, 2 for PB...
 * @port_num: port index in gpio group, eg: 0 for PA0, 1 for PA1...
 *
 * return the gpio index for the port, GPIO_INDEX_INVALID indicate err
 */
static inline u32 port_to_gpio_index(u32 port, u32 port_num)
{
	u32 	usign = 0;
	struct pio_group {
		u32 	base;
		u32 	nr;
	};
	const struct pio_group pio_buf[] = {
		{PA_NR_BASE,         PA_NR},
		{PB_NR_BASE,         PB_NR},
		{PC_NR_BASE,         PC_NR},
		{PD_NR_BASE,         PD_NR},
		{PE_NR_BASE,         PE_NR},
		{PF_NR_BASE,         PF_NR},
		{PG_NR_BASE,         PG_NR},
		{PH_NR_BASE,         PH_NR},
		{GPIO_INDEX_INVALID, 0    },
		{GPIO_INDEX_INVALID, 0    },
		{GPIO_INDEX_INVALID, 0    },
		{PL_NR_BASE,         PL_NR},
		{PM_NR_BASE,         PM_NR}
	};

	PIO_DBG("%s: port %d, port_num %d\n", __FUNCTION__, port, port_num);

	/* para check */
	if(port - 1 >= ARRAY_SIZE(pio_buf)
		|| GPIO_INDEX_INVALID == pio_buf[port - 1].base) {
		usign = __LINE__;
		goto End;
	}

	/* check if port valid */
	if(port_num >= pio_buf[port - 1].nr) {
		usign = __LINE__;
		goto End;
	}

End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d\n", __FUNCTION__, usign);
		return GPIO_INDEX_INVALID;
	} else {
		PIO_DBG("%s success\n", __FUNCTION__);
		return (pio_buf[port - 1].base + port_num);
	}
}

/**
 * sw_gpio_request - config a group of gpio, for each, config the mul_sel/pull/drv_level/data...
 * @gpio_list: ��������õ���GPIO���ݵ����飬GPIO��ֱ��ʹ���������
 * @group_count_max: ����ĳ�Ա������GPIO�趨��ʱ�򣬽�������GPIO��󲻳������ֵ
 *
 * return gpio handle if success, 0 if failed
 */
u32 sw_gpio_request(user_gpio_set_t *gpio_list, u32 group_count_max)
{
	char               *user_gpio_buf;                                        //����char��������
	system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;                      //user_gpio_set���������ڴ�ľ��
	user_gpio_set_t  *tmp_user_gpio_data;
	u32                real_gpio_count = 0, first_port;                      //����������Ч��GPIO�ĸ���
	u32  port, port_num;
	s32  i;

	u32		pio_index = 0;
	u32		usign = 0;
	struct gpio_config config_stru = {0};

	// ���gpio_list��������Ч��gpio����
	if((!gpio_list) || (!group_count_max))
		return (u32)0;
	for(i = 0; i < group_count_max; i++) {
		tmp_user_gpio_data = gpio_list + i;                 //gpio_set����ָ��ÿ��GPIO�����Ա
		if(!tmp_user_gpio_data->port)
			continue;
		real_gpio_count ++;
	}

	// ����user_gpio_buf, ����Ϊ�������
	// fix bug: may be memory overflow in "tmp_sys_gpio_data  = user_gpio_set + i" line below.
	user_gpio_buf = (char *)CSP_OSAL_MALLOC(16 + sizeof(system_gpio_set_t) * group_count_max);   //�����ڴ棬������16���ֽڣ����ڴ��GPIO��������Ϣ
	//user_gpio_buf = (char *)CSP_OSAL_MALLOC(16 + sizeof(system_gpio_set_t) * real_gpio_count);
	if(!user_gpio_buf)
		return (u32)0;
	memset(user_gpio_buf, 0, 16 + sizeof(system_gpio_set_t) * real_gpio_count);        //����ȫ������
	    *(int *)user_gpio_buf = real_gpio_count;                                           //������Ч��GPIO����
	user_gpio_set = (system_gpio_set_t *)(user_gpio_buf + 16);                         //ָ���һ���ṹ��

	//�ҵ�gpio_list��һ����Чport, ����ȡ��ԭʼӲ��������Ϣ
	for(first_port = 0; first_port < group_count_max; first_port++)	{
		tmp_user_gpio_data = gpio_list + first_port;
		port     = tmp_user_gpio_data->port;                         //�����˿���ֵ
		port_num = tmp_user_gpio_data->port_num;                     //�����˿��е�ĳһ��GPIO
		if(!port) {
			PIO_DBG_FUN_LINE;
			continue;
		}

#if 0
		// get the orignal reg value for first valid port
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
		tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);  //����pull�Ĵ���
		tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);//����level�Ĵ���
		tmp_group_data_addr    = PIO_REG_DATA(port);                 //����data�Ĵ���

		tmp_group_func_data    = *tmp_group_func_addr;
		tmp_group_pull_data    = *tmp_group_pull_addr;
		tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
		tmp_group_data_data    = *tmp_group_data_addr;
#endif
		break;
	}
	if(first_port >= group_count_max) { //�Ҳ�����Чport, �򷵻�
		PIO_DBG_FUN_LINE;
		return 0;
	}

	// ��gpio_listÿһ����Ч��, �����û�������Ϣ��user_gpio_buf; ��Ӳ����������;
	//	  ������hardware_gpio_status�ṹ(��Ӳ����ȡ)
	// ��Ӳ������ʱ, ������������������ͬһ�˿ڵ�����
	for(i = first_port; i < group_count_max; i++) {
		// ����������Ч�򷵻�.
		tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
		tmp_user_gpio_data = gpio_list + i;                 //gpio_set����ָ���û���ÿ��GPIO�����Ա
		port     = tmp_user_gpio_data->port;                //�����˿���ֵ
		port_num = tmp_user_gpio_data->port_num;            //�����˿��е�ĳһ��GPIO
		if(!port)
			continue;

		// �����û�������Ϣ
		strcpy(tmp_sys_gpio_data->gpio_name, tmp_user_gpio_data->gpio_name);
		tmp_sys_gpio_data->port                       = port;
		tmp_sys_gpio_data->port_num                   = port_num;
		tmp_sys_gpio_data->user_gpio_status.mul_sel   = tmp_user_gpio_data->mul_sel;
		tmp_sys_gpio_data->user_gpio_status.pull      = tmp_user_gpio_data->pull;
		tmp_sys_gpio_data->user_gpio_status.drv_level = tmp_user_gpio_data->drv_level;
		tmp_sys_gpio_data->user_gpio_status.data      = tmp_user_gpio_data->data;

#if 1
		/* get the gpio index */
		pio_index = port_to_gpio_index(port, port_num);

		/* backup the last config(read from hw) to hardware_gpio_status */
		tmp_sys_gpio_data->hardware_gpio_status.mul_sel = sw_gpio_getcfg(pio_index);
		tmp_sys_gpio_data->hardware_gpio_status.pull = sw_gpio_getpull(pio_index);
		tmp_sys_gpio_data->hardware_gpio_status.drv_level = sw_gpio_getdrvlevel(pio_index);

		/* config pio reg */
		config_stru.gpio = pio_index;
		config_stru.mul_sel = tmp_user_gpio_data->mul_sel;
		config_stru.pull = tmp_user_gpio_data->pull;
		config_stru.drv_level = tmp_user_gpio_data->drv_level;
		if(0 != sw_gpio_setall_range(&config_stru, 1)) {
			usign = __LINE__;
			goto End;
		}
#else
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		// ��Ӳ����������
		if((port_num_pull != pre_port_num_pull) || (port != pre_port)) { //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
			if(func_change)	{
				*tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
				func_change = 0;
			}
			if(pull_change)	{
				pull_change = 0;
				*tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
			}
			if(dlevel_change) {
				dlevel_change = 0;
				*tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���
			}
			if(data_change)	{
				data_change = 0;
				*tmp_group_data_addr   = tmp_group_data_data;    //��д
			}

			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
			tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);   //����pull�Ĵ���
			tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull); //����level�Ĵ���
			tmp_group_data_addr    = PIO_REG_DATA(port);                  //����data�Ĵ���

			tmp_group_func_data    = *tmp_group_func_addr;
			tmp_group_pull_data    = *tmp_group_pull_addr;
			tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
			tmp_group_data_data    = *tmp_group_data_addr;

		}
		else if(pre_port_num_func != port_num_func) { //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
			*tmp_group_func_addr   = tmp_group_func_data;    //��ֻ��д���ܼĴ���
			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ

			tmp_group_func_data    = *tmp_group_func_addr;
		}

		pre_port_num_pull = port_num_pull;                      //���浱ǰӲ���Ĵ�������, ���õ�ǰGPIO��Ϊǰһ��GPIO
		pre_port_num_func = port_num_func;
		pre_port          = port;

		// ���½�Ҫд��func reg��ֵ
		if(tmp_user_gpio_data->mul_sel >= 0) {
			tmp_val = (port_num - (port_num_func<<3)) << 2; /* get the bits offset of cfg reg */
			tmp_sys_gpio_data->hardware_gpio_status.mul_sel = (tmp_group_func_data >> tmp_val) & 0x07;
			tmp_group_func_data &= ~(                              0x07  << tmp_val);
			tmp_group_func_data |=  (tmp_user_gpio_data->mul_sel & 0x07) << tmp_val;
			func_change = 1;
		}
		// ���½�Ҫд��pull reg��ֵ
		tmp_val = (port_num - (port_num_pull<<4)) << 1; /* get the bits offset of pull reg */
		if(tmp_user_gpio_data->pull >= 0) {
			tmp_sys_gpio_data->hardware_gpio_status.pull = (tmp_group_pull_data >> tmp_val) & 0x03;
			if(tmp_user_gpio_data->pull >= 0) {
				tmp_group_pull_data &= ~(                           0x03  << tmp_val);
				tmp_group_pull_data |=  (tmp_user_gpio_data->pull & 0x03) << tmp_val;
				pull_change = 1;
			}
		}
		// ���½�Ҫд��drv level reg��ֵ
		if(tmp_user_gpio_data->drv_level >= 0) { /* tmp_val is as above */
			tmp_sys_gpio_data->hardware_gpio_status.drv_level = (tmp_group_dlevel_data >> tmp_val) & 0x03;
			if(tmp_user_gpio_data->drv_level >= 0) {
				tmp_group_dlevel_data &= ~(                                0x03  << tmp_val);
				tmp_group_dlevel_data |=  (tmp_user_gpio_data->drv_level & 0x03) << tmp_val;
				dlevel_change = 1;
			}
		}
#endif

	// �����û����룬�Լ����ܷ�������Ƿ����data�Ĵ���
#if 1
		if(tmp_user_gpio_data->mul_sel == 1) {
			if(tmp_user_gpio_data->data >= 0)
				__gpio_set_value(pio_index, tmp_user_gpio_data->data);
		}
#else
		if(tmp_user_gpio_data->mul_sel == 1) {
			if(tmp_user_gpio_data->data >= 0) {
				tmp_val = tmp_user_gpio_data->data;
				tmp_val &= 1;
				tmp_group_data_data &= ~(1 << port_num);
				tmp_group_data_data |= tmp_val << port_num;
				data_change = 1;
			}
		}
#endif
	}

#if 0
	// forѭ��������������ڻ�û�л�д�ļĴ���������д�ص�Ӳ������
	if(tmp_group_func_addr) {			//ֻҪ���¹��Ĵ�����ַ���Ϳ��Զ�Ӳ����ֵ
							//��ô�����е�ֵȫ����д��Ӳ���Ĵ���
	        *tmp_group_func_addr   = tmp_group_func_data;       //��д���ܼĴ���
		if(pull_change) {
			*tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
		}
		if(dlevel_change) {
			*tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���
		}
		if(data_change) {
			*tmp_group_data_addr   = tmp_group_data_data;    //��дdata�Ĵ���
		}
	}
#endif

End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);

		/* free buf if err */
		if(NULL != user_gpio_buf) {
			CSP_OSAL_FREE(user_gpio_buf);
			user_gpio_buf = NULL;
		}
		return 0;
	} else {
		return (u32)user_gpio_buf;
	}
}
EXPORT_SYMBOL_GPL(sw_gpio_request);

/**
 * sw_gpio_request_ex - gpio request
 * @main_name: �������������ƣ�ƥ��ģ��(��������)
 * @sub_name: �������Ӽ����ƣ�����ǿգ���ʾȫ��������Ѱ�ҵ�ƥ��ĵ���GPIO
 *
 * return gpio handle if success, 0 if failed
 */
u32 sw_gpio_request_ex(char *main_name, const char *sub_name)  //�豸����GPIO������չ�ӿ�
{
	user_gpio_set_t    *gpio_list=NULL;
	user_gpio_set_t     one_gpio;
	u32               gpio_handle;
	s32               gpio_count;

	if(!sub_name) {
		gpio_count = script_parser_mainkey_get_gpio_count(main_name);
		if(gpio_count <= 0) {
			printk("err: gpio count < =0 ,gpio_count is: %d \n", gpio_count);
			return 0;
		}
		gpio_list = (user_gpio_set_t *)CSP_OSAL_MALLOC(sizeof(system_gpio_set_t) * gpio_count); //����һƬ��ʱ�ڴ棬���ڱ����û�����
		if(!gpio_list) {
			printk("malloc gpio_list error \n");
			return 0;
		}
		if(!script_parser_mainkey_get_gpio_cfg(main_name,gpio_list,gpio_count)){
			gpio_handle = sw_gpio_request(gpio_list, gpio_count);
			CSP_OSAL_FREE(gpio_list);
		} else {
			return 0;
		}
	} else {
		if(script_parser_fetch((char *)main_name, (char *)sub_name, (int *)&one_gpio, (sizeof(user_gpio_set_t) >> 2)) < 0){
			printk("script parser fetch err. \n");
			return 0;
		}

		gpio_handle = sw_gpio_request(&one_gpio, 1);
	}

	return gpio_handle;
}
EXPORT_SYMBOL(sw_gpio_request_ex);

/**
 * sw_gpio_release - gpio release
 * @p_handler: gpio handler
 * @if_release_to_default_status: �Ƿ��ͷŵ�ԭʼ״̬(�Ĵ���ԭ��״̬)
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32 sw_gpio_release(u32 p_handler, s32 if_release_to_default_status)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max, first_port;                    //���GPIO����
	system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32               i;

	u32 	usign = 0;
	u32 	pio_index = 0;
	struct gpio_config config_stru = {0};

	// ��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(!group_count_max)
		return EGPIO_FAIL;
	if(if_release_to_default_status == 2) {
		//printk("gpio module :  release p_handler = %x\n",p_handler);
		CSP_OSAL_FREE((char *)p_handler);
		return EGPIO_SUCCESS;
	}
	user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);

	// ��ȡ�û�����, �ҵ���һ����Ч��
	for(first_port = 0; first_port < group_count_max; first_port++) {
		tmp_sys_gpio_data  = user_gpio_set + first_port;
		port     = tmp_sys_gpio_data->port;                 //�����˿���ֵ
		port_num = tmp_sys_gpio_data->port_num;             //�����˿��е�ĳһ��GPIO
		if(!port)
			continue;

#if 0
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
		tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);  //����pull�Ĵ���
		tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull);//����level�Ĵ���

		tmp_group_func_data    = *tmp_group_func_addr;
		tmp_group_pull_data    = *tmp_group_pull_addr;
		tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
#endif
		break;
	}
	if(first_port >= group_count_max)
		return 0;

	for(i = first_port; i < group_count_max; i++) {
		tmp_sys_gpio_data  = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
		port     = tmp_sys_gpio_data->port;                 //�����˿���ֵ
		port_num = tmp_sys_gpio_data->port_num;             //�����˿��е�ĳһ��GPIO

#if 1
		/* get the gpio index */
		pio_index = port_to_gpio_index(port, port_num);

		config_stru.gpio = pio_index;
		config_stru.mul_sel = 0; /* input */
		config_stru.pull = tmp_sys_gpio_data->hardware_gpio_status.pull;
		config_stru.drv_level = tmp_sys_gpio_data->hardware_gpio_status.drv_level;
		if(0 != sw_gpio_setall_range(&config_stru, 1)) {
			usign = __LINE__;
			goto End;
		}
#else
		port_num_func = (port_num >> 3);
		port_num_pull = (port_num >> 4);

		if((port_num_pull != pre_port_num_pull) || (port != pre_port)) {   //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
			*tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
			*tmp_group_pull_addr   = tmp_group_pull_data;    //��дpull�Ĵ���
			*tmp_group_dlevel_addr = tmp_group_dlevel_data;  //��дdriver level�Ĵ���

			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
			tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);   //����pull�Ĵ���
			tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull); //����level�Ĵ���

			tmp_group_func_data    = *tmp_group_func_addr;
			tmp_group_pull_data    = *tmp_group_pull_addr;
			tmp_group_dlevel_data  = *tmp_group_dlevel_addr;
		} else if(pre_port_num_func != port_num_func) {                      //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
			*tmp_group_func_addr   = tmp_group_func_data;                 //��ֻ��д���ܼĴ���
			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
			tmp_group_func_data    = *tmp_group_func_addr;
		}

		pre_port_num_pull = port_num_pull;
		pre_port_num_func = port_num_func;
		pre_port          = port;
		//���¹��ܼĴ���, ����Ϊ����
		tmp_group_func_data &= ~(0x07 << ((port_num - (port_num_func<<3)) << 2));
		//����pull״̬�Ĵ���
		tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
		tmp_group_pull_data &= ~(0x03  << tmp_val);
		tmp_group_pull_data |= (tmp_sys_gpio_data->hardware_gpio_status.pull & 0x03) << tmp_val;
		//����driver״̬�Ĵ���
		tmp_val              =  (port_num - (port_num_pull<<4)) << 1;
		tmp_group_dlevel_data &= ~(0x03  << tmp_val);
		tmp_group_dlevel_data |= (tmp_sys_gpio_data->hardware_gpio_status.drv_level & 0x03) << tmp_val;
#endif
	}

#if 0
	if(tmp_group_func_addr) {               //ֻҪ���¹��Ĵ�����ַ���Ϳ��Զ�Ӳ����ֵ
						//��ô�����е�ֵȫ����д��Ӳ���Ĵ���
	        *tmp_group_func_addr   = tmp_group_func_data;    //��д���ܼĴ���
	}
	if(tmp_group_pull_addr) {
	        *tmp_group_pull_addr   = tmp_group_pull_data;
	}
	if(tmp_group_dlevel_addr) {
	        *tmp_group_dlevel_addr = tmp_group_dlevel_data;
	}
#endif

End:
	CSP_OSAL_FREE((char *)p_handler);

	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);
		return EGPIO_FAIL;
	} else {
		return EGPIO_SUCCESS;
	}
}
EXPORT_SYMBOL(sw_gpio_release);

/**
 * sw_gpio_get_all_pin_status - ��ȡ�û������������GPIO��״̬
 * @p_handler: gpio handler
 * @gpio_status: �����û����ݵ�����
 * @gpio_count_max: ��������������������Խ��
 * @if_get_user_set_flag: ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_get_all_pin_status(u32 p_handler, user_gpio_set_t *gpio_status, u32 gpio_count_max, u32 if_get_from_hardware)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max, first_port;                    //���GPIO����
	system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
	user_gpio_set_t  *script_gpio;
	u32               port, port_num;
	u32               i;

	u32 	usign = 0;
	u32 	pio_index = 0;
	struct gpio_config config_stru = {0};

	if((!p_handler) || (!gpio_status))
		return EGPIO_FAIL;
	if(gpio_count_max <= 0)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(group_count_max <= 0)
		return EGPIO_FAIL;
	user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
	if(group_count_max > gpio_count_max)
		group_count_max = gpio_count_max;

	//��ȡ�û�����
	//��ʾ��ȡ�û�����������
	if(!if_get_from_hardware) {
		for(i = 0; i < group_count_max; i++) {
			tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
			script_gpio       = gpio_status + i;               //script_gpioָ���û������Ŀռ�

			script_gpio->port      = tmp_sys_gpio_data->port;                       //����port����
			script_gpio->port_num  = tmp_sys_gpio_data->port_num;                   //����port_num����
			script_gpio->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //����pull����
			script_gpio->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //������������
			script_gpio->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //����������������
			script_gpio->data      = tmp_sys_gpio_data->user_gpio_status.data;      //����data����
			strcpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);
		}
	} else {
		for(first_port = 0; first_port < group_count_max; first_port++) {
			tmp_sys_gpio_data  = user_gpio_set + first_port;
			port     = tmp_sys_gpio_data->port;               //�����˿���ֵ
			port_num = tmp_sys_gpio_data->port_num;           //�����˿��е�ĳһ��GPIO

			if(!port)
				continue;

#if 0
			port_num_func = (port_num >> 3);
			port_num_pull = (port_num >> 4);
			tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
			tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);   //����pull�Ĵ���
			tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull); //����level�Ĵ���
			tmp_group_data_addr    = PIO_REG_DATA(port);                  //����data�Ĵ���
#endif
			break;
		}
		if(first_port >= group_count_max)
			return 0;

		for(i = first_port; i < group_count_max; i++) {
			tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
			script_gpio       = gpio_status + i;               //script_gpioָ���û������Ŀռ�

			port     = tmp_sys_gpio_data->port;                //�����˿���ֵ
			port_num = tmp_sys_gpio_data->port_num;            //�����˿��е�ĳһ��GPIO

			script_gpio->port = port;                          //����port����
			script_gpio->port_num  = port_num;                 //����port_num����
			strcpy(script_gpio->gpio_name, tmp_sys_gpio_data->gpio_name);

#if 1
			/* get the gpio index */
			pio_index = port_to_gpio_index(port, port_num);

			config_stru.gpio = pio_index;
			if(0 != sw_gpio_getall_range(&config_stru, 1)) {
				usign = __LINE__;
				goto End;
			} else {
				script_gpio->mul_sel = config_stru.mul_sel;	/* get mul sel */
				script_gpio->pull = config_stru.pull;		/* get pull val */
				script_gpio->drv_level = config_stru.drv_level; /* get drv level val */
				if(script_gpio->mul_sel <= 1) /* get data val if cfg is input/output */
					script_gpio->data = __gpio_get_value(pio_index);
				else
					script_gpio->data = -1;
			}
#else
			port_num_func = (port_num >> 3);
			port_num_pull = (port_num >> 4);

			if((port_num_pull != pre_port_num_pull) || (port != pre_port)) {    //������ֵ�ǰ���ŵĶ˿ڲ�һ�£��������ڵ�pull�Ĵ�����һ��
				tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
				tmp_group_pull_addr    = PIO_REG_PULL(port, port_num_pull);   //����pull�Ĵ���
				tmp_group_dlevel_addr  = PIO_REG_DLEVEL(port, port_num_pull); //����level�Ĵ���
				tmp_group_data_addr    = PIO_REG_DATA(port);                  //����data�Ĵ���
			}
			else if(pre_port_num_func != port_num_func) {                      //������ֵ�ǰ���ŵĹ��ܼĴ�����һ��
				tmp_group_func_addr    = PIO_REG_CFG(port, port_num_func);   //���¹��ܼĴ�����ַ
			}

			pre_port_num_pull = port_num_pull;
			pre_port_num_func = port_num_func;
			pre_port          = port;
			//���û��ؼ���ֵ
			script_gpio->pull      = (*tmp_group_pull_addr   >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //����pull����
			script_gpio->drv_level = (*tmp_group_dlevel_addr >> ((port_num - (port_num_pull<<4))<<1)) & 0x03;    //������������
			script_gpio->mul_sel   = (*tmp_group_func_addr   >> ((port_num - (port_num_func<<3))<<2)) & 0x07;    //������������
			if(script_gpio->mul_sel <= 1) {
				script_gpio->data  = (*tmp_group_data_addr   >>   port_num) & 0x01;                              //����data����
			} else {
				script_gpio->data = -1;
			}
#endif
		}
	}

End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);
		return EGPIO_FAIL;
	} else {
		return EGPIO_SUCCESS;
	}
}
EXPORT_SYMBOL(sw_gpio_get_all_pin_status);

/**
 * sw_gpio_get_one_pin_status - ��ȡ�û��������GPIO��״̬
 * @p_handler: gpio handler
 * @gpio_status: �����û����ݵ�����
 * @gpio_name: Ҫ������GPIO������
 * @if_get_user_set_flag: ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_get_one_pin_status(u32 p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, u32 if_get_from_hardware)
{
	char              *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32               i;

	u32 	usign = 0;
	u32 	pio_index = 0;
	struct gpio_config config_stru = {0};

	//��鴫���ľ������Ч��
	if((!p_handler) || (!gpio_status))
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(group_count_max <= 0)
		return EGPIO_FAIL;
	else if((group_count_max > 1) && (!gpio_name))
		return EGPIO_FAIL;
	user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);

	//��ȡ�û�����
	//��ʾ��ȡ�û�����������
	for(i = 0; i < group_count_max; i++) {
		tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
		if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
			continue;
		strcpy(gpio_status->gpio_name, tmp_sys_gpio_data->gpio_name);
		port                   = tmp_sys_gpio_data->port;
		port_num               = tmp_sys_gpio_data->port_num;
		gpio_status->port      = port;                                              //����port����
		gpio_status->port_num  = port_num;                                          //����port_num����

		if(!if_get_from_hardware) {                                                   //��ǰҪ������û���Ƶ�����
			gpio_status->mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;   //���û����������ж�����������
			gpio_status->pull      = tmp_sys_gpio_data->user_gpio_status.pull;      //���û����������ж���pull����
			gpio_status->drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level; //���û����������ж���������������
			gpio_status->data      = tmp_sys_gpio_data->user_gpio_status.data;      //���û����������ж���data����
		} else {                                                                      //��ǰ�����Ĵ���ʵ�ʵĲ���
#if 1
			/* get the gpio index */
			pio_index = port_to_gpio_index(port, port_num);

			config_stru.gpio = pio_index;
			if(0 != sw_gpio_getall_range(&config_stru, 1)) {
				usign = __LINE__;
				goto End;
			} else {
				gpio_status->mul_sel = config_stru.mul_sel;	/* get mul sel */
				gpio_status->pull = config_stru.pull;		/* get pull val */
				gpio_status->drv_level = config_stru.drv_level; /* get drv level val */
				if(gpio_status->mul_sel <= 1) /* get data val if cfg is input/output */
					gpio_status->data = __gpio_get_value(pio_index);
				else
					gpio_status->data = -1;
			}
#else
			port_num_func = (port_num >> 3);
			port_num_pull = (port_num >> 4);

			tmp_val1 = ((port_num - (port_num_func << 3)) << 2);
			tmp_val2 = ((port_num - (port_num_pull << 4)) << 1);
			gpio_status->mul_sel   = (PIO_REG_CFG_VALUE(port, port_num_func)>>tmp_val1) & 0x07;       //��Ӳ���ж������ܼĴ���
			gpio_status->pull      = (PIO_REG_PULL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;      //��Ӳ���ж���pull�Ĵ���
			gpio_status->drv_level = (PIO_REG_DLEVEL_VALUE(port, port_num_pull)>>tmp_val2) & 0x03;    //��Ӳ���ж���level�Ĵ���
			if(gpio_status->mul_sel <= 1)
			{
				gpio_status->data = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;                     //��Ӳ���ж���data�Ĵ���
			}
			else
			{
				gpio_status->data = -1;
			}
#endif
		}
		break;
	}

End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);
		return EGPIO_FAIL;
	} else {
		return EGPIO_SUCCESS;
	}
}
EXPORT_SYMBOL(sw_gpio_get_one_pin_status);

/**
 * sw_gpio_set_one_pin_status - �����û��������GPIO��ĳһ����״̬
 * @p_handler: gpio handler
 * @gpio_status: �����û����ݵ�����
 * @gpio_name: Ҫ������GPIO������
 * @if_set_to_current_input_status: ��ȡ��־����ʾ��ȡ�û��趨���ݻ�����ʵ������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_set_one_pin_status(u32 p_handler, user_gpio_set_t *gpio_status, const char *gpio_name, u32 if_set_to_current_input_status)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set, *tmp_sys_gpio_data;
	user_gpio_set_t     script_gpio;
	u32               port, port_num;
	u32               i;

	u32 	usign = 0;
	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if((!p_handler) || (!gpio_name))
		return EGPIO_FAIL;
	if((if_set_to_current_input_status) && (!gpio_status))
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(group_count_max <= 0)
		return EGPIO_FAIL;
	user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);

	//��ȡ�û�����
	//��ʾ��ȡ�û�����������
	for(i = 0; i < group_count_max; i++) {
		tmp_sys_gpio_data = user_gpio_set + i;             //tmp_sys_gpio_dataָ�������GPIO�ռ�
		if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name))
			continue;

		port          = tmp_sys_gpio_data->port;                           //����port����
		port_num      = tmp_sys_gpio_data->port_num;                       //����port_num����

		if(if_set_to_current_input_status) {                                //���ݵ�ǰ�û��趨����
			//�޸�FUCN�Ĵ���
			script_gpio.mul_sel   = gpio_status->mul_sel;
			script_gpio.pull      = gpio_status->pull;
			script_gpio.drv_level = gpio_status->drv_level;
			script_gpio.data      = gpio_status->data;
		} else {
			script_gpio.mul_sel   = tmp_sys_gpio_data->user_gpio_status.mul_sel;
			script_gpio.pull      = tmp_sys_gpio_data->user_gpio_status.pull;
			script_gpio.drv_level = tmp_sys_gpio_data->user_gpio_status.drv_level;
			script_gpio.data      = tmp_sys_gpio_data->user_gpio_status.data;
		}

#if 1
		/* get the gpio index */
		pio_index = port_to_gpio_index(port, port_num);
#endif

		if(script_gpio.mul_sel >= 0) {
#if 1
			/* set cfg */
			if(0 != sw_gpio_setcfg(pio_index, script_gpio.mul_sel)) {
				usign = __LINE__;
				goto End;
			}
#else
			tmp_addr = PIO_REG_CFG(port, port_num_func);
			reg_val = *tmp_addr;                                                       //�޸�FUNC�Ĵ���
			tmp_val = (port_num - (port_num_func<<3))<<2;
			reg_val &= ~(0x07 << tmp_val);
			reg_val |=  (script_gpio.mul_sel) << tmp_val;
			*tmp_addr = reg_val;
#endif
		}

		//�޸�PULL�Ĵ���
		if(script_gpio.pull >= 0) {
#if 1
			if(0 != sw_gpio_setpull(pio_index, script_gpio.pull)) {
				usign = __LINE__;
				goto End;
			}
#else
		tmp_addr = PIO_REG_PULL(port, port_num_pull);
		reg_val = *tmp_addr;                                                     //�޸�FUNC�Ĵ���
		tmp_val = (port_num - (port_num_pull<<4))<<1;
		reg_val &= ~(0x03 << tmp_val);
		reg_val |=  (script_gpio.pull) << tmp_val;
		            *tmp_addr = reg_val;
#endif
		}

		//�޸�DLEVEL�Ĵ���
		if(script_gpio.drv_level >= 0) {
#if 1
			if(0 != sw_gpio_setdrvlevel(pio_index, script_gpio.drv_level)) {
				usign = __LINE__;
				goto End;
			}
#else
			tmp_addr = PIO_REG_DLEVEL(port, port_num_pull);
			reg_val = *tmp_addr;                                                         //�޸�FUNC�Ĵ���
			tmp_val = (port_num - (port_num_pull<<4))<<1;
			reg_val &= ~(0x03 << tmp_val);
			reg_val |=  (script_gpio.drv_level) << tmp_val;
			*tmp_addr = reg_val;
#endif
		}

		//�޸�data�Ĵ���
		if(script_gpio.mul_sel == 1) {
			if(script_gpio.data >= 0) {
#if 1
				__gpio_set_value(pio_index, script_gpio.data);
#else
				tmp_addr = PIO_REG_DATA(port);
				reg_val = *tmp_addr;                                                      //�޸�DATA�Ĵ���
				reg_val &= ~(0x01 << port_num);
				reg_val |=  (script_gpio.data & 0x01) << port_num;
				                *tmp_addr = reg_val;
#endif
			}
		}
break;
	}

End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);
		return EGPIO_FAIL;
	} else {
		return EGPIO_SUCCESS;
	}
}
EXPORT_SYMBOL(sw_gpio_set_one_pin_status);

/**
 * sw_gpio_set_one_pin_io_status - �޸��û��������GPIO�е�ĳһ��IO�ڵģ��������״̬
 * @p_handler: gpio handler
 * @if_set_to_output_status: ���ó����״̬��������״̬
 * @gpio_name: Ҫ������GPIO������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_set_one_pin_io_status(u32 p_handler, u32 if_set_to_output_status, const char *gpio_name)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32                i;

	u32 	ucfg = 0;
	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	if(if_set_to_output_status > 1)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
	if(group_count_max == 0) {
		return EGPIO_FAIL;
	} else if(group_count_max == 1) {
		user_gpio_set = tmp_sys_gpio_data;
	} else if(gpio_name) {
		for(i=0; i<group_count_max; i++) {
			if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name)) {
				tmp_sys_gpio_data ++;
				continue;
			}
			user_gpio_set = tmp_sys_gpio_data;
			break;
		}
	}
	if(!user_gpio_set)
		return EGPIO_FAIL;

	port     = user_gpio_set->port;
	port_num = user_gpio_set->port_num;

#if 1
	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);

	/* set cfg */
	ucfg = (if_set_to_output_status ? 1 : 0);
	if(0 != sw_gpio_setcfg(pio_index, ucfg)) {
		PIO_ERR_FUN_LINE;
		return EGPIO_FAIL;
	}
#else
	port_num_func = port_num >> 3;

	tmp_group_func_addr = PIO_REG_CFG(port, port_num_func);
	reg_val = *tmp_group_func_addr;
	reg_val &= ~(0x07 << (((port_num - (port_num_func<<3))<<2)));
	reg_val |=   if_set_to_output_status << (((port_num - (port_num_func<<3))<<2));
	*tmp_group_func_addr = reg_val;
#endif

return EGPIO_SUCCESS;
}
EXPORT_SYMBOL(sw_gpio_set_one_pin_io_status);

/**
 * sw_gpio_set_one_pin_pull - �޸��û��������GPIO�е�ĳһ��IO�ڵģ�PULL״̬
 * @p_handler: gpio handler
 * @if_set_to_output_status: �����õ�pull״̬
 * @gpio_name: Ҫ������GPIO������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_set_one_pin_pull(u32 p_handler, u32 set_pull_status, const char *gpio_name)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32                i;

	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	if(set_pull_status >= 4)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);
	if(group_count_max == 0) {
		return EGPIO_FAIL;
	} else if(group_count_max == 1)	{
		user_gpio_set = tmp_sys_gpio_data;
	} else if(gpio_name) {
		for(i=0; i<group_count_max; i++) {
			if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name)) {
				tmp_sys_gpio_data ++;
				continue;
			}
			user_gpio_set = tmp_sys_gpio_data;
			break;
		}
	}
	if(!user_gpio_set)
		return EGPIO_FAIL;

	port     = user_gpio_set->port;
	port_num = user_gpio_set->port_num;

#if 1
	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);

	if(0 != sw_gpio_setpull(pio_index, set_pull_status)) {
		PIO_ERR_FUN_LINE;
		return EGPIO_FAIL;
	}
#else
	port_num_pull = port_num >> 4;

	tmp_group_pull_addr = PIO_REG_PULL(port, port_num_pull);
	reg_val = *tmp_group_pull_addr;
	reg_val &= ~(0x03 << (((port_num - (port_num_pull<<4))<<1)));
	reg_val |=  (set_pull_status << (((port_num - (port_num_pull<<4))<<1)));
	*tmp_group_pull_addr = reg_val;
#endif

return EGPIO_SUCCESS;
}
EXPORT_SYMBOL(sw_gpio_set_one_pin_pull);

/**
 * sw_gpio_set_one_pin_driver_level - �޸��û��������GPIO�е�ĳһ��IO�ڵģ���������
 * @p_handler: gpio handler
 * @set_driver_level: �����õ����������ȼ�
 * @gpio_name: Ҫ������GPIO������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_set_one_pin_driver_level(u32 p_handler, u32 set_driver_level, const char *gpio_name)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32                i;

	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	if(set_driver_level >= 4)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

	if(group_count_max == 0) {
		return EGPIO_FAIL;
	} else if(group_count_max == 1)	{
		user_gpio_set = tmp_sys_gpio_data;
	} else if(gpio_name) {
		for(i=0; i<group_count_max; i++) {
			if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name)) {
				tmp_sys_gpio_data ++;
				continue;
			}
			user_gpio_set = tmp_sys_gpio_data;
			break;
		}
	}
	if(!user_gpio_set)
		return EGPIO_FAIL;

	port     = user_gpio_set->port;
	port_num = user_gpio_set->port_num;

#if 1
	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);

	if(0 != sw_gpio_setdrvlevel(pio_index, set_driver_level)) {
		PIO_ERR_FUN_LINE;
		return EGPIO_FAIL;
	}
#else
	port_num_dlevel = port_num >> 4;

	tmp_group_dlevel_addr = PIO_REG_DLEVEL(port, port_num_dlevel);
	reg_val = *tmp_group_dlevel_addr;
	reg_val &= ~(0x03 << (((port_num - (port_num_dlevel<<4))<<1)));
	reg_val |=  (set_driver_level << (((port_num - (port_num_dlevel<<4))<<1)));
	*tmp_group_dlevel_addr = reg_val;
#endif

return EGPIO_SUCCESS;
}
EXPORT_SYMBOL(sw_gpio_set_one_pin_driver_level);

/**
 * sw_gpio_read_one_pin_value - ��ȡ�û��������GPIO�е�ĳһ��IO�ڵĶ˿ڵĵ�ƽ
 * @p_handler: gpio handler
 * @gpio_name: Ҫ������GPIO������
 *
 * return the pin value if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_read_one_pin_value(u32 p_handler, const char *gpio_name)
{
	char               *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t  *user_gpio_set = NULL, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32                i;

	u32 	ucfg = 0;
	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

	if(group_count_max == 0) {
		return EGPIO_FAIL;
	} else if(group_count_max == 1)	{
		user_gpio_set = tmp_sys_gpio_data;
	} else if(gpio_name) {
		for(i=0; i<group_count_max; i++) {
			if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name)) {
				tmp_sys_gpio_data ++;
				continue;
			}
			user_gpio_set = tmp_sys_gpio_data;
			break;
		}
	}
	if(!user_gpio_set)
		return EGPIO_FAIL;

	port     = user_gpio_set->port;
	port_num = user_gpio_set->port_num;

#if 1
	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);

	ucfg = sw_gpio_getcfg(pio_index);
	if(0 == ucfg)
		return __gpio_get_value(pio_index);
#else
	port_num_func = port_num >> 3;

	reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);
	func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;
	if(func_val == 0)
	{
		reg_val = (PIO_REG_DATA_VALUE(port) >> port_num) & 0x01;

		return reg_val;
	}
#endif

	PIO_ERR_FUN_LINE;
	return EGPIO_FAIL;
}
EXPORT_SYMBOL(sw_gpio_read_one_pin_value);

/**
 * sw_gpio_write_one_pin_value - �޸��û��������GPIO�е�ĳһ��IO�ڵĶ˿ڵĵ�ƽ
 * @p_handler: gpio handler
 * @value_to_gpio:  Ҫ���õĵ�ƽ�ĵ�ѹ
 * @gpio_name: Ҫ������GPIO������
 *
 * return EGPIO_SUCCESS if success, EGPIO_FAIL if failed
 */
s32  sw_gpio_write_one_pin_value(u32 p_handler, u32 value_to_gpio, const char *gpio_name)
{
	char              *tmp_buf;                                        //ת����char����
	u32               group_count_max;                                //���GPIO����
	system_gpio_set_t *user_gpio_set = NULL, *tmp_sys_gpio_data;
	u32               port, port_num;
	u32               i;

	u32 	ucfg = 0;
	u32 	uval = 0;
	u32 	pio_index = 0;

	//��鴫���ľ������Ч��
	if(!p_handler)
		return EGPIO_FAIL;
	if(value_to_gpio >= 2)
		return EGPIO_FAIL;
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	tmp_sys_gpio_data = (system_gpio_set_t *)(tmp_buf + 16);

	if(group_count_max == 0) {
		return EGPIO_FAIL;
	} else if(group_count_max == 1)	{
		user_gpio_set = tmp_sys_gpio_data;
	} else if(gpio_name) {
		for(i=0; i<group_count_max; i++) {
			if(strcmp(gpio_name, tmp_sys_gpio_data->gpio_name)) {
				tmp_sys_gpio_data ++;
				continue;
			}
			user_gpio_set = tmp_sys_gpio_data;
			break;
		}
	}
	if(!user_gpio_set)
		return EGPIO_FAIL;

	port     = user_gpio_set->port;
	port_num = user_gpio_set->port_num;

#if 1
	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);

	ucfg = sw_gpio_getcfg(pio_index);
	if(1 == ucfg) {
		uval = (value_to_gpio ? 1 : 0);
		__gpio_set_value(pio_index, value_to_gpio);
		return EGPIO_SUCCESS;
	}
#else
	port_num_func = port_num >> 3;

	reg_val  = PIO_REG_CFG_VALUE(port, port_num_func);
	func_val = (reg_val >> ((port_num - (port_num_func<<3))<<2)) & 0x07;
	if(func_val == 1)
	{
		tmp_group_data_addr = PIO_REG_DATA(port);
		reg_val = *tmp_group_data_addr;
		reg_val &= ~(1 << port_num);
		reg_val |=  (value_to_gpio << port_num);
		*tmp_group_data_addr = reg_val;

		return EGPIO_SUCCESS;
	}
#endif

	return EGPIO_FAIL;
}
EXPORT_SYMBOL(sw_gpio_write_one_pin_value);

/**
 * sw_gpio_get_index - get the global gpio index
 * @p_handler: gpio handler
 * @gpio_name: gpio name whose index will be got. when NULL,
 �* 		the first port of p_handler willbe treated.
 *
 * return the gpio index for the port, GPIO_INDEX_INVALID indicate err
 */
u32 sw_gpio_get_index(u32 p_handler, const char *gpio_name)
{
	char		*tmp_buf;                                        //ת����char����
	u32		group_count_max;                                //���GPIO����
	system_gpio_set_t *user_gpio_set = NULL;
	u32		port, port_num;
	u32		i;
	u32		usign = 0;
	u32 		pio_index = 0;

	if(!p_handler) {
		usign = __LINE__;
		goto End;
	}
	tmp_buf = (char *)p_handler;
	group_count_max = *(int *)tmp_buf;
	if(0 == group_count_max) {
		usign = __LINE__;
		goto End;
	}

	user_gpio_set = (system_gpio_set_t *)(tmp_buf + 16);
	if(NULL == gpio_name) { /* when name is NULL, treat the first one */
		port     = user_gpio_set->port;
		port_num = user_gpio_set->port_num;
	} else {
		for(i = 0; i < group_count_max; i++, user_gpio_set++) {
			if(!strcmp(gpio_name, user_gpio_set->gpio_name)) {
				port     = user_gpio_set->port;
				port_num = user_gpio_set->port_num;
				break;
			}
		}
		if(i == group_count_max) { /* cannot find the gpio_name item */
			usign = __LINE__;
			goto End;
		}
	}

	/* get the gpio index */
	pio_index = port_to_gpio_index(port, port_num);
End:
	if(0 != usign) {
		PIO_ERR("%s err, line %d", __FUNCTION__, usign);
		return GPIO_INDEX_INVALID;
	} else
		return pio_index;
}
EXPORT_SYMBOL(sw_gpio_get_index);

/**
 * sw_gpio_port_to_index - gpio port to global index, port is from script
 * @port: gpio port group index, eg: 1 for PA, 2 for PB...
 * @port_num: port index in gpio group, eg: 0 for PA0, 1 for PA1...
 *
 * return the gpio index for the port, GPIO_INDEX_INVALID indicate err
 */
u32 sw_gpio_port_to_index(u32 port, u32 port_num)
{
	return port_to_gpio_index(port, port_num);
}
EXPORT_SYMBOL(sw_gpio_port_to_index);
