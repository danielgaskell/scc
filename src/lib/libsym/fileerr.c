#include <string.h>
#include <symbos.h>

char* _fileerrstr[50] = {
    "Undefined", "Device does not exist", "",
    "Device not initialised",
    "Media is damaged",
    "Partition does not exist",
    "Unsupported media or partition",
    "Error while sector read/write",
    "Error while positioning",
    "Abort while volume access",
    "Unknown volume error",
    "No free filehandler",
    "Device does not exist",
    "Path does not exist",
    "File does not exist",
    "Access is forbidden",
    "Invalid path or filename",
    "Filehandler does not exist",
    "Device slot already occupied",
    "Error in file organisation",
    "Invalid destination name",
    "File/path already exist",
    "Wrong sub command code",
    "Wrong attribute",
    "Directory full",
    "Media full",
    "Media is write protected",
    "Device is not ready",
    "Directory is not empty",
    "Invalid destination device",
    "Not supported by file system",
    "Unsupported device",
    "File is read only",
    "Device channel not available",
    "Destination is not a directory",
    "Destination is not a file"};

void File_ErrMsg(void* modalWin) {
    char textbuf[24];
    if (_fileerr >= 15 && _fileerr <= 49) {
        strcpy(textbuf, "Disc error (Code ");
        itoa(_fileerr - 16, textbuf + 17, 10);
        strcat(textbuf, "):");
        MsgBox(textbuf, _fileerrstr[_fileerr - 15], 0, COLOR_BLACK, BUTTON_OK, 0, modalWin);
    }
}
