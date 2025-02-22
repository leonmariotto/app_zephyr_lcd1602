#define DT_DRV_COMPAT mine_aip31068l

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

#include "aip31068l.h"

LOG_MODULE_REGISTER(aip31068l, CONFIG_I2C_LOG_LEVEL);

/*************************************/
/* Per-device configuration and data */
/*************************************/

struct aip31068l_config {
#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)
	struct i2c_dt_spec i2c_config;
#endif
    uint8_t cursor;
    uint8_t blink;
};

struct aip31068l_transfer_function {
	int (*read_reg)(const struct device *dev, uint8_t reg_addr,
			uint8_t *value);
	int (*write_reg)(const struct device *dev, uint8_t reg_addr,
			 uint8_t value);
	int (*update_reg)(const struct device *dev, uint8_t reg_addr,
			  uint8_t mask, uint8_t value);
};

struct aip31068l_data {
	const struct device *bus;
	const struct aip31068l_transfer_function *hw_tf;
    uint8_t cursor_pos;
};

/*************************************/
/* Hardware tranfert functions       */
/*************************************/

static int aip31068l_i2c_read_reg(const struct device *dev, uint8_t reg_addr,
				uint8_t *value)
{
	const struct aip31068l_config *cfg = dev->config;

	return i2c_reg_read_byte_dt(&cfg->i2c_config, reg_addr, value);
}

static int aip31068l_i2c_write_reg(const struct device *dev, uint8_t reg_addr,
				uint8_t value)
{
	const struct aip31068l_config *cfg = dev->config;
    uint8_t tx_buf[2] = {reg_addr, value};
    int r = i2c_write_dt(&cfg->i2c_config, tx_buf, 2);
    return r;
}

static int aip31068l_i2c_update_reg(const struct device *dev, uint8_t reg_addr,
				  uint8_t mask, uint8_t value)
{
	const struct aip31068l_config *cfg = dev->config;

	return i2c_reg_update_byte_dt(&cfg->i2c_config, reg_addr, mask, value);
}

static const struct aip31068l_transfer_function aip31068l_i2c_transfer_fn = {
	.read_reg  = aip31068l_i2c_read_reg,
	.write_reg  = aip31068l_i2c_write_reg,
	.update_reg = aip31068l_i2c_update_reg,
};

/*************************************/
/* Driver functions                  */
/*************************************/

// TODO ALL PARAMETERS IN DTS
/* Init function called by zephyr driver init */
static int aip31068l_init(const struct device *dev)
{
    int r = 0;
	const struct aip31068l_config *cfg = dev->config;
	struct aip31068l_data *data = dev->data;

	if (!device_is_ready(cfg->i2c_config.bus)) {
		LOG_ERR("Bus device is not ready");
		r = -ENODEV;
	}
    if (r == 0) {
	    data->hw_tf = &aip31068l_i2c_transfer_fn;

        // [0x7C 0x80 0x28] # Function set (2 rows)
        r = data->hw_tf->write_reg(dev, 0x80, 0x28);
    }
    if (r == 0) {
        // [0x7C 0x80 0x0C] # Display on
        // r = data->hw_tf->write_reg(dev, 0x80, 0x0C);
        // [0x7C 0x80 0x0E] # Display on with cursor
        LOG_DBG("Init display cursor=%d blink=%d", cfg->cursor, cfg->blink);
        uint8_t cmd = 0x0C;
        if (cfg->cursor)
            cmd |= 0x02;
        if (cfg->blink)
            cmd |= 0x01;
        r = data->hw_tf->write_reg(dev, 0x80, cmd);
    }
    if (r == 0) {
        // [0x7C 0x80 0x06] # Init text direction
        r = data->hw_tf->write_reg(dev, 0x80, 0x06);
    }
    if (r == 0) {
        // [0x7C 0x80 0x01] # Clear: write space everywhere and move cursor to origin position.
        r = data->hw_tf->write_reg(dev, 0x80, 0x01);
    }
    data->cursor_pos = 0;
    return (r);
}


/* Exported drivers functions  */

int aip31068l_putc(const struct device *dev, char c)
{
	struct aip31068l_data *data = dev->data;
    data->cursor_pos++;
    // [0x7C 0x40 0x42] # Write ASCII 0x42 in current position and move next char.
    return data->hw_tf->write_reg(dev, 0x40, (uint8_t)c);
    
}

static int aip31068l_restore_cursor(const struct device *dev)
{
	struct aip31068l_data *data = dev->data;
    uint8_t column = data->cursor_pos % 40;
    uint8_t row = (data->cursor_pos >= 40);
    data->cursor_pos = 0;
    return aip31068l_cursor_set(dev, column, row);
}

int aip31068l_clear_screen(const struct device *dev)
{
	struct aip31068l_data *data = dev->data;
    // [0x7C 0x80 0x01] # Clear: write space everywhere and move cursor to origin position.
    int r = data->hw_tf->write_reg(dev, 0x80, 0x01);

    if (r == 0) {
        r = aip31068l_restore_cursor(dev);
    }
    return (r);
}

int aip31068l_cursor_home(const struct device *dev)
{
	struct aip31068l_data *data = dev->data;
    data->cursor_pos = 0;
    // [0x7C 0x80 0x01] # Clear display: write space everywhere and move cursor to origin position.
    return (data->hw_tf->write_reg(dev, 0x80, 0x02));
}

int aip31068l_cursor_set(const struct device *dev, uint8_t column, uint8_t row)
{
	struct aip31068l_data *data = dev->data;
    int r = 0;
    uint8_t pos = 0;
    if (column > 15 || row > 1) {
		LOG_ERR("Invalid parameters column=%d, row=%d", pos, row);
        r = -EINVAL;
    }
    else {
        pos = column + row * 40;
    }
    while (data->cursor_pos < pos && r == 0) {
        // [0x7C 0x80 0x14] # Cursor or display shift, S/C=0, R/L=1, to the right
        r = data->hw_tf->write_reg(dev, 0x80, 0x14);
        data->cursor_pos++;
    }
    while (data->cursor_pos > pos && r == 0) {
        // [0x7C 0x80 0x10] # Cursor or display shift, S/C=0, R/L=0, to the left
        r = data->hw_tf->write_reg(dev, 0x80, 0x10);
        data->cursor_pos--;
    }
    return r;
}

int aip31068l_display_shift(const struct device *dev, int8_t column)
{
    int r = 0;
	struct aip31068l_data *data = dev->data;
    uint8_t shift = (column < 0 ? -column : column);

    for (int i = 0 ; i < shift && r == 0 ; i++) {
        // [0x7C 0x80 0x14] # Cursor or display shift, S/C=0, R/L=1, to the right
        uint8_t cmd = 0x18;
        if (column > 0) {
            cmd |= 0x4;
        }
        r = data->hw_tf->write_reg(dev, 0x80, cmd);
    }

    return (r);
}

int aip31068l_set_cg(const struct device *dev, uint8_t id, uint8_t *value)
{
	struct aip31068l_data *data = dev->data;
    int r = 0;
    if (id > 8) {
		LOG_ERR("Invalid parameters id=%d", id);
        r = -EINVAL;
    }
    if (r == 0) {
        // [0x7C 0x80 0x40] # Set CGRAM address for custom char 0
        // [0x7C 0x80 0x48] # Set CGRAM address for custom char 8
        r = data->hw_tf->write_reg(dev, 0x80, 0x40 | (id * 8));
    }
    for (int i = 0; i < 8 && r == 0 ; i++) {
        r = data->hw_tf->write_reg(dev, 0x40, value[i]);
    }
    if (r == 0) {
        // [0x7C 0x80 0x00] # Set DDRAM address 0
        // For 2 lines mode DDRAM addrs are 0x00-0x27 0x40-0x67
        uint8_t pos = (data->cursor_pos >= 40 ? 0x40 : 0) + data->cursor_pos % 40;
        r = data->hw_tf->write_reg(dev, 0x80, 0x80 | pos);
    }
    return r;
}

#define AIP31068L_INIT(i)   \
	static const struct aip31068l_config aip31068l_config_##i = { \
		.i2c_config = I2C_DT_SPEC_GET(DT_DRV_INST(i)),	\
		.cursor = DT_INST_PROP(i, cursor),	\
		.blink = DT_INST_PROP(i, blink),	\
	}; \
	static struct aip31068l_data aip31068l_data_##i; \
	DEVICE_DT_INST_DEFINE(i, \
                  &aip31068l_init, \
                  PM_DEVICE_DT_INST_GET(i), \
			      &aip31068l_data_##i, \
                  &aip31068l_config_##i, \
			      POST_KERNEL, \
                  CONFIG_I2C_INIT_PRIORITY, \
                  NULL);

DT_INST_FOREACH_STATUS_OKAY(AIP31068L_INIT)

