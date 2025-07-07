#include <symbos.h>
#include <network.h>

char* _neterrstr[27] = {
    "Undefined",
    "No network hardware",
    "No IP configuration",
    "Function not supported",
    "Unknown hardware error",
    "Offline / no daemon",
    "WiFi error", "",
    "Out of free sockets",
    "Socket does not exist",
    "Wrong socket type",
    "Socket already in use", "", "", "", ""
    "Invalid domain",
    "Connection timeout",
    "Recursion not supported",
    "Truncated response",
    "Packet too large", "", "", "",
    "Connection not established",
    "File error",
    "Unexpected response"};

void Net_ErrMsg(void* modalWin) {
    char* str;
    if (_neterr) {
        if (_neterr > 0 && _neterr <= 26)
            str = _neterrstr[_neterr];
        else
            str = _neterrstr[0];
        MsgBox("Network error:", str, "", COLOR_BLACK, BUTTON_OK, 0, modalWin);
    }
}
