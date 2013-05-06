#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#include <mach/msm_battery.h>
#include <mach/board_lge.h>

#include "devices-msm7x2xa.h"

static u32 e0_spg_batt_capacity(u32 current_soc)
{
	if(current_soc > 100)
		current_soc = 100;

	return current_soc;
}

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design	= 3200,
	.voltage_max_design	= 4200,
	.avail_chg_sources		= AC_CHG | USB_CHG,
	.batt_technology		= POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity		= &e0_spg_batt_capacity,
};


static struct platform_device msm_batt_device = {
	.name               = "msm-battery",
	.id                 = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

/* LED platform data */
static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
};

/* misc platform devices */
static struct platform_device *e0_misc_devices[] __initdata = {
	&msm_batt_device,
	&msm_device_pmic_leds,
};

/* main interface */
void __init lge_add_misc_devices(void)
{
	platform_add_devices(e0_misc_devices, ARRAY_SIZE(e0_misc_devices));

	/* QCT native Vibrator enable [mach-msm/msm_vibrator.c] */
	msm_init_pmic_vibrator();
}

