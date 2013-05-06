#include <linux/init.h>
#include <linux/platform_device.h>
#include <mach/rpc_server_handset.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/delay.h>
#include <linux/gpio_event.h>
#ifdef CONFIG_PN544_NFC_XXX	//wongab.jeon@lge.com
#include <linux/nfc/pn544.h> // 2011.06.24 kiwon.jeon@lge.com NFC
#endif
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/board_lge.h>
#include <linux/regulator/consumer.h>

#include "devices-msm7x2xa.h"
#include "board-e0.h"

#define HARD_HOME_KEY 172
/* handset device */
static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 10, /* 0 will disable end key */
};

static struct platform_device hs_pdev = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static unsigned int keypad_row_gpios[] = {
	36, 37, 38
};
static unsigned int keypad_col_gpios[] = {32, 33};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_e0[] = {
	[KEYMAP_INDEX(1, 1)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(1, 0)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(0, 2)] = KEY_HOME,
};

int e0_matrix_info_wrapper(struct gpio_event_input_devs *input_dev,
							 struct gpio_event_info *info, void **data, int func)
{
        int ret ;
	if (func == GPIO_EVENT_FUNC_RESUME) {
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[0], 0,
					GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[1], 0,
					GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);

		/* LGE_CHANGE_S: E0 wonsang.yoon@lge.com [2011-10-17] : for Rev.B Key MAPl */
		gpio_tlmm_config(
			GPIO_CFG(keypad_row_gpios[2], 0,
					GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		}
		/* LGE_CHANGE_N: E0 wonsang.yoon@lge.com [2011-10-17] : for Rev.B Key MAPl */

		ret = gpio_event_matrix_func(input_dev,info, data,func);
        return ret ;
}

static int e0_gpio_matrix_power(const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function
	 * to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */

	return 0;
}

static struct gpio_event_matrix_info e0_keypad_matrix_info = {
	.info.func	= e0_matrix_info_wrapper,
	.keymap		= keypad_keymap_e0,
	.output_gpios	= keypad_col_gpios,
	.input_gpios	= keypad_row_gpios,
	.noutputs	= ARRAY_SIZE(keypad_col_gpios),
	.ninputs	= ARRAY_SIZE(keypad_row_gpios),
	.settle_time.tv_nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv_nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *e0_keypad_info[] = {
	&e0_keypad_matrix_info.info
};

static struct gpio_event_platform_data e0_keypad_data = {
	.name		= "e0_keypad",
	.info		= e0_keypad_info,
	.info_count	= ARRAY_SIZE(e0_keypad_info),
	.power          = e0_gpio_matrix_power,
};

struct platform_device keypad_device_e0 = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &e0_keypad_data,
	},
};


/* input platform device */
static struct platform_device *e0_input_devices[] __initdata = {
	&hs_pdev,
};

static struct platform_device *e0_gpio_input_devices[] __initdata = {
	&keypad_device_e0,
};

/* Melfas MCS8000 Touch (mms-128)*/
#if defined(CONFIG_TOUCHSCREEN_MCS8000_MMS128)
static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= TS_GPIO_I2C_SDA,
		.scl_pin	= TS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= TS_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay			= 1,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};

int ts_set_vreg(unsigned char onoff)
{
	int rc;
	static struct regulator *ldo1 = NULL;
	static int init = 0;
	ldo1 = regulator_get(NULL, "RT8053_LDO1");

	if (ldo1 == NULL)
		pr_err(
			"%s: regulator_get(ldo1) failed\n",
			__func__);

	printk(KERN_INFO "ts_set_vreg : %d\n", onoff);
	if(onoff){
		rc = regulator_set_voltage(ldo1, 3050000, 3050000);
		if (rc < 0)
			pr_err(
				"%s: regulator_set_voltage(ldo1) failed\n",
				__func__);

		rc = regulator_enable(ldo1);
		if (rc < 0)
			pr_err(
				"%s: regulator_enable(ldo1) failed\n",
				__func__);

		init = 1;
	} else {
		if (init > 0) {
			rc = regulator_disable(ldo1);
			if (rc < 0)
				pr_err(
					"%s: regulator_disble(ldo1) failed\n",
					__func__);

			regulator_put(ldo1);
		}
	}
	msleep(20);
	return 0;
}


static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power 	  = ts_set_vreg,
	.irq 	  = TS_GPIO_IRQ,
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs8000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs8000",
		.platform_data = &ts_pdata,
	},
};

/* this routine should be checked for nessarry */
static int init_gpio_i2c_pin_touch(
	struct i2c_gpio_platform_data *i2c_adap_pdata,
	struct gpio_i2c_pin gpio_i2c_pin,
	struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	/*
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	*/
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		/*gpio_tlmm_config(
			GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);*/
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	return 0;
}

static void __init e0_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}
#endif /* CONFIG_TOUCHSCREEN_MCS8000_MMS128 */

#ifdef CONFIG_LGE_DIAGTEST
static struct platform_device lg_diag_input_device = {
	.name = "ats_input",
	.id = -1,
	.dev = { .platform_data = 0, },
};

static struct platform_device *e0_ats_input_devices[] __initdata = {
       &lg_diag_input_device,
};
#endif

/** accelerometer & ecompass **/
#if defined (CONFIG_SENSORS_BMM050) ||defined(CONFIG_SENSORS_BMA250)
static struct gpio_i2c_pin sensor_i2c_pin[] = {
	[0] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
	[1] = {
		.sda_pin	= SENSOR_GPIO_I2C_SDA,
		.scl_pin	= SENSOR_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data sensor_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device sensor_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &sensor_i2c_pdata,
};

static struct i2c_board_info sensor_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("bma250", ACCEL_I2C_ADDRESS),
		.type = "bma250",
	},
	[1] = {
		I2C_BOARD_INFO("bmm050", ECOM_I2C_ADDRESS),
		.type = "bmm050",
	},
};

static void __init e0_init_i2c_sensor(int bus_num)
{
	sensor_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, sensor_i2c_pin[0], &sensor_i2c_bdinfo[0]);
	lge_init_gpio_i2c_pin(&sensor_i2c_pdata, sensor_i2c_pin[1], &sensor_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, sensor_i2c_bdinfo, ARRAY_SIZE(sensor_i2c_bdinfo));

	platform_device_register(&sensor_i2c_device);
}
#endif

/* proximity */

extern int bu61800_ldo_enable(struct device *dev, unsigned num, unsigned enable);

static int prox_power_set(unsigned char onoff)
{	
   if(onoff == 1) {
		bu61800_ldo_enable(NULL,1,1);
	} else {
		bu61800_ldo_enable(NULL,1,0);
	}

	printk("[Proximity] %s() : Power %s\n",__FUNCTION__, onoff ? "On" : "Off");

	return 0;
}

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 0,
	.operation_mode		= 0,
	.debounce	 = 0,
	.cycle = 2,
};

static struct i2c_board_info prox_i2c_bdinfo = {
	I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
	.type = "proximity_gp2ap",
	.platform_data = &proxi_pdata,
};

static struct gpio_i2c_pin proxi_i2c_pin = {
	.sda_pin	= PROXI_GPIO_I2C_SDA,
	.scl_pin	= PROXI_GPIO_I2C_SCL,
	.reset_pin	= 0,
	.irq_pin	= PROXI_GPIO_DOUT,
};

static struct i2c_gpio_platform_data proxi_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device proxi_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &proxi_i2c_pdata,
};

static void __init e0_init_i2c_prox(int bus_num)
{
	proxi_i2c_device.id = bus_num;

	lge_init_gpio_i2c_pin(&proxi_i2c_pdata, proxi_i2c_pin, &prox_i2c_bdinfo);

	i2c_register_board_info(bus_num, &prox_i2c_bdinfo, 1);
	platform_device_register(&proxi_i2c_device);
}

void __init lge_add_input_devices(void)
{
	platform_add_devices(e0_input_devices, ARRAY_SIZE(e0_input_devices));
	platform_add_devices(e0_gpio_input_devices, ARRAY_SIZE(e0_gpio_input_devices));	
	lge_add_gpio_i2c_device(e0_init_i2c_touch);

#ifdef CONFIG_LGE_DIAGTEST
	platform_add_devices(e0_ats_input_devices, ARRAY_SIZE(e0_ats_input_devices));
#endif

	lge_add_gpio_i2c_device(e0_init_i2c_sensor);

	lge_add_gpio_i2c_device(e0_init_i2c_prox);
	
}
