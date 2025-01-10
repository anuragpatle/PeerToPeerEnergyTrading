// jpeg_functions.h

#ifndef JPEG_FUNCTIONS_H
#define JPEG_FUNCTIONS_H

#include <stdint.h>

#if defined(LCD_MODULE_CMD_1)
for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++)
{
    tft.writecommand(lcd_st7789v[i].cmd);
    for (int j = 0; j < lcd_st7789v[i].len & 0x7f; j++)
    {
        tft.writedata(lcd_st7789v[i].data[j]);
    }

    if (lcd_st7789v[i].len & 0x80)
    {
        delay(120);
    }
}
#endif

// Function declarations
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos);
void renderJPEG(int xpos, int ypos);
void jpegInfo();

#endif // JPEG_FUNCTIONS_H
