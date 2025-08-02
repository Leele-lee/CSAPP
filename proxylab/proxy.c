#include <stdio.h>
#include "./csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* 
 * parse url static version: http://www.cmu.edu:8080/hub/index.html
 * or dynamic version: http://bluefish.ics.cs.cmu.edu:8000/cgi-bin/adder?15000&213
 * to get 
 * host: www.cmu.edu
 * path: /hub/index.html
 * port: 8080 (default 80)
 */
int parse_url(char *url, char *host, char *path, char* port) {
    char *pos = strstr(url, "//");
    if (!pos)
        return 0;
    pos += 2;
    char *colon = strstr(pos, ":");
    char *slash = strstr(pos, "/"); 

    if (!slash) {
        strcat(pos, "/");
        slash = strstr(pos, "/");
        strcpy(path, "/");
    } else {
        sscanf(slash, "%s", path); 
    }

    char res[MAXLINE];

    if (colon) {
        *colon = '\0';
        sscanf(pos, "%s %s", host, res);  // bluefish.ics.cs.cmu.edu 8000/cgi-bin/adder?15000&213
        printf("host: %s\n", host);
        size_t len = slash - colon - 1;
        strncpy(port, colon + 1, slash - colon - 1); // error! find why?
    } else {
        strcpy(port, "80");
        strncpy(host, pos, slash - pos);
    }
    return 1;
}


/*
 * 1. first parse request `GET http://www.cmu.edu/hub/index.html HTTP/1.1` to 
 *    get method, url and version
 * 2. then build new request send to server: `GET /hub/index.html HTTP/1.0`,
 *    ps: pay attention to these request headers Host, User-agent, Connection, Proxy-connection
 * 3. finally read the response from server and forward to client
 */
void handle_request(int connfd) {
    rio_t rio;
    int clientfd;
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];

    /* Read request line and headers */
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, url, version);

    char host[MAXLINE], path[MAXLINE], port[MAXLINE];
    int parse = parse_url(url, host, path, port);
    if (!parse) {
        fprintf(stderr, "The request's url: %s is not standard\n", url);
        return;
    }

    //open clientfd in proxy in order to pass request to server 
    //now proxy served as client
    clientfd = Open_clientfd(host, port);
    if (clientfd < 0) {
        Close(connfd);
        return;
    }

    /* build and send request line to server */
    sprintf(buf, "%s %s %s", method, path, version);
    Rio_writen(clientfd, buf, strlen(buf));

    /* build request headers and send to server */
    // read requesr headers from client
    while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        // when read empty line terminate headers --- "\r\n" break the loop
        if (!strcmp(buf, "\r\n")) break;
        
        // if the line start with host:, user-agent:, connection:, proxy-connection:
        // we replace them with our version 
        if ((strncasecmp(buf, "host:", 5) == 0) 
            || (strncasecmp(buf, "user-agent:", 11) == 0) 
            || (strncasecmp(buf, "connection:", 11)) == 0 
            || (strncasecmp(buf, "proxy-connection") == 0)) {
            break;
        }
        Rio_writen(clientfd, buf, strlen(buf));
    }
    sprintf(buf, 
            "Host: %s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n");
    Rio_writen(clientfd, buf, strlen(buf));

    // accepet response from server and send to client
    rio_t server_rio;
    Rio_readinitb(&server_rio, clientfd);
    ssize_t n;
    while (n = Rio_readn(&server_rio, clientfd, MAXLINE) > 0) {
        Rio_writen(connfd, buf, n);
    }
    Close(clientfd);
    Close(connfd);
}

int main(int argc, char **argv)
{
    // Listening on given port and accept new connect from client 
    // and handle_request in a infinite loop
    int listenfd, connfd;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;

    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "Using default port 5050\n");
        listenfd = Open_listenfd("5050");
    } else {
        listenfd = Open_listenfd(argv[1]);
    }
    while (1) {
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        handle_request(connfd);
    }
}
