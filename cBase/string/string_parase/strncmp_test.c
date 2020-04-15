#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SSID_MACR "ssid"
#define PASSWD "passwd"
#define PORT "3333"
#define DNS "dns"
#define PROTOCOL "protocol"
#define IPV4 "ipv4"
#define MAC_ADDR "mac_addr"
#define SERVER_ADDR "server_addr"
#define STATE_GET "state_get"

int main(){
    //char *s = "ssid:vision";
    //char *s = "passwd:12345678";
    //char *s = "port:3333";
    //char *s = "dns:8.8.8.8";
    //char *s = "protocol:a";
    //char *s = "server_addr:www.baidu.com";
    char *s = "state_get:on";
    char *p;
    printf(" ssid:%s len:%d \n\t",SSID_MACR,sizeof(SSID_MACR));
    if(0 == strncmp(s,SSID_MACR,sizeof(SSID_MACR) - 1))
    {
            p = s + sizeof(SSID_MACR);
            printf("ssid is:%s \n\t",p);
    }
    
    if(0 == strncmp(s,PASSWD,sizeof(PASSWD) - 1))
    {
            p = s + sizeof(PASSWD);
            printf("passwd is:%s \n\t",p);
    }
    
    if(0 == strncmp(s,PORT,sizeof(PORT) - 1))
    {
            p = s + sizeof(PORT);
            printf("port is:%s \n\t",p);
    }

    if(0 == strncmp(s,DNS,sizeof(DNS) - 1))
    {
            p = s + sizeof(DNS);
            printf("dns is:%s \n\t",p);
    }

    if(0 == strncmp(s,PROTOCOL,sizeof(PROTOCOL) - 1))
    {
            p = s + sizeof(PROTOCOL);
            printf("protocol is:%s \n\t",p);
    }

    if(0 == strncmp(s,IPV4,sizeof(IPV4) - 1))
    {
            p = s + sizeof(IPV4);
            printf("ipv4 is:%s \n\t",p);
    }

    if(0 == strncmp(s,MAC_ADDR,sizeof(MAC_ADDR) - 1))
    {
            p = s + sizeof(MAC_ADDR);
            printf("mac addr is:%s \n\t",p);
    }

    if(0 == strncmp(s,SERVER_ADDR,sizeof(SERVER_ADDR) - 1))
    {
            p = s + sizeof(SERVER_ADDR);
            printf("server addr is:%s \n\t",p);
    }

    if(0 == strncmp(s,STATE_GET,sizeof(STATE_GET) - 1))
    {
            p = s + sizeof(STATE_GET);
            printf("state get is:%s \n\t",p);
    }

    return 0;
}
