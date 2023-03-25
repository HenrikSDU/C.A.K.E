#include <stdio.h>

void cursor_pos(int x, int y)
{
	printf("\033[%d;%dH", x, y);
}