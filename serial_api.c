#include <stdio.h>
#include <stdlib.h>     //atoi
#include <termios.h>    //termios struct
#include <fcntl.h>      //O_RDWR | O_NOCTTY
#include <unistd.h>
#include <string.h>     //needed for memset
#include <getopt.h>     //get cmdline parm
#include "serial_api.h"

/***** local typedef *****/
typedef struct
{
    CommIcf_e   icf;
    const char  *s_opt;
}icfMap_t;

typedef struct
{
    CommFct_e   ftc;
    const char  *s_opt;
}fctMap_t;

/***** local define *****/
#define ICF_MAP_LEN (sizeof(icfMap)) / (sizeof(icfMap[0]))
#define FCT_MAP_LEN (sizeof(fctMap)) / (sizeof(fctMap[0]))
#define INVALID_HANDLE (int)0xFFFFFFFF

/***** local function declare *****/
static CommErrorCode_e comm_Brt_Set(portObj_t portObj);
static CommErrorCode_e comm_Icf_Set(portObj_t portObj);
static CommErrorCode_e comm_Fct_Set(portObj_t portObj);
static CommErrorCode_e comm_Fct_Map(portObj_t *portObj, char *opt);
static CommErrorCode_e comm_Icf_Map(portObj_t *portObj, char *opt);

/***** local data *****/
static struct option commOpts[] = {
    { "port",           required_argument, 0, 'p'},
    { "baudrate",       required_argument, 0, 'b'},
    { "flowcontrol",    required_argument, 0, 'f'},
    { "icf",            required_argument, 0, 'i'},
    { 0, 0, 0, 0}
};
static char shortOpts[] = "p:b:f:i:";

static const icfMap_t icfMap[] = {
    { ICF_8N1, "8N1" },
    { ICF_7E1, "7E1" },
    { ICF_7O1, "7O1" },
    { ICF_7S1, "7S1" },
    { ICF_UNKNOWN, NULL },
};

static const fctMap_t fctMap[] = {
    { FCT_RTS_CTS,  "rtscts" },
    { FCT_XON_XOFF, "xonxoff" },
    { FCT_NO,       "none" },
    { FCT_UNKNOWN, NULL },
};

/**** function implementation *****/
CommErrorCode_e comm_Get_Opt(portObj_t *portObj, int argc, char **argv)
{
    while(1)
    {
        int c = 0 ;
        int optIndex = 0;
        CommErrorCode_e res;

        c = getopt_long(argc, argv, shortOpts, commOpts, &optIndex);
        if(c == -1)
        {
            break;
        }

        switch(c)
        {
            case 'p':
                if(strlen(optarg)<PORT_PATH_MAX)
                {
                    strncpy(portObj->portName, optarg, strlen(optarg));
                }
                else
                {
                    printf("device path length exceed!\n");
                    return COMM_PORT_GET_PARM_ERROR;
                }
                break;
            case 'b':
                portObj->brt = (unsigned int)atoi(optarg);
                break;
            case 'f':
                res = comm_Fct_Map(portObj, optarg);
                if(res != COMM_PORT_OK)
                {
                    return res;
                }
                break;
            case 'i':
                res = comm_Icf_Map(portObj, optarg);
                if(res != COMM_PORT_OK)
                {
                    return res;
                }
                break;
            default:
                break;
        }
    }

    if(optind < argc)
    {
        printf("non-option ARGV:");
        while(optind < argc)
        {
            printf("%s ", argv[optind++]);
        }
        printf("\n");

        return COMM_PORT_GET_PARM_ERROR;
    }

    return COMM_PORT_OK;
}

CommErrorCode_e comm_Open_Port(portObj_t portObj)
{
    struct termios  setting;
    CommErrorCode_e res = COMM_PORT_OK;

    portObj.hd = open(portObj.portName, O_RDWR | O_NOCTTY);
    if(portObj.hd < 0)
    {
        portObj.hd = INVALID_HANDLE;
        return COMM_PORT_OPEN_ERROR;
    }

    memset(&setting, 0x0, sizeof(setting));

    setting.c_cflag = CREAD | CLOCAL | HUPCL;

    /* set baud rate */
    res = comm_Brt_Set(portObj);
    if(res != COMM_PORT_OK)
    {
        return res;
    }

    /* set icf */
    res = comm_Icf_Set(portObj);
    if(res != COMM_PORT_OK)
    {
        return res;
    }

    /* set flow control */
    res = comm_Fct_Set(portObj);
    if(res != COMM_PORT_OK)
    {
        return res;
    }

    /* update termiox NOW */
    if(tcsetattr(portObj.hd, TCSANOW, &setting) != 0)
    {
        close(portObj.hd);
        portObj.hd = INVALID_HANDLE;

        return COMM_PORT_SET_FAIL;
    }

    printf("port handle: %d\n", portObj.hd);
    printf("port path: %s\n", portObj.portName);
    printf("port baudrare: %u\n", portObj.brt);
    printf("port icf: %d\n", portObj.icf);
    printf("port flow control: %d\n", portObj.fct);

    return res;
}

CommErrorCode_e comm_Close_Port(portObj_t portObj)
{
    if(portObj.hd != INVALID_HANDLE)
    {
        close(portObj.hd);
        portObj.hd = INVALID_HANDLE;
    }
    return COMM_PORT_OK;
}

static CommErrorCode_e comm_Fct_Map(portObj_t *portObj, char *opt)
{
    unsigned int i;

    for(i=0; i<FCT_MAP_LEN; i++)
    {
        if(strncmp(opt, fctMap[i].s_opt, strlen(fctMap[i].s_opt)) == 0)
        {
            portObj->fct = fctMap[i].ftc;
            return COMM_PORT_OK;
        }
    }

    return COMM_PORT_MAP_FCT_ERROR;
}

static CommErrorCode_e comm_Icf_Map(portObj_t *portObj, char *opt)
{
    unsigned int i;

    for(i=0; i<ICF_MAP_LEN; i++)
    {
        if(strncmp(opt, icfMap[i].s_opt, strlen(icfMap[i].s_opt)) == 0)
        {
            portObj->icf = icfMap[i].icf;
            return COMM_PORT_OK;
        }
    }

    return COMM_PORT_MAP_ICF_ERROR;
}

static CommErrorCode_e comm_Brt_Set(portObj_t portObj)
{
    struct termios setting;

    if(tcgetattr(portObj.hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    switch(portObj.brt)
    {
        case 115200:
            cfsetispeed(&setting, B115200);
            cfsetospeed(&setting, B115200);
            break;
        case 230400:
            cfsetispeed(&setting, B230400);
            cfsetospeed(&setting, B230400);
            break;
        case 460800:
            cfsetispeed(&setting, B460800);
            cfsetospeed(&setting, B460800);
            break;
        case 921600:
            cfsetispeed(&setting, B921600);
            cfsetospeed(&setting, B921600);
            break;
        default:
            cfsetispeed(&setting, B115200);
            cfsetospeed(&setting, B115200);
            break;
    }

    if(tcsetattr(portObj.hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_FAIL;
    }

    return COMM_PORT_OK;
}

static CommErrorCode_e comm_Icf_Set(portObj_t portObj)
{
    struct termios setting;

    if(tcgetattr(portObj.hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    switch(portObj.icf)
    {
        case ICF_8N1:
            setting.c_cflag &= ~PARENB;
            setting.c_cflag &= ~CSTOPB;
            setting.c_cflag &= ~CSIZE;
            setting.c_cflag |= CS8;
            break;
        case ICF_7E1:
            setting.c_cflag |= PARENB;
            setting.c_cflag &= ~PARODD;
            setting.c_cflag &= ~CSTOPB;
            setting.c_cflag &= ~CSIZE;
            setting.c_cflag |= CS7;
            break;
        case ICF_7O1:
            setting.c_cflag |= PARENB;
            setting.c_cflag |= PARODD;
            setting.c_cflag &= ~CSTOPB;
            setting.c_cflag &= ~CSIZE;
            setting.c_cflag |= CS7;
            break;
        default:
            setting.c_cflag &= ~PARENB;
            setting.c_cflag &= ~CSTOPB;
            setting.c_cflag &= ~CSIZE;
            setting.c_cflag |= CS8;
            break;
    }

    if(tcsetattr(portObj.hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_ICF_ERROR;
    }

    return COMM_PORT_OK;
}

static CommErrorCode_e comm_Fct_Set(portObj_t portObj)
{
    struct termios setting;

    if(tcgetattr(portObj.hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    switch(portObj.fct)
    {
        case FCT_RTS_CTS:
            setting.c_cflag |= CRTSCTS;
            break;
        case FCT_NO:
            setting.c_cflag &= ~CRTSCTS;
            break;
        case FCT_XON_XOFF:
            setting.c_cflag &= ~CRTSCTS;
            setting.c_iflag |= IXON | IXOFF | IXANY;
            break;
        default:
            break;
    }

    if(tcsetattr(portObj.hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_ICF_ERROR;
    }

    return COMM_PORT_OK;
}
