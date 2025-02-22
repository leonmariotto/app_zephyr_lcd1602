#ifndef AIP31068L_H
# define AIP31068L_H

/*
** @func aip31068l_putc
** @summary
** write a char at the current cursor place
** the cursor is then shift to the left (if direction shift is default).
** Char 0-7 is identifier for custom charactere in CGRAM
*/
int aip31068l_putc(const struct device *dev, char c);

/*
** @func aip31068l_cursor_set
** @summary
** set the cursor position to column - row.
*/
int aip31068l_cursor_set(const struct device *dev, uint8_t column, uint8_t row);

/*
** @func aip31068l_cursor_home
** @summary
** set the cursor position to home (0)
*/
int aip31068l_cursor_home(const struct device *dev);

/*
** @func aip31068l_set_cg
** @summary
** write into aip31068l CGRAM to store a custome char.
** The char is then writtable with aip31068l_putc(dev, id)
** Each char is a 8 bytes array. The 3 last bit is ignored for each char.
** For example the following array draw a smiley, we can see the pattern :
** uint8_t custom_char[8] = {
**     0b11100000,
**     0b11110001,
**     0b11110001,
**     0b11100000,
**     0b11100000,
**     0b11110001,
**     0b11101110,
**     0b11100000,
** };
*/
int aip31068l_set_cg(const struct device *dev, uint8_t id, uint8_t *value);

/*
** @func aip31068l_clear_screen
** @summary
** Clear the screen, BUT DO NOT move the cursor
*/
int aip31068l_clear_screen(const struct device *dev);

/*
** @func aip31068l_display_shift
** @summary
** Shift the display of n column. The sign of column define the direction.
** If column is positive column move on the left (common direction to display a msg to be read)
** Else column move on the right.
*/
int aip31068l_display_shift(const struct device *dev, int8_t column);

#endif /* AIP32068L_H */

