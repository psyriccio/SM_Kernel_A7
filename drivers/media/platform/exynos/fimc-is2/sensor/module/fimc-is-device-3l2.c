/*
 * Samsung Exynos5 SoC series Sensor driver
 *
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <mach/exynos-fimc-is-sensor.h>

#include "fimc-is-hw.h"
#include "fimc-is-core.h"
#include "fimc-is-device-sensor.h"
#include "fimc-is-resourcemgr.h"
#include "fimc-is-dt.h"
#include "fimc-is-device-3l2.h"

#define SENSOR_NAME "S5K3L2"

static struct fimc_is_sensor_cfg config_3l2[] = {
	/* 4144x3106@30fps */
	FIMC_IS_SENSOR_CFG(4144, 3106, 30, 24, 0),
	/* 4144x2332@30fps */
	FIMC_IS_SENSOR_CFG(4144, 2332, 30, 24, 1),
	/* 4128x3096@30fps */
	FIMC_IS_SENSOR_CFG(4128, 3096, 30, 24, 2),
	/* 4128x2322@30fps */
	FIMC_IS_SENSOR_CFG(4128, 2322, 30, 24, 3),
	/* 2072x1166@60fps */
	FIMC_IS_SENSOR_CFG(2072, 1166, 60, 12, 4),
	/* 1008x754@120fps */
	FIMC_IS_SENSOR_CFG(1008, 754, 120, 9, 5),
	/* 1008x568@120fps */
	FIMC_IS_SENSOR_CFG(1008, 568, 120, 9, 6),
	/* 2072x1552@15fps */
	FIMC_IS_SENSOR_CFG(2072, 1552, 15, 24, 7),
	/* 2072x1552@30fps */
	FIMC_IS_SENSOR_CFG(2072, 1552, 30, 19, 8),
	/* 2072x1552@60fps */
	FIMC_IS_SENSOR_CFG(2072, 1552, 60, 24, 9),
	/* 2072x1162@60fps */
	FIMC_IS_SENSOR_CFG(2072, 1162, 60, 12, 10),
};

static struct fimc_is_vci vci_3l2[] = {
	{
		.pixelformat = V4L2_PIX_FMT_SBGGR10,
		.config = {{0, HW_FORMAT_RAW10}, {1, HW_FORMAT_UNKNOWN}, {2, HW_FORMAT_USER}, {3, 0}}
	}, {
		.pixelformat = V4L2_PIX_FMT_SBGGR12,
		.config = {{0, HW_FORMAT_RAW10}, {1, HW_FORMAT_UNKNOWN}, {2, HW_FORMAT_USER}, {3, 0}}
	}, {
		.pixelformat = V4L2_PIX_FMT_SBGGR16,
		.config = {{0, HW_FORMAT_RAW10}, {1, HW_FORMAT_UNKNOWN}, {2, HW_FORMAT_USER}, {3, 0}}
	}
};


static int sensor_3l2_init(struct v4l2_subdev *subdev, u32 val)
{
	int ret = 0;
	struct fimc_is_module_enum *module;

	BUG_ON(!subdev);

	module = (struct fimc_is_module_enum *)v4l2_get_subdevdata(subdev);

	pr_info("[MOD:D:%d] %s(%d)\n", module->sensor_id, __func__, val);

	return ret;
}

static const struct v4l2_subdev_core_ops core_ops = {
	.init = sensor_3l2_init
};

static const struct v4l2_subdev_ops subdev_ops = {
	.core = &core_ops
};

#ifdef CONFIG_OF
static int sensor_3l2_power_setpin(struct platform_device *pdev,
	struct exynos_platform_fimc_is_module *pdata)
{
	struct device *dev;
	struct device_node *dnode;
	int gpio_reset = 0;
	int gpio_none = 0;
	int gpio_af_pwr_en = 0;
	int gpio_ois_pwr_en = 0;
	int gpio_ois_reset = 0;

	BUG_ON(!pdev);

	dev = &pdev->dev;
	dnode = dev->of_node;

	dev_info(dev, "%s E v4\n", __func__);

	gpio_reset = of_get_named_gpio(dnode, "gpio_reset", 0);
	if (!gpio_is_valid(gpio_reset)) {
		dev_err(dev, "failed to get PIN_RESET\n");
		return -EINVAL;
	} else {
		gpio_request_one(gpio_reset, GPIOF_OUT_INIT_LOW, "CAM_GPIO_OUTPUT_LOW");
		gpio_free(gpio_reset);
	}

	gpio_af_pwr_en = of_get_named_gpio(dnode, "gpio_af_pwr_en", 0);
	if (!gpio_is_valid(gpio_af_pwr_en)) {
		err("failed to get af power en gpio");
	} else {
		gpio_request_one(gpio_af_pwr_en, GPIOF_OUT_INIT_LOW, "CAM_GPIO_OUTPUT_LOW");
		gpio_free(gpio_af_pwr_en);
	}

	gpio_ois_pwr_en = of_get_named_gpio(dnode, "gpio_ois_pwr_en", 0);
	if (!gpio_is_valid(gpio_ois_pwr_en)) {
		err("failed to get ois power en gpio");
	} else {
		gpio_request_one(gpio_ois_pwr_en, GPIOF_OUT_INIT_LOW, "CAM_GPIO_OUTPUT_LOW");
		gpio_free(gpio_ois_pwr_en);
	}

	gpio_ois_reset = of_get_named_gpio(dnode, "gpio_ois_reset", 0);
	if (!gpio_is_valid(gpio_ois_reset)) {
		err("failed to get ois reset gpio");
	} else {
		gpio_request_one(gpio_ois_reset, GPIOF_OUT_INIT_LOW, "CAM_GPIO_OUTPUT_LOW");
		gpio_free(gpio_ois_reset);
	}

	SET_PIN_INIT(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON);
	SET_PIN_INIT(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF);
	SET_PIN_INIT(pdata, SENSOR_SCENARIO_VISION, GPIO_SCENARIO_ON);
	SET_PIN_INIT(pdata, SENSOR_SCENARIO_VISION, GPIO_SCENARIO_OFF);
	SET_PIN_INIT(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON);
	SET_PIN_INIT(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF);

#if defined(CONFIG_MACH_SMDK7580)
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_reset, "sen_rst low", PIN_OUTPUT, 0, 0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "pin", PIN_FUNCTION, 0, 0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_reset, "sen_rst high", PIN_OUTPUT, 1, 0);

	/* BACK CAEMRA - POWER OFF */
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_reset, "sen_rst", PIN_RESET, 0, 0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_reset, "sen_rst input", PIN_INPUT, 0 ,0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "pin", PIN_FUNCTION, 1, 0);
#else
	/* BACK CAMERA - POWER ON */
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 1, 2000);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "VDD_CAM_SENSOR_A2P8", PIN_REGULATOR, 1, 0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "VDD_CAM_1P2", PIN_REGULATOR, 1, 0);
#if defined(CONFIG_REGULATOR_SM5703)
	SET_PIN_VOLTAGE(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 1, 2000, 2800000);
#else
	if (gpio_is_valid(gpio_af_pwr_en)) {  /* VDDAF_2.8V_CAM */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_af_pwr_en, NULL, PIN_OUTPUT, 1, 2000);
	} else {
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 1, 2000);
	}
#endif
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 1, 2000);
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_ois_reset, NULL, PIN_OUTPUT, 1, 2000);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_none, "pin", PIN_FUNCTION, 0, 3000);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_ON, gpio_reset, NULL, PIN_OUTPUT, 1, 0);

	/* BACK CAMERA - POWER OFF */
	if (gpio_is_valid(gpio_af_pwr_en)) {   /* VDDAF_2.8V_CAM */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_af_pwr_en, NULL, PIN_OUTPUT, 0, 10);
	} else {
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 0, 10);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "pin", PIN_FUNCTION, 1, 10);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_reset, NULL, PIN_OUTPUT, 0, 10);
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_ois_reset, NULL, PIN_OUTPUT, 0, 10);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 0, 0);
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 0, 10);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "VDD_CAM_1P2", PIN_REGULATOR, 0, 0);
	SET_PIN(pdata, SENSOR_SCENARIO_NORMAL, GPIO_SCENARIO_OFF, gpio_none, "VDD_CAM_SENSOR_A2P8", PIN_REGULATOR, 0, 0);

	/* READ_ROM - POWER ON */
	SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 1, 0);
#if defined(CONFIG_CAMERA_ACTUATOR_DW9807) || defined(CONFIG_CAMERA_ACTUATOR_AK7371)
#if defined(CONFIG_REGULATOR_SM5703)
	SET_PIN_VOLTAGE(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 1, 0, 2800000);
#endif
	if (gpio_is_valid(gpio_af_pwr_en)) {  /* VDDAF_2.8V_CAM */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_af_pwr_en, NULL, PIN_OUTPUT, 1, 10000);
	} else {
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 1, 10000);
	}
#endif
#if defined(CONFIG_CAMERA_EEPROM_SUPPORT_OIS) && defined(CONFIG_SEC_FACTORY)
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 1, 2000);
	}
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON, gpio_ois_reset, NULL, PIN_OUTPUT, 1, 2000);
	}
#endif

	/* READ_ROM - POWER OFF */
	SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 0, 0);
#if defined(CONFIG_CAMERA_ACTUATOR_DW9807) || defined(CONFIG_CAMERA_ACTUATOR_AK7371)
	if (gpio_is_valid(gpio_af_pwr_en)) {  /* VDDAF_2.8V_CAM */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF, gpio_af_pwr_en, NULL, PIN_OUTPUT, 0, 0);
	} else {
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF, gpio_none, "VDDAF_2.8V_CAM", PIN_REGULATOR, 0, 0);
	}
#endif
#if defined(CONFIG_CAMERA_EEPROM_SUPPORT_OIS) && defined(CONFIG_SEC_FACTORY)
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF, gpio_ois_reset, NULL, PIN_OUTPUT, 0, 10);
	}
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 0, 10);
	}
#endif

#if defined(CONFIG_OIS_USE)
	/* BACK CAMERA - OIS FACTORY ON */
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_ON, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 1, 2000);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_ON, gpio_af_pwr_en, NULL, PIN_OUTPUT, 1, 2000);
	SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_ON, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 1, 2000);
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_ON, gpio_ois_reset, NULL, PIN_OUTPUT, 1, 2000);
	}
	/* BACK CAMERA - OIS FACTORY OFF */
	SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_OFF, gpio_af_pwr_en, NULL, PIN_OUTPUT, 0, 10);
	if (gpio_is_valid(gpio_ois_reset)) {  /* OIS_RESET */
		SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_OFF, gpio_ois_reset, NULL, PIN_OUTPUT, 0, 10);
	}
	SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_OFF, gpio_none, "VDDIO_1.8V_CAM", PIN_REGULATOR, 0, 0);
	if (gpio_is_valid(gpio_ois_pwr_en)) {  /* VDD_OIS_2P8, VDD_OIS_IO_1P8 */
		SET_PIN(pdata, SENSOR_SCENARIO_OIS_FACTORY, GPIO_SCENARIO_OFF, gpio_ois_pwr_en, NULL, PIN_OUTPUT, 0, 10);
	}
#endif
#endif
	dev_info(dev, "%s X v4\n", __func__);
	return 0;
}
#endif /* CONFIG_OF */


int sensor_3l2_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct fimc_is_core *core;
	struct v4l2_subdev *subdev_module;
	struct fimc_is_module_enum *module;
	struct fimc_is_device_sensor *device;
	struct sensor_open_extended *ext;
	struct exynos_platform_fimc_is_module *pdata;
	struct device *dev;

	BUG_ON(!fimc_is_dev);

	core = (struct fimc_is_core *)dev_get_drvdata(fimc_is_dev);
	if (!core) {
		probe_err("core device is not yet probed");
		return -EPROBE_DEFER;
	}

	dev = &pdev->dev;

#ifdef CONFIG_OF
	fimc_is_sensor_module_parse_dt(pdev, sensor_3l2_power_setpin);
#endif

	pdata = dev_get_platdata(dev);
	device = &core->sensor[pdata->id];

	subdev_module = kzalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
	if (!subdev_module) {
		probe_err("subdev_module is NULL");
		ret = -ENOMEM;
		goto p_err;
	}

	module = &device->module_enum[atomic_read(&core->resourcemgr.rsccount_module)];
	atomic_inc(&core->resourcemgr.rsccount_module);
	clear_bit(FIMC_IS_MODULE_GPIO_ON, &module->state);
	module->pdata = pdata;
	module->pdev = pdev;
	module->sensor_id = SENSOR_NAME_S5K3L2;
	module->subdev = subdev_module;
	module->device = pdata->id;
	module->client = NULL;
	module->active_width = 4128;
	module->active_height = 3096;
	module->margin_left = 8;
	module->margin_right = 8;
	module->margin_top = 6;
	module->margin_bottom = 4;
	module->pixel_width = module->active_width + 16;
	module->pixel_height = module->active_height + 10;
	module->max_framerate = 120;
	module->position = SENSOR_POSITION_REAR;
	module->mode = CSI_MODE_CH0_ONLY;
	module->lanes = CSI_DATA_LANES_4;
	module->vcis = ARRAY_SIZE(vci_3l2);
	module->vci = vci_3l2;
	module->sensor_maker = "SLSI";
	module->sensor_name = "S5K3L2";
	module->setfile_name = "setfile_3l2.bin";
	module->cfgs = ARRAY_SIZE(config_3l2);
	module->cfg = config_3l2;
	module->ops = NULL;
	module->private_data = NULL;

	ext = &module->ext;
	ext->mipi_lane_num = module->lanes;
	ext->I2CSclk = I2C_L0;

	ext->sensor_con.product_name = SENSOR_NAME_S5K3L2;
	ext->sensor_con.peri_type = SE_I2C;
	ext->sensor_con.peri_setting.i2c.channel = pdata->sensor_i2c_ch;;
	ext->sensor_con.peri_setting.i2c.slave_address = pdata->sensor_i2c_addr;
	ext->sensor_con.peri_setting.i2c.speed = 400000;

	if (pdata->af_product_name !=  ACTUATOR_NAME_NOTHING) {
		ext->actuator_con.product_name = pdata->af_product_name;
		ext->actuator_con.peri_type = SE_I2C;
		ext->actuator_con.peri_setting.i2c.channel = pdata->af_i2c_ch;
		ext->actuator_con.peri_setting.i2c.slave_address = pdata->af_i2c_addr;
		ext->actuator_con.peri_setting.i2c.speed = 400000;
	}

	if (pdata->flash_product_name != FLADRV_NAME_NOTHING) {
		ext->flash_con.product_name = pdata->flash_product_name;
		ext->flash_con.peri_type = SE_GPIO;
		ext->flash_con.peri_setting.gpio.first_gpio_port_no = pdata->flash_first_gpio;
		ext->flash_con.peri_setting.gpio.second_gpio_port_no = pdata->flash_second_gpio;
	}

	ext->from_con.product_name = FROMDRV_NAME_NOTHING;
	ext->companion_con.product_name = COMPANION_NAME_NOTHING;

	if  (pdata->ois_product_name !=  OIS_NAME_NOTHING) {
		ext->ois_con.product_name = pdata->ois_product_name;;
		ext->ois_con.peri_type = SE_I2C;
		ext->ois_con.peri_setting.i2c.channel =  pdata->ois_i2c_ch;;
		ext->ois_con.peri_setting.i2c.slave_address =  pdata->ois_i2c_addr;;
		ext->ois_con.peri_setting.i2c.speed = 400000;
	}

	v4l2_subdev_init(subdev_module, &subdev_ops);
	v4l2_set_subdevdata(subdev_module, module);
	v4l2_set_subdev_hostdata(subdev_module, device);
	snprintf(subdev_module->name, V4L2_SUBDEV_NAME_SIZE, "sensor-subdev.%d", module->sensor_id);

p_err:
	probe_info("%s(%d)\n", __func__, ret);
	return ret;
}

static int sensor_3l2_remove(struct platform_device *pdev)
{
	int ret = 0;

	info("%s\n", __func__);

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id exynos_fimc_is_sensor_3l2_match[] = {
	{
		.compatible = "samsung,exynos5-fimc-is-sensor-3l2",
	},
	{},
};
#endif

static struct platform_driver sensor_3l2_driver = {
	.probe  = sensor_3l2_probe,
	.remove = sensor_3l2_remove,
	.driver = {
		.name   = SENSOR_NAME,
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = exynos_fimc_is_sensor_3l2_match,
#endif
	}
};

static int __init fimc_is_sensor_module_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&sensor_3l2_driver);
	if (ret)
		err("platform_driver_register failed: %d\n", ret);

	return ret;
}

static void __exit fimc_is_sensor_module_exit(void)
{
	platform_driver_unregister(&sensor_3l2_driver);
}
module_init(fimc_is_sensor_module_init);
module_exit(fimc_is_sensor_module_exit);

MODULE_AUTHOR("Gilyeon lim");
MODULE_DESCRIPTION("Sensor 3L2 driver");
MODULE_LICENSE("GPL v2");
