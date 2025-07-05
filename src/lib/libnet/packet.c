char _nethost[65];

// Length is 1400 bytes because this is the maximum single send size for small
// WiFi chips (like the RSF3 M4 emulation).
_transfer char _netpacket[1400];
