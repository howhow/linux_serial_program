#include <stdio.h>
#include "serial_api.h"

int main(int argc,char** argv)
{
    portObj_t portObj;
    CommErrorCode_e res;

    res = comm_Get_Opt(&portObj, argc, argv);
    if(res != COMM_PORT_OK)
    {
        printf("device parm get fail!\n");
        return res;
    }

    res = comm_Open_Port(portObj);
    if(res != COMM_PORT_OK)
    {
        printf("device open fail!\n");
    }

    res = comm_Close_Port(portObj);
    if(res != COMM_PORT_OK)
    {
        printf("device close fail!\n");
    }

    return 0;
}

