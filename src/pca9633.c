#define DT_DRV_COMPAT mine_pca9633

#include <zephyr/init.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#include <zephyr/drivers/i2c.h>

#include "pca9633.h"

LOG_MODULE_REGISTER(pca9633, CONFIG_I2C_LOG_LEVEL);

/*************************************/
/* Per-device configuration and data */
/*************************************/

struct pca9633_config {
#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)
    struct i2c_dt_spec i2c_config;
#endif
};

struct pca9633_transfer_function {
    int (*read_reg)(const struct device *dev, uint8_t reg_addr,
            uint8_t *value);
    int (*write_reg)(const struct device *dev, uint8_t reg_addr,
            uint8_t value);
    int (*update_reg)(const struct device *dev, uint8_t reg_addr,
            uint8_t mask, uint8_t value);
};

struct pca9633_data {
    const struct device *bus;
    const struct pca9633_transfer_function *hw_tf;
};

/*************************************/
/* Hardware tranfert functions       */
/*************************************/

static int pca9633_i2c_read_reg(const struct device *dev, uint8_t reg_addr,
        uint8_t *value)
{
    const struct pca9633_config *cfg = dev->config;

    return i2c_reg_read_byte_dt(&cfg->i2c_config, reg_addr, value);
}

static int pca9633_i2c_write_reg(const struct device *dev, uint8_t reg_addr,
        uint8_t value)
{
    const struct pca9633_config *cfg = dev->config;
    uint8_t tx_buf[2] = {reg_addr, value};
    int r = i2c_write_dt(&cfg->i2c_config, tx_buf, 2);
    return r;
}

static int pca9633_i2c_update_reg(const struct device *dev, uint8_t reg_addr,
        uint8_t mask, uint8_t value)
{
    const struct pca9633_config *cfg = dev->config;

    return i2c_reg_update_byte_dt(&cfg->i2c_config, reg_addr, mask, value);
}

static const struct pca9633_transfer_function pca9633_i2c_transfer_fn = {
    .read_reg  = pca9633_i2c_read_reg,
    .write_reg  = pca9633_i2c_write_reg,
    .update_reg = pca9633_i2c_update_reg,
};

/*************************************/
/* Driver functions                  */
/*************************************/

int pca9633_set_rgb(const struct device *dev,
        uint8_t r,
        uint8_t g,
        uint8_t b)
{
    int ret = 0;
    struct pca9633_data *data = dev->data;
    uint8_t REG_RED   =      0x04;
    uint8_t REG_GREEN =      0x03;
    uint8_t REG_BLUE  =      0x02;

    if ((ret = data->hw_tf->write_reg(dev, REG_RED, r)) != 0
            || (ret = data->hw_tf->write_reg(dev, REG_GREEN, g) != 0)
            || (ret = data->hw_tf->write_reg(dev, REG_BLUE, b)) != 0) {
        LOG_DBG("%s: Error: set color return %d", __func__, ret);
    }
    return ret;
}


static int pca9633_init(const struct device *dev)
{
    struct pca9633_data *data = dev->data;
    const struct pca9633_config *cfg = dev->config;

    if (!device_is_ready(cfg->i2c_config.bus)) {
        LOG_ERR("Bus device is not ready");
        return -ENODEV;
    }
    data->hw_tf = &pca9633_i2c_transfer_fn;

    return 0;
}

int pca9633_configure(const struct device *dev)
{
    int r = 0;
    struct pca9633_data *data = dev->data;

    // Set MODE1 values
    r = data->hw_tf->write_reg(dev, REG_MODE1, 0);
    LOG_DBG("%s: Set MODE1 return %d", __func__, r);

    // Set MODE2 values
    r = data->hw_tf->write_reg(dev, REG_MODE2, 0x20);
    LOG_DBG("%s: Set MODE2 return %d", __func__, r);

    // Set LEDs controllable by both PWM and GRPPWM registers
    r = data->hw_tf->write_reg(dev, REG_OUTPUT, 0xFF);
    LOG_DBG("%s: Set REG_OUTPUT return %d", __func__, r);

    r = pca9633_set_rgb(dev, 255, 255, 255);
    LOG_DBG("%s: Set RGB return %d", __func__, r);

    return (r);
}


#define PCA9633_INIT(i)   \
    static const struct pca9633_config pca9633_config_##i = { \
        .i2c_config = I2C_DT_SPEC_GET(DT_DRV_INST(i)),	\
    }; \
    static struct pca9633_data pca9633_data_##i; \
    DEVICE_DT_INST_DEFINE(i, \
            &pca9633_init, \
            PM_DEVICE_DT_INST_GET(i), \
            &pca9633_data_##i, \
            &pca9633_config_##i, \
            POST_KERNEL, \
            CONFIG_I2C_INIT_PRIORITY, \
            NULL);

DT_INST_FOREACH_STATUS_OKAY(PCA9633_INIT)
