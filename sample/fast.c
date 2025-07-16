// A simple network speed test, demonstrating how to implement TCP connections
// by making an HTTP POST request to the echo server httpbin.org.
//
// (For most HTTP requests, just use HTTP_GET() or HTTP_POST(). This example
// demonstrates the full TCP connection procedure for reference and to allow
// timing uploads/downloads separately.)
//
// compile with: cc -o fast.com fast.c -lnet

#include <string.h>
#include <symbos.h>
#include <network.h>

// some globals
char packet[1024];
char textbuf[129];
signed char socket = -1;
unsigned long counter;
char* headers = "POST /anything HTTP/1.1\r\nHost: httpbin.org\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: 32768\r\n\r\n";

// display an error and quit early
void fail(void) {
    Net_ErrMsg(0);
    if (socket != -1)
        TCP_Disconnect(socket);
    exit(0);
}

// calculate and show speed, in KBPS
void show_speed(unsigned short len) {
    int kbps = len / (Sys_Counter() - counter) * 100 * 8 / 1024;
    itoa(kbps, textbuf, 10);
    strcat(textbuf, " kbps\r\n");
    Shell_Print(textbuf);
}

// main routine
int main(int argc, char* argv[]) {
    int i;
    unsigned short get_bytes;
    unsigned short received_bytes;
    char ch, first;
    char ip[4];
    NetStat net_stat;
    TCP_Trans trans_stat;

    Shell_Print("Network Speed Test\r\n");

    // locate network daemon
    if (Net_Init())
        fail();

    // get IP of echo service (httpbin.org)
    if (DNS_Resolve(_symbank, "httpbin.org", ip))
        fail();

    // open client connection
    socket = TCP_OpenClient(ip, -1, 80);
    if (socket == -1)
        fail();

    // make a dummy packet
    ch = 'A';
    for (i = 0; i < sizeof(packet); ++i) {
        packet[i] = ch++;
        if (ch > 'Z')
            ch = 'A';
    }

    // send to echo service
    Shell_Print("Upload: ");
    counter = Sys_Counter();
    if (TCP_Send(socket, _symbank, headers, strlen(headers))) // this is an HTTP POST request, so we first send HTTP headers
        fail();
    for (i = 0; i < 32; ++i) {
        if (TCP_Send(socket, _symbank, packet, sizeof(packet))) // ..then the body content, in chunks of any size (the content length is specified in the headers)
            fail();
    }
    show_speed(32*sizeof(packet) + strlen(headers));

    // receive response
    Shell_Print("Download: ");
    first = 1;
    received_bytes = 0;
    for (;;) {
        // wait for a NET_TCPEVT message from the daemon
        _symmsg[0] = 0;
        Msg_Sleep(_sympid, _netpid, _symmsg);
        if (_symmsg[0] == NET_TCPEVT) {
            TCP_Event(_symmsg, &net_stat);
            // fully download any received data (see docs for a discussion of why this loops)
            get_bytes = net_stat.bytesrec;
            while (get_bytes) {
                // download up to 1K of data
                if (get_bytes > sizeof(packet))
                    get_bytes = sizeof(packet);
                if (TCP_Receive(socket, _symbank, packet, get_bytes, &trans_stat))
                    fail();
                get_bytes = trans_stat.remaining; // check if there is still data left in the buffer

                // keep track of bytes received and time taken
                if (first) { // only starts counter after first transfer, to avoid counting the response time
                    counter = Sys_Counter();
                    first = 0;
                } else {
                    received_bytes += trans_stat.transferred;
                }
            }

            // received enough bytes for a measurement, stop listening
            if (received_bytes >= 31*sizeof(packet))
                break;

            // connection closed by server, stop listening
            if (net_stat.status == TCP_CLOSING || net_stat.status == TCP_CLOSING)
                break;

            // The "correct" way to know when an HTTP response is done is to keep track of
            // "Content-Length" in the response headers, as HTTP_POST() does. Including
            // "Connection: close" in the request headers *should* make the remote server
            // close the connection when finished, but it doesn't always, especially
            // when using a proxy. Here, we cheat by just stopping after getting enough
            // bytes for a measurement.
        }
    }
    show_speed(received_bytes);

    // disconnect
    if (TCP_Disconnect(socket))
        fail();
    Shell_Print("\r\n");
}
