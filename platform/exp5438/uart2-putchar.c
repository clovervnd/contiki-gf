#include <stdio.h>
#include "dev/uart2.h"
#include "uart2-putchar.h"
#include "lcd.h"

int
putchar2(int c)
{
  uart2_writeb((char)c);

  lcd_write_char((char)c);
  return c;
}
