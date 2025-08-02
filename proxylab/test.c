#include <stdio.h>
#include "./csapp.h"

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

int main(int argc, char **argv) {
  char url[100] = "http://www.cmu.edu:8080/hub/index.html";
  char host[50], path[50], port[50];
  printf("before enter parse_uri\n");
  parse_url(url, host, path, port);
  printf("test\n");
  printf("host: %s, path: %s, port: %s\n", host, path, port);
  return 0;
}
