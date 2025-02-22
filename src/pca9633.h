#ifndef PCA9633_H
# define PCA9633_H

// 16 * 2 charactere screen
#define LCD_COLS        16
#define LCD_ROWS        2

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

int pca9633_configure(const struct device *dev);
int pca9633_set_rgb(const struct device *dev,
    uint8_t r,
    uint8_t g,
    uint8_t b);

#endif /* PCA9633_H */
