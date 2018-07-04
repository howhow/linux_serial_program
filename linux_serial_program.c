#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial_api.h"

#define TEST_AT "ATI51\r\n"

int main(int argc,char** argv)
{
    SportObj_t *portObj = NULL;
    SportErrorCode_e res;

    portObj = (SportObj_t *)malloc(sizeof(SportObj_t));
    memset(portObj, 0x0, sizeof(SportObj_t));

    res = SPORT_Get_Opt(portObj, argc, argv);
    if(res != SPORT_OK)
    {
        printf("device parm get fail!\n");
        free(portObj);
        return res;
    }

    res = SPORT_Open_Port(portObj);
    if(res != SPORT_OK)
    {
        printf("device open fail!\n");
        free(portObj);
        return res;
    }

    res = SPORT_Get_Port_Atrribute(portObj);
    if(res != SPORT_OK)
    {
        printf("device attribute get fail!\n");
        free(portObj);
        return res;
    }

    res = SPORT_Write(portObj, (char *)TEST_AT);
    if(res != SPORT_OK)
    {
        printf("device write fail!\n");
        free(portObj);
        return res;
    }

    res = SPORT_Read(portObj);
    if(res != SPORT_OK)
    {
        printf("device read fail!\n");
        free(portObj);
        return res;
    }

    res = SPORT_Close_Port(portObj);
    if(res != SPORT_OK)
    {
        printf("device close fail!\n");
        free(portObj);
        return res;
    }

    free(portObj);
    return 0;
}

