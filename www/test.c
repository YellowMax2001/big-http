#include <stdio.h>                                            
#include <unistd.h>

int  main(int argc, char *argv[])                                                   
{                                                                 
    printf("Content-type: text/html\n\n");                        

    printf("<html>\n");                                            
    printf("<head>\n");                                            
    printf("<title>CGI Output</title>\n");                         
    printf("</head>\n");                                           
                                                                   
    printf("<body>");                                             
    printf("<h1> Hellow, world. </h1>");                           

    printf("SERVER_SOFTWARE = %s<br/>", getenv("SERVER_SOFTWARE"));
    printf("SERVER_NAME = %s<br/>", getenv("SERVER_NAME"));
    printf("GATEWAY_INTERFACE = %s<br/>", getenv("GATEWAY_INTERFACE"));
    printf("SERVER_PROTOCOL = %s<br/>", getenv("SERVER_PROTOCOL"));
    printf("SERVER_PORT = %s<br/>", getenv("SERVER_PORT"));
    printf("REQUEST_METHOD = %s<br/>", getenv("REQUEST_METHOD"));
    printf("PATH_INFO = %s<br/>", getenv("PATH_INFO"));
    printf("PATH_TRANSLATED = %s<br/>", getenv("PATH_TRANSLATED"));
    printf("SCRIPT_NAME = %s<br/>", getenv("SCRIPT_NAME"));
    printf("QUERY_STRING = %s<br/>", getenv("QUERY_STRING"));
    printf("REMOTE_HOST = %s<br/>", getenv("REMOTE_HOST"));
    printf("REMOTE_ADDR = %s<br/>", getenv("REMOTE_ADDR"));
    printf("AUTH_TYPE = %s<br/>", getenv("AUTH_TYPE"));
    printf("REMOTE_USER = %s<br/>", getenv("REMOTE_USER"));
    printf("REMOTE_IDENT = %s<br/>", getenv("REMOTE_IDENT"));
    printf("QUERY_STRING = %s<br/>", getenv("QUERY_STRING"));

    printf("</body>");                                           
    printf("</html>\n");                                         
    return 0;                                                    
}

