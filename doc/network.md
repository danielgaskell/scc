# Network Library

**Note: The network library as described below is currently only available in development builds of SymbOS. Slightly different versions of `Net_Init()` and the raw TCP/UDP/DNS functions can be found in older releases in the header file `symbos/network.h`.**

**Note:** All network functions are thread-safe, and multiple threads can have connections open simultaneously (although the network daemon usually only permits sockets to be accessed by the thread that originally opened them). If a thread calls a network function that is currently in use by another thread, it will automatically wait its turn.

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
             unsigned char keep_headers);
```

Executes a complete HTTP GET request, downloading whatever content is returned from the URL `url` into the buffer at the address `dest`. A maximum of `maxlen` characters will be written, discarding any extra. If `maxlen` = `HTTP_FILE`, `dest` will instead be treated as the absolute path of a file to save the response to (with no length limit). If `keep_headers` is nonzero, the response will include the full HTTP headers as well as the body content.

`headers` can be optionally used to specify additional HTTP header requests in the outgoing request (use 0 for no custom headers). Each custom header line should be followed by the HTTP-standard line break `\r\n` (even the last one):

```c
status = HTTP_GET("http://numbersapi.com", buffer, sizeof(buffer),
                  "Accept: text/plain\r\nAccept-Language: en-US\r\n", 1);
```

*Return value*: On success, writes the response to the buffer `dest` (or the file indicated by `dest`) and returns the HTTP response status code (e.g., 200 "OK", 404 "Not Found"). On failure, sets `_neterr` and returns -1. If a response is received but it does not contain a valid HTTP response status code, returns 0.

### HTTP_POST()

```c
int HTTP_POST(char* url, char* dest, unsigned short maxlen, char* headers,
              char* body, unsigned short bodylen, unsigned char keep_headers);
```

Executes a complete HTTP POST request, downloading whatever content is returned from the URL `url` into the buffer at the address `dest`. A maximum of `maxlen` characters will be written, discarding any extra. If `maxlen` = `HTTP_FILE`, `dest` will instead be treated as the absolute path of a file to save the response to (with no length limit). If `keep_headers` is nonzero, the response will include the full HTTP headers as well as the body content.

POST data are passed in the buffer `body`, where `bodylen` is the length of the buffer. (It is necessary to specify `bodylen` manually because POST requests may include binary data.)

`headers` can be optionally used to specify additional HTTP header requests in the outgoing request (use 0 for no custom headers). Each custom header line should be followed by the HTTP-standard line break `\r\n` (even the last one).

```c
status = HTTP_POST("http://example.com", "A:\\EXAMPLE.HTM", HTTP_FILE,
                   0, "username=test&password=123", 26, 1);
```

*Return value*: On success, writes the response to the buffer `dest` (or the file indicated by `dest`) and returns the HTTP response status code (e.g., 200 "OK", 404 "Not Found"). On failure, sets `_neterr` and returns -1. If a response is received but it does not contain a valid HTTP response status code, returns 0.

### Proxy servers

By default, HTTP functions support only unencrypted connections (URLs beginning with `http://`). Native support for SSL-encrypted connections (URLs beginning with `https://`) is not practical on 8-bit hardware. However, it is possible to access HTTPS sites by using a modern go-between computer running a web proxy such as [WebOne](https://github.com/atauenis/webone). To route requests through a proxy, place the IP address of the proxy computer into the global buffer `_http_proxy_ip` (using, e.g., `DNS_Resolve()`) and the proxy's port into the global variable `_http_proxy_port`. For example:

```c
DNS_Resolve(_symbank, "192.168.0.19", _http_proxy_ip);
_http_proxy_port = 1234;
```

(In practice, of course, it is bad practice to hard-code proxy addresses like this---this should be a setting the user can edit, so the app can keep working even if the original proxy server goes down.) To stop routing requests through a proxy, clear `_http_proxy_ip` to all zeros.

### Tracking progress

Download speeds on 8-bit hardware are not very fast, typically on the order of 56-112 kbps (that is, a bit faster than a dialup modem). At this speed, a multi-megabyte file can take several minutes to download, so it is helpful to have a way to track the progress of a download and interrupt it if needed.

This is most easily done by running `HTTP_GET()` or `HTTP_POST()` on a [separate thread](syscall2.md#multithreading). When running on a different thread, we can interrupt execution by writing a nonzero value to the global variable `_http_interrupt`. `HTTP_GET()` and `HTTP_POST()` will then stop downloading at their earliest convenience, writing/saving only what they have downloaded so far.

`HTTP_GET()` and `HTTP_POST()` also record their progress in the global variable `_http_progress`, with values matching the following constants:

* `HTTP_LOOKUP` - performing DNS lookup and initial preparation
* `HTTP_CONNECTING` - connecting to remote server
* `HTTP_SENDING` - sending request
* `HTTP_WAITING` - waiting for first response packet
* `HTTP_DONE` - finished execution (whether successfully or not)
* Any positive value - download percentage (i.e., `_http_progress` = 72 means 72% complete)

## TCP functions

TCP is the primary low-level network protocol supported by SymbOS network hardware and underlies other protocols such as Telnet, HTTP, and FTP. In a TCP connection, a **client** requests to establish a two-way connection with a **server** (located at a specific IPv4 address and port number). Once established, both parties can send and receive streams of data, with the network hardware ensuring that the streams are delivered completely, correctly, and in the right order. The connection remains open until it is closed by one party (e.g., at the end of a Telnet session or HTTP request).

* To open a TCP connection, use `TCP_OpenClient()` or `TCP_OpenServer()`.
* Once open, data can be sent to the other party at any time with `TCP_Send()`.
* The network daemon will alert us to the arrival of new data (or a change in connection status) by sending message ID `NET_TCPEVT`. We can watch for this message in our main event loop and use `TCP_Event()` to understand its contents.
	* When `TCP_Event()` yields `NetStat.datarec` = 1, incoming data bytes are waiting; we can download them into a buffer using `TCP_Receive()`.
	* When `TCP_Event()` yields `NetStat.status` = `TCP_CLOSED`, the remote server has closed the connection and we can free the socket with `TCP_Close()`.
* To disconnect from our end, use `TCP_Disconnect()`.

For a complete example of implementing a TCP connection, see `fast.c` in the **sample** folder.

### TCP_OpenClient()

```c
signed char TCP_OpenClient(char* ip, signed short lport, unsigned short rport);
```

Opens a client TCP connection to the IPv4 address `ip` (stored in a 4-byte buffer, one byte per octet) on local port `lport`, connecting to remote port `rport`. (For client connections, `lport` should usually be set to -1 to obtain a dynamic port number.) The function will then wait for the socket status to become `TCP_OPENED`, indicating a successful connection. (If the server refuses the connection, this function will fail with `_neterr` = `ERR_CONNECT`.)

*Return value*: On success, returns a socket handle to the new connection. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_OpenServer()

```c
signed char TCP_OpenServer(unsigned short lport);
```

Opens a TCP server listening on local port `lport`.

*Return value*: On success, returns a socket handle to the new server. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Open` (`TCPOPN`).

### TCP_Event()

```c
void TCP_Event(char* msg, NetStat* obj);
```

A utility function for parsing a `NET_TCPEVT` message into a readable status report. `msg` is the address of the 14-byte message buffer containing the message; `obj` is the address of a `NetStat` struct to store the results, with the format:

```c
typedef struct {
    unsigned char socket;    // socket this message pertains to
    unsigned char status;    // status (see below)
    unsigned char ip[4];     // remote IP address
    unsigned short rport;    // remote port
    unsigned char datarec;   // 1 = data received, 0 = none
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

`status` may be one of `TCP_OPENING`, `TCP_OPENED`, `TCP_CLOSING`, or `TCP_CLOSED`.

### TCP_Status()

```c
signed char TCP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the TCP connection associated with the socket `handle` and stores the results in the `NetStat` struct `obj` (see `TCP_Event()`). This is equivalent to `TCP_Event()` but can be used to query the status of the connection at any time, not just after receiving a `NET_TCPEVT` message.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Status` (`TCPSTA`).

### TCP_Send()

```c
signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len);
```

Sends data from memory bank `bank`, address `addr`, to the host associated with the socket `handle`. A total of `len` bytes will be sent.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Send` (`TCPSND`).

### TCP_Receive()

```c
signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr,
                        unsigned short len, TCP_Trans* obj);
```

Moves data which has been received from the remote host associated with socket `handle` to the memory at bank `bank`, address `addr`. Up to `len` bytes will be moved (or the actual amount in the buffer, whichever is less).

`obj` is an optional pointer to a `TCP_Trans` struct, into which additional information about the transfer will be loaded. This parameter may be set to NULL to omit this information. The structure of the struct is:

```c
typedef struct {
    unsigned short transferred;  // bytes transferred to destination
    unsigned short remaining;    // bytes remaining in the buffer
} TCP_Trans;
```

A few subtleties:

* Note that setting `len` to the total number of available bytes and calling `TCP_Receive()` is not guaranteed to leave the buffer empty, because the network daemon can receive additional bytes at any time. The main situation where this matters is when the app wants to empty the buffer and wait for a `NET_TCPEVT` message from the network daemon alerting it when new data arrives. However, the network daemon will only send a `NET_TCPEVT` message when adding data *to an empty buffer*, so if the initial `TCP_Receive()` call does not actually empty the buffer completely, no message will arrive. One way to avoid this is to check the returned `TCP_Trans.remaining` value and keep calling `TCP_Receive()` until this value is actually zero; alternatively, the app can not rely on receiving a message.

* Note that, because TCP data are sent as a continuous stream, fully clearing the TCP buffer does *not* necessarily mean that we have processed all the data the other party intends to send! It is the program's responsibility to understand (from the content of the data stream) when a pause in receiving data means "request finished, please process it" versus "more data coming soon, please wait." Likewise, we need to be careful about assuming that any given call of `TCP_Receive()` will yield a "complete" response, as incoming data may arrive discontinuously and split into chunks of any size (most often 1460 or 2048 bytes, but potentially as small as 1 byte each).

* Some older daemon versions contained a bug that implicitly required `addr` to be in the **data** or **transfer** segments. While this has been fixed, if the buffer is small, you may consider putting it in these segments for maximum compatibility.

*Return value*: On success, returns 0 and loads information into `obj`, if specified. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Receive` (`TCPRCV`).

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

### TCP_Close()

```c
signed char TCP_Close(unsigned char handle);
```

Closes and releases the TCP connection associated with the socket `handle`, without first sending a disconnect signal. (This is intended for when the remote host has already closed the connection with us; see also `TCP_Disconnect()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `TCP_Close` (`TCPCLO`).

## UDP functions

UDP is a simple "connectionless" protocol that allows raw packets to be sent and received from arbitrary IP addresses. Unlike TCP, no connection is negotiated, and the network hardware does not verify that sent packets are actually delivered (or delivered in the right order). This makes UDP most suitable for use-cases like a multiplayer game, where we may want to receive status updates from an arbitrary number of other players without it mattering very much if an individual packet gets lost.

We can open a UDP session with `UDP_Open()`. When a UDP packet arrives on an open session, the network daemon will send message ID `NET_UDPEVT`, which we can process using `UDP_Event()`.

**Warning**: UDP functions are not available on all devices. In particular, the M4 Board (the most common network hardware for Amstrad CPC) does not provide UDP; `UDP_Open()` will always fail with an error when used on this device.

### UDP_Open()

```c
signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank);
```

Opens a UDP session on local port `lport`. Data for this session will be stored in RAM bank `bank`

*Return value*: On success, returns a socket handle to the new session. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Open` (`UDPOPN`).

### UDP_Event()

```c
void UDP_Event(char* msg, NetStat* obj);
```

A utility function for parsing a `NET_UDPEVT` message into a readable status report. `msg` is the address of the 14-byte message buffer containing the message; `obj` is the address of a `NetStat` struct to store the results, which has the same format as for `TCP_Status()`:

```c
typedef struct {
    unsigned char socket;    // socket this message pertains to
    unsigned char status;    // status
    unsigned char ip[4];     // remote IP address
    unsigned short rport;    // remote port
    unsigned char datarec;   // n/a
    unsigned short bytesrec; // received bytes waiting in buffer
} NetStat;
```

### UDP_Status()

```c
signed char UDP_Status(unsigned char handle, NetStat* obj);
```

Returns the status of the UDP connection associated with the socket `handle` and stores the results in the `NetStat` struct `obj` (see `UDP_Event()`). This is equivalent to `UDP_Event()` but can be used to query the status of the connection at any time, not just after receiving a `NET_UDPEVT` message.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Status` (`UDPSTA`).

### UDP_Send()

```c
signed char UDP_Send(unsigned char handle, char* addr, unsigned short len,
                     char* ip, unsigned short rport)
```

Sends a data packet from the memory address `addr` (in the bank specified in `UDP_Open()` for this session) to the IP address stored in the 4-byte buffer `ip`, using the UDP session with the socket `handle`. If sending fails because the buffer is full, the application should idle briefly and try again.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Send` (`UDPSND`).

### UDP_Receive()

```c
signed char UDP_Receive(unsigned char handle, char* addr);
```

Moves data which has been received on the UDP session with the socket `handle` to the memory at address `addr` (in the bank specified in `UDP_Open()` for this session). The entire packet will be transferred at once, so be sure that there is enough space for an entire packet at the destination address. (UDP packets have a theoretical limit of 65507 bytes, but we can also check how much data is waiting with `UDP_Event()` or `UDP_Status()`.)

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Receive` (`UDPRCV`).

### UDP_Skip()

```c
signed char UDP_Skip(unsigned char handle);
```

Skips and throws away a complete packet which has already been received on the UDP session with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Skip` (`UDPSKP`).

### UDP_Close()

```c
signed char UDP_Close(unsigned char handle);
```

Closes and releases the UDP session associated with the socket `handle`.

*Return value*: On success, returns 0. On failure, sets `_neterr` and returns -1.

*SymbOS name*: `UDP_Close` (`UDPCLO`).

## DNS functions

### DNS_Resolve()

```c
signed char DNS_Resolve(unsigned char bank, char* addr, char* ip);
```

Performs a DNS lookup and attempts to resolve the host IP/URL stored in the string at bank `bank`, address `addr` into an IPv4 address, which will be stored in the 4-byte buffer at `ip`. (Note that the URL should *not* be prefixed with a protocol like `http://`; the easiest way to guarantee this is to use `Net_SplitURL()` and then run `DNS_Resolve()` only on the extracted hostname.)

*Return value*: On success, stores the IP address in the 4-byte buffer pointed to by `ip` and returns 0. On failure, sets `_neterr` and returns -1.

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

A utility function that splits the string `url` (containing a URL) into its constituent components, writing the hostname to the buffer `host`, the address of the path/query string to the variable passed by reference as `path`, and the port number to the variable passed by reference as `port`. The buffer `host` should be at least 65 bytes long.

For example, the URL `http://example.com:8080/path?id=1` would be split into:

* `host`: `example.com`
* `path`: address of `path?id=1` in `url`
* `port`: 8080

If not explicitly specified in the URL, `Net_SplitURL()` will attempt to determine a default port number from the detected protocol (e.g., port 80 for HTTP).

*Return value*: On success, returns the identified protocol (one of `PROTO_OTHER`, `PROTO_HTTPS`, `PROTO_HTTP`, `PROTO_FTP`, `PROTO_IRC`, `PROTO_SFTP`, `PROTO_FILE`, `PROTO_IMAP`, `PROTO_POP`, or `PROTO_NNTP`). On failure, sets `_neterr` = `ERR_BADDOMAIN` and returns -1.

### Net_PublicIP()

```c
signed char Net_PublicIP(char* ip);
```

A utility function that attempts to determine the computer's public-facing IP address (by querying the free AWS service `checkip.amazonaws.com`). This is useful for server programs that need to tell the user what IP address remote clients should try to connect to.

**Note**: This function is larger that one might expect because it pulls in the full HTTP-handling portion of the library.

*Return value*: On success, stores the IP address in the 4-byte buffer pointed to by `ip` and returns 0. On failure, sets `_neterr` and returns -1.

### Net_SkipMsg()

```c
void Net_SkipMsg(signed char handle);
```

A utility function that removes all remaining `NET_TCPEVT` or `NET_UDPEVT` messages pertaining to the socket `handle` from the message queue. This is mainly used internally, but can be useful to keep the ring buffer from overflowing in apps that only selectively ingest certain event messages, putting the rest back on the queue. (If you don't know what that means, you probably don't need to worry about this.)

### iptoa()

```c
char* iptoa(char* ip, char* dest);
```

Converts the numeric IPv4 address stored in the 4-byte buffer `ip` into a readable string representation in the buffer `dest` (e.g., `192.168.0.1`). `dest` must be at least 16 bytes long.

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
* `ERR_CONNECT`: TCP connection not established (or not yet established)
* `ERR_NETFILE`: File error while performing network operation; check `_fileerr`
