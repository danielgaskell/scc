#include <symbos.h>

unsigned char TextBox_Pos(void* win, void* textbox) {
    Ctrl_Group* group = ((Window*)win)->controls;
    Ctrl_TextBox* box;
    unsigned short i = 2000;
    if (group == 0 || group->focusctrl != ((Ctrl*)textbox)->value)
        return 0;
    box = ((Ctrl*)textbox)->param;
    box->column = 32767;
    Key_Put(29);
    while (i--) { // time out after awhile, since there is a real chance of failure here
        if (box->column != 32767)
            return 1;
    }
    return 0;
}

void TextBox_Redraw(void* win, void* textbox) {
    Ctrl_Group* group = ((Window*)win)->controls;
    if (group == 0 || group->focusctrl != ((Ctrl*)textbox)->value)
        return;
    Key_Put(30);
}

void TextBox_Select(void* win, void* textbox, int cursor, int selection) {
    Ctrl_Group* group = ((Window*)win)->controls;
    Ctrl_TextBox* box;
    if (group == 0 || group->focusctrl != ((Ctrl*)textbox)->value)
        return;
    box = ((Ctrl*)textbox)->param;
    ((Ctrl_TextBox*)box)->column = cursor;
    ((Ctrl_TextBox*)box)->line = selection;
    Key_Put(31);
}
