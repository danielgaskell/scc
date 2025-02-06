#include <symbos.h>

_transfer unsigned short _systype = 255;
unsigned short _systypes[32] = { // type is a 5-bit number; extra zero bytes here so future types return TYPE_OTHER
    TYPE_CPC464, TYPE_CPC664, TYPE_CPC6128, TYPE_CPC464PLUS, TYPE_CPC6128PLUS, TYPE_OTHER,
    TYPE_ENTERPRISE,
    TYPE_MSX1, TYPE_MSX2, TYPE_MSX2PLUS, TYPE_MSXTURBOR, TYPE_OTHER,
    TYPE_PCW8, TYPE_PCW9, TYPE_PCW16,
    TYPE_NC100, TYPE_NC150, TYPE_NC200,
    TYPE_SVM};

unsigned short Sys_Type(void) {
    if (_systype == 255) {
        Sys_GetConfig((char*)&_systype, 82, 1);
        _systype = _systypes[_systype & 0x1F];

    }
    return _systype;
}
