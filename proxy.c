#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void split_url(char *, char *, char *, char *);
void proxy_mode(int);
void forward_message(int, int);
void add_hdrs(int, char *);

/* test on browser : yes*/

int main(int argc, char **argv)
{
    int listenfd,connfd;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
        
    listenfd = Open_listenfd(argv[1]);  // listen for client request 
   
    while (1) {
        connfd = Accept(listenfd,(SA*)&clientaddr,&clientlen); // establish connection with the client
 
        proxy_mode(connfd);  // proxy routine
        Close(connfd);
    }    
    printf("%s", user_agent_hdr);
    return 0;
}
 
void proxy_mode(int to_clientfd) 
{
    char buf[MAXLINE],method[MAXLINE],url[MAXLINE],version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE], request[MAXLINE], port[MAXLINE];
    int to_serverfd;
    rio_t rp;
    Rio_readinitb(&rp, to_clientfd);  

    Rio_readlineb(&rp,buf,MAXLINE); //read client request
    sscanf(buf,"%s %s %s",method,url,version);  // parse request
    split_url(url,hostname,path, port);
    sprintf(request, "%s %s HTTP/1.0\r\n", method, path); // reconstruct http request sent to server
   
    to_serverfd = Open_clientfd(hostname,port);
    Rio_writen(to_serverfd, request, strlen(request));
    add_hdrs(to_serverfd, hostname);
    forward_message(to_serverfd, to_clientfd);    
    Close(to_serverfd);    
        
}


void split_url(char *url,char *hostname,char *path, char *port) {
    char *h_begin,*p_begin, *port_split;
    size_t host_port_length, hostlength, portlength, pathlength;
    char host_port[MAXLINE]; // store host:port 
    
    h_begin = url + 7;             //begin of host:port
    p_begin = strchr(h_begin,'/'); //begin of the path
    host_port_length = p_begin - h_begin; // length of the host:port
    strncpy(host_port,h_begin,host_port_length); // split host:port
    
    
    port_split = strchr(host_port, ':');
    if (!port_split) { // port not specified, use default port 80
        sprintf(port, "80");
        port[2] = '\0';
        strncpy(hostname, host_port, host_port_length);
        hostname[host_port_length] = '\0';
    } else {
       hostlength = port_split - host_port;
       portlength = strlen(host_port) - hostlength - 1;
       strncpy(hostname, host_port, hostlength); // split hostname
       strncpy(port, port_split + 1, portlength); // split port
       hostname[hostlength] = '\0';
       port[portlength] = '\0';
    }
 
    pathlength = strlen(url) - host_port_length - 7;
    strncpy(path,p_begin,pathlength);
    path[pathlength] = '\0';
}
   
void forward_message(int to_serverfd, int to_clientfd) {
    rio_t rp;
    char buf[MAXLINE];
    size_t n;

    Rio_readinitb(&rp, to_serverfd);    
    while ((n = Rio_readlineb(&rp,buf,MAXLINE)) != 0) {
        Rio_writen(to_clientfd, buf, n);
    }
}

void add_hdrs(int to_serverfd, char *hostname) {
    char buf[MAXLINE];
    
    sprintf(buf, "Host : %s\r\n", hostname);
    Rio_writen(to_serverfd, buf, strlen(buf));
    sprintf(buf, "User-Agent : %s\r\n",user_agent_hdr);
    Rio_writen(to_serverfd, buf, strlen(buf));
    sprintf(buf, "Connection : close\r\n");
    Rio_writen(to_serverfd, buf, strlen(buf));
    sprintf(buf, "Proxy Connection : close\r\n");
    Rio_writen(to_serverfd, buf, strlen(buf)); 
}


