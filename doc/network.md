# Network Library

**Note: The enhanced network library as described below is currently only available in development builds of SymbOS, but `Net_Init()` and the raw TCP/UDP/DNS functions can be found in older releases in the header file `symbos/network.h`.**

**Note:** All raw TCP/UDP/DNS functions are thread-safe, but HTTP functions are not. Only one thread should make HTTP requests at a time.

## Contents

* [Using the library](#using-the-library)
* [Function reference](#http-functions)
	* [HTTP functions](#http-functions)
	* [TCP functions](#tcp-functions)
	* [UDP functions](#udp-functions)
	* [Helper functions](#helper-functions)
* [Reference](#reference)
	* [Error codes](#error-codes)

## Using the library

To use the library, include the `network.h` header:

```c
#include <network.h>
```

Additionally, use the `-lnet` option when compiling to specify that the executable should be linked with the network library `libnet.a`:

```bash
cc source.c -net
```

Network capabilities are only available if an appropriate network daemon is running. Use `Net_Init()` to initialize the network and connect to the daemon.

Network errors are recorded in the global variable `_neterr`, documented [below](#error-codes).

### Net_Init()

```c
signed char Net_Init(void);
```

Initializes the network interface, if present. This should be called before using any other network functions.

*Return value*: On success, sets `_netpid` to the process ID of the network daemon and returns 0. On failure, sets `_neterr` and returns -1.

## HTTP functions

### HTTP_GET()

```c
int HTTP_GET(char* url, char* dest, unsigned short maxlen, char* headers,
             unsigned char bodyonly);
```

Executes a complete HTTP GET request, downloading whatever content is returned from the URL `url` into the buffer at the address `dest`, which should be in the **data** or **transfer** segment. A maximum of `maxlen` characters will be written, discarding any extra. If `bodyonly` is nonzero, the HTTP headers will be stripped from the response, leaving only the response data. (Note that `maxlen` initially includes the space required to retrieve the headers, so when `bodyonly` is nonzero, the final length of the retrieved response may be less than `maxlen`.)

`headers` can be optionally used to specify additional HTTP header requests in the outgoing request (use 0 for no custom headers). Each custom header line should be followed by the HTTP-standard line break `\r\n` (even the last one):

```c
status = HTTP_GET("http://numbersapi.com", buffer, sizeof(buffer),
                  "Accept: text/plain\r\nAccept-Language: en-US\r\n", 1);
```

*Return value*: On success, writes the response to `buffer` and returns the HTTP response status code (e.g., 200 "OK", 404 "Not Found"). On failure, sets `_neterr` and returns -1. If a response is received but it does not contain a valid HTTP response status code, returns 0.

### HTTP_POST()

```c
int HTTP_POST(char* url, char* dest, unsigned short maxlen, char* headers,
              char* body, unsigned short bodylen, unsigned char bodyonly);
```

Executes a complete HTTP POST request, downloading whatever content is returned from the URL `url` into the buffer at the address `dest`, which should be in the **data** or **transfer** segment. A maximum of `maxlen` characters will be written, discarding any extra. If `bodyonly` is nonzero, the HTTP headers will be stripped from the response, leaving only the response data. (Note that `maxlen` initially includes the space required to retrieve the headers, so when `bodyonly` is nonzero, the final length of the retrieved response may be less than `maxlen`.)

POST data is passed in the buffer `body`, where `bodylen` is the length of the buffer. (It is necessary to specify the buffer length manually because POST requests may include binary data.)

`headers` can be optionally used to specify additional HTTP header requests in the outgoing request (use 0 for no custom headers). Each custom header line should be followed by the HTTP-standard line break `\r\n` (even the last one).

```c
status = HTTP_POST("http://example.com", buffer, sizeof(buffer), 0,
                   "username=test&password=123", 26, 1);
```

*Return value*: On success, writes the response to `buffer` and returns the HTTP response status code (e.g., 200 "OK", 404 "Not Found"). On failure, sets `_neterr` and returns -1. If a response is received but it does not contain a valid HTTP response status code, returns 0.

## TCP functions

### TCP_OpenClient()

```c
signed char TCP_OpenClient(unsigned long ip, signed short lport, unsigned short rport);
```

Opens a client TCP connection to the IPv4 address `ip` (formatted as a 32-bit number) on local port `lport`, connecting to remote port `rport`. For client connections, `lport` should usually be set to -1 to obtain a dynamic port number.

*Return value*: On success, returns a socket handle to the new connection. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_OpenServer()

```c
signed char TCP_OpenServer(unsigned short lport);
```

Opens a server TCP connection on local port `lport`.

*Return value*: On success, returns a socket handle to the new connection. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_Close()

```c
signed char TCP_Close(unsigned char handle);
```

Closes and releases the TCP connection associated with the socket `handle`, without first sending a disconnect signal. (This is intended for when the remote host has already closed the connection with us; see also `TCP_Disconnect()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Close` (`TCPCLO`).

### TCP_Status()

```c
signed char TCP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the TCP connection associated with the socket `handle` and stores the results in the `NetStat` struct `obj`, which has the format:

```c
typedef struct {
    unsigned char status;    // status (see below)
    unsigned long ip;        // remote IP address
    unsigned short rport;    // remote port
	unsigned char datarec;   // 1 = data received, 0 = none
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

`status` may be one of `TCP_OPENING`, `TCP_OPENED`, `TCP_CLOSING`, or `TCP_CLOSED`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Status` (`TCPSTA`).

### TCP_Receive()

```c
signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr,
                        unsigned short len, TCP_Trans* obj);
```

Moves data which has been received from the remote host associated with socket `handle` to the memory at bank `bank`, address `addr`, which should be in the **data** or **transfer** segment. Up to `len` bytes will be moved (or the actual amount in the buffer, whichever is less).

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

```c
typedef struct {
    unsigned short transferred;  // bytes transferred to destination
    unsigned short remaining;    // bytes remaining in the buffer
} TCP_Trans;
```

A subtlety: Note that setting `len` to the total number of available bytes and calling `TCP_Receive()` is not guaranteed to leave the buffer empty, because the network daemon can receive additional bytes at any time. The main situation where this matters is when the app wants to empty the buffer and wait for a message from the network daemon alerting it when new data arrives. However, the network daemon will only send such a message when adding data *to an empty buffer*, so if the initial `TCP_Receive()` call does not actually empty the buffer completely, no message will arrive. One way to avoid this is to check the returned `TCP_Trans.remaining` value and keep calling `TCP_Receive()` until this value is actually zero; alternatively, the app can not rely on receiving a message.

*Return value*: On success, returns 0 and loads information into `obj`, if specified. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Receive` (`TCPRCV`).

### TCP_Send()

```c
signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len);
```

Sends data from memory bank `bank`, address `addr`, to the host associated with the socket `handle`. A total of `len` bytes will be sent.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Send` (`TCPSND`).

### TCP_Skip()

```c
signed char TCP_Skip(unsigned char handle, unsigned short len);
```

Skips and throws away `len` bytes of data which have already been received from the host associated with the socket `handle`. `len` must be equal to or smaller than the total number of bytes in the buffer (see `TCP_Status()`).

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Skip` (`TCPSKP`).

### TCP_Flush()

```c
signed char TCP_Flush(unsigned char handle);
```

Flushes the send buffer immediately. Some network hardware or software may hold data in the send buffer for a brief period of time before sending; this command requests to send it immediately.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Flush` (`TCPFLS`).

### TCP_Disconnect()

```c
signed char TCP_Disconnect(unsigned char handle);
```

Sends a disconnect signal to the remote host associated with the socket `handle`, closes the TCP connection, and releases the socket. (This is intended for we want to initiate the disconnection with the remote host; see also `TCP_Close()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Disconnect` (`TCPDIS`).

## UDP functions

**Warning**: UDP functions are not available on all devices. In particular, the M4 Board does not provide UDP; `UDP_Open()` will always fail with an error when used on this device.

### UDP_Open()

```c
signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank);
```

Opens a UDP session on local port `lport`. Data for this session will be stored in RAM bank `bank`

*Return value*: On success, returns a socket handle to the new session. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Open` (`UDPOPN`).

### UDP_Close()

```c
signed char UDP_Close(unsigned char handle);
```

Closes and releases the UDP session associated with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Close` (`UDPCLO`).

### UDP_Status()

```c
signed char UDP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the UDP session associated with the socket `handle` and stores the results in the `NetStat` struct `obj`, which has the same format as for `TCP_Status()`:

```c
typedef struct {
    unsigned char status;    // status
    unsigned long ip;        // remote IP address
    unsigned short rport;    // remote port
	unsigned char datarec;   // n/a
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Status` (`UDPSTA`).

### UDP_Receive()

```c
signed char UDP_Receive(unsigned char handle, char* addr);
```

Moves data which has been received from the remote host associated with socket `handle` to the memory at address `addr` (in the bank specified to `UDP_Open()`). The entire packet will be transferred at once, so be sure that there is enough space for an entire packet at the destination address. UDP packets have a theoretical limit of 65507 bytes, but we can also check how much data is waiting with `UDP_Status()`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Receive` (`UDPRCV`).

### UDP_Send()

```c
signed char UDP_Send(unsigned char handle, char* addr, unsigned short len,
                     unsigned long ip, unsigned short rport)
```

Sends a data packet from the memory address `addr` (in the bank specified to `UDP_Open()`) to the host associated with the socket `handle`. If sending fails becaus the buffer is full, the application should idle briefly and try again.

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Send` (`UDPSND`).

### UDP_Skip()

```c
signed char UDP_Skip(unsigned char handle);
```

Skips and throws away a complete packet which has already been received from the host associated with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Skip` (`UDPSKP`).

## DNS functions

### DNS_Resolve()

```c
unsigned long DNS_Resolve(unsigned char bank, char* addr);
```

Performs a DNS lookup and attempts to resolve the host IP/URL stored in the string at bank `bank`, address `addr` into an IPv4 address. (Note that the URL should *not* be prefixed with a protocol like `http://`; the easiest way to guarantee this is to use `Net_SplitURL()` and then run `DNS_Resolve()` only on the extracted hostname.)

*Return value*: On success, returns the IPv4 address (as a 32-bit integer). On failure, sets `_neterr` and returns 0.

*SymbOS name*: `DNS_Resolve` (`DNSRSV`).

### DNS_Verify()

```c
unsigned char DNS_Verify(unsigned char bank, char* addr);
```

Verifies whether the IP/URL stored in the string at bank `bank`, address `addr` is a valid IP or domain address. This function does not interact with the network hardware, so can be used to quickly determine whether an address is valid before initiating a full network request.

*Return value*: On success, returns `DNS_IP` for a valid IP address, `DNS_DOMAIN` for a valid domain address, or `DNS_INVALID` for an invalid address. On failure, sets `_neterr` and returns `DNS_INVALID`.

*SymbOS name*: `DNS_Verify` (`DNSVFY`).

## Helper functions

### Net_ErrMsg()

```c
void Net_ErrMsg(void* modalWin);
```

Displays a message box with the current error in `_neterr`, if any. `modalWin` specifies the address of a `Window` data record that should be declared modal, if any; this window will not be able to be focused until the message box is closed. If `modalWin` = 0, no window will be declared modal.

### Net_SplitURL()

```c
signed char Net_SplitURL(char* url, char* host, char** path, int* port);
```

A utility function that splits the string `url` into its constituent components, writing the hostname to the buffer at `host`, the address of the path/query string to the variable passed by reference as `path`, and the port number to the variable passed by reference as `port`. The buffer `host` should be at least 65 bytes long.

*Return value*: On success, returns the identified protocol (one of `PROTO_OTHER`, `PROTO_HTTPS`, `PROTO_HTTP`, `PROTO_FTP`, `PROTO_IRC`, `PROTO_SFTP`, `PROTO_FILE`, `PROTO_IMAP`, `PROTO_POP`, or `PROTO_NNTP`). On failure, sets `_neterr` = `ERR_BADDOMAIN` and returns -1.

### Net_PublicIP()

```c
unsigned long Net_PublicIP(void);
```

A utility function that attempts to determine the computer's public-facing IP address (by querying the free AWS service `checkip.amazonaws.com`). This is useful for server programs that need to tell the user what IP address remote clients should try to connect to.

*Return value*: On success, returns an IP address. On failure, sets `_neterr` and returns 0.

### iptoa()

```c
char* iptoa(unsigned long ip, char* dest);
```

Converts the numeric IPv4 address `ip` into a readable string representation in the buffer `dest` (e.g., `192.168.0.1`). `dest` must be at least 16 bytes long.

*Return value*: `dest`.

## Reference

### Error codes

The following errors are primarily issued by the network interface (stored in `_neterr`):

* `ERR_OFFLINE`: Offline/not connected/no network daemon
* `ERR_NOHW`: No hardware setup
* `ERR_NOIP`: No IP configuration
* `ERR_NOFUNC`: Function not supported
* `ERR_HARDWARE`: Unknown hardware error
* `ERR_WIFI`: WiFi error (SymbiFace 3)
* `ERR_NOSOCKET`: No more free sockets
* `ERR_BADSOCKET`: Socket does not exist
* `ERR_SOCKETTYPE`: Wrong socket type
* `ERR_SOCKETUSED`: Socket is already in use by another process
* `ERR_BADDOMAIN`: Invalid domain string
* `ERR_TIMEOUT`: Connection timeout
* `ERR_RECURSION`: Recursion not supported
* `ERR_TRUNCATED`: Truncated response
* `ERR_TOOLARGE`: Packet too large
* `ERR_CONNECT`: TCP connection not yet established
