#include <symbos.h>

unsigned char Mouse_Dragging(unsigned char delay) {
    unsigned long counter_stop = Sys_Counter() + delay;
    unsigned short old_mousex = Mouse_X();
    unsigned short old_mousey = Mouse_Y();
    while (Mouse_Buttons() != 0 &&
           counter_stop > Sys_Counter() &&
           Mouse_X() == old_mousex &&
           Mouse_Y() == old_mousey);
    return Mouse_Buttons();
}
