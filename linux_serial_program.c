#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial_api.h"

int main(int argc,char** argv)
{
    portObj_t *portObj = NULL;
    CommErrorCode_e res;

    portObj = (portObj_t *)malloc(sizeof(portObj_t));
    memset(portObj, 0x0, sizeof(portObj_t));

    res = comm_Get_Opt(portObj, argc, argv);
    if(res != COMM_PORT_OK)
    {
        printf("device parm get fail!\n");
        free(portObj);
        return res;
    }

    res = comm_Open_Port(portObj);
    if(res != COMM_PORT_OK)
    {
        printf("device open fail!\n");
        free(portObj);
        return res;
    }

    res = comm_Get_Port_Atrribute(portObj);
    if(res != COMM_PORT_OK)
    {
        printf("device attribute get fail fail!\n");
        free(portObj);
        return res;
    }


    res = comm_Close_Port(portObj);
    if(res != COMM_PORT_OK)
    {
        printf("device close fail!\n");
        free(portObj);
        return res;
    }

    free(portObj);
    return 0;
}

