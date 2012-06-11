#include <network/network.h>
#include <ppc/timebase.h>
#include <time/time.h>
#include <string.h>
#include <debug.h>
#include "http_client.h"
#include "http_parser.h"

static struct tcp_pcb * http_pcb = NULL;
static http_parser *parser = NULL;
static http_parser_settings settings;

/**
 * Close the http client
 * @param tpcb
 */
static void http_close(struct tcp_pcb * tpcb) {
        TR;
        tcp_arg(http_pcb, NULL);
        tcp_err(http_pcb, NULL);
        tcp_recv(http_pcb, NULL);
        tcp_sent(http_pcb, NULL);
        tcp_close(http_pcb);
}

static int message_begin_cb(http_parser *p) {
        TR
        xHttpRequest * xhr = (xHttpRequest*) p->data;
        xhr->start = mftb();
        xhr->data = NULL;
        xhr->len = NULL;

        return 0;
}

static int message_complete_cb(http_parser *p) {
        TR
        xHttpRequest * xhr = (xHttpRequest*) p->data;
        xhr->end = mftb();

        xhr->status_code = p->status_code;
        if (xhr && xhr->callback)
                xhr->callback(xhr);

        return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len) {
        TR;
        // got response
        xHttpRequest * xhr = (xHttpRequest*) p->data;
        size_t t = xhr->len;
        xhr->len += len;
        xhr->data = realloc(xhr->data, xhr->len);
        memcpy(xhr->data + t, buf, len);

        return 0;
}

static void handle_http_response(unsigned char * data, int len) {
        TR;
        http_parser_execute(parser, &settings, data, len);
}

static void http_error(void * arg, err_t err) {
        printf("Http client error\n");
        http_close(http_pcb);
}

static err_t http_recv(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err) {
        TR;
        if (err == ERR_OK && p != NULL) {
                int i = 0;
                unsigned char * data = (unsigned char *) malloc(p->tot_len);
                unsigned char * pc = (unsigned char *) p->payload;

                tcp_recved(tpcb, p->tot_len);

                memcpy(data, pc, p->tot_len);

                handle_http_response(data, p->tot_len);

                free(data);
        }
        if (p != NULL)
                pbuf_free(p);

        if (err == ERR_OK && p == NULL) {
                http_close(tpcb);
        }
        return ERR_OK;
}

static err_t http_sent(void * arg, struct tcp_pcb * tpcb, u16_t len) {
        TR;
        return ERR_OK;
}

static err_t http_connect(void * arg, struct tcp_pcb * tpcb, u16_t len) {
        xHttpRequest * xhr = (xHttpRequest *) arg;

        char * request = xhr->request;
        char * buff = request;
        memset(request, 0, 512);

        buff += sprintf(buff, "GET %s HTTP/1.0\r\n", xhr->url);
        if (xhr->hostname[0])
                buff += sprintf(buff, "Host: %s\r\n", xhr->hostname);

        buff += sprintf(buff, "User-Agent: LibXenon\r\n\r\n");

        printf("connected \r\n");
        printf("%s\n", request);

        tcp_write(http_pcb, request, strlen(request), 0);
        
        TR

        return ERR_OK;
}

 void http_get_files(xHttpRequest * request) {
        // init parser
        parser = malloc(sizeof (http_parser));

        settings.on_body = body_cb;
        settings.on_message_begin = message_begin_cb;
        settings.on_message_complete = message_complete_cb;

        http_parser_init(parser, HTTP_RESPONSE);

        parser->data = request;

        // init lwip http client
        http_pcb = tcp_new();
        tcp_arg(http_pcb, request);

        tcp_bind(http_pcb, IP_ADDR_ANY, 80);

        tcp_err(http_pcb, http_error);
        tcp_recv(http_pcb, http_recv);
        tcp_sent(http_pcb, http_sent);

        tcp_connect(http_pcb, request->ipaddr, 80, http_connect);
        
        TR;
}
 

 
 
#include <ppc/timebase.h>
#include <time/time.h>

void xhttpRequestCallback(void * args) {
        xHttpRequest * request = (xHttpRequest*)args;
//
        printf("%d bytes => received in %dms, %d kb/s\n",
                request->len, (int) tb_diff_msec(request->end, request->start),
                (int) (request->len / 1024 * 1000 / tb_diff_msec(request->end, request->start)));
//
//        char * body = (char*) request->data;
//        printf(body);
        
//        FILE *fd = fopen("uda:/dest.bin","wb");
//        fwrite(body,1,request->len,fd);
//        fclose(fd);
}

 void init_http_client(){
	xHttpRequest httpRequest;
        httpRequest.hostname[0] = 0; // not set
        strcpy(httpRequest.url, "/test_xb/ChronoTrigger(fr).zip");
        ipaddr_aton("192.168.1.98", &httpRequest.ipaddr); // ip
        httpRequest.callback = xhttpRequestCallback;
        http_get_files(&httpRequest);
}