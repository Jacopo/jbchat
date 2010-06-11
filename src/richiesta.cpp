#include "richiesta.h"
#include <fcgiapp.h>
#include <cstdlib>
#include <string>
#include <cstring>

using namespace std;

void Richiesta::parse_query()
{
    // TODO
    char* str;
    METODO metodo;
    str=FCGX_GetParam("REQUEST_METHOD", fcgi_request.envp);
    if (strcmp ( str, "GET")==0){
        metodo=GET;
    }else if (strcmp ( str, "POST")==0){
        metodo=POST;
    }else {
        metodo=UNDEFINED;
    }
    switch (metodo){
        case GET:
            processGET();
            break;
        case POST:
            processPOST();
            break;
        default:
            m_tipo=IGNOTA;
    };

    FCGX_GetParam("QUERY_STRING", fcgi_request.envp);

    m_da = 0;

    already_parsed = true;
}

void Richiesta::processGET(){
        char* str;
        char num[30];

        str=FCGX_GetParam("QUERY_STRING", fcgi_request.envp);
        if (strncmp(str,"ricevi_da=",9)==0){
            m_tipo=RICEZIONE;
            int i=0;
            for (i=0;i<30;i++){
                if(str[9+i]<'0' || str[9+i]>'9')
                    break;
                num[i]=str[9+i];
            }
            if (i==30 || i==1){
                m_tipo=IGNOTA;  //ERRORE
                return;
            }
                num[i+1]='\0';
            m_da=atoi(num);
            return;
        }
        m_tipo=IGNOTA;  //ERRORE
        return;
    }

void Richiesta::processPOST(){
    m_tipo=INVIO;
}

