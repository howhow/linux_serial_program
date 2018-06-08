#include <stdio.h>
#include <stdlib.h>     //atoi
#include <termios.h>    //termios struct
#include <fcntl.h>      //O_RDWR | O_NOCTTY
#include <unistd.h>     //close()
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

typedef struct
{
    unsigned int    brt;
    speed_t         brt_mask;
}brtMap_t;

/***** local function declare *****/
static CommErrorCode_e comm_Fct_Map(portObj_t *portObj, char *opt);
static CommErrorCode_e comm_Icf_Map(portObj_t *portObj, char *opt);
static unsigned int comm_Brt_Map(speed_t speed);

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

static const brtMap_t brtMap[] = {
    { 300,      B300    },
    { 1200,     B1200   },
    { 2400,     B2400   },
    { 4800,     B4800   },
    { 9600,     B9600   },
    { 19200,    B19200  },
    { 38400,    B38400  },
    { 57600,    B57600  },
    { 115200,   B115200 },
    { 230400,   B230400 },
    { 460800,   B460800 },
    { 921600,   B921600 },
};

/***** local define *****/
#define ICF_MAP_LEN (sizeof(icfMap)) / (sizeof(icfMap[0]))
#define FCT_MAP_LEN (sizeof(fctMap)) / (sizeof(fctMap[0]))
#define BRT_MAP_LEN (sizeof(brtMap)) / (sizeof(brtMap[0]))
#define INVALID_HANDLE (int)0xFFFFFFFF

/***** local function *****/
static unsigned int comm_Brt_Map(speed_t speed)
{
    unsigned int i;

    for(i=0; i<BRT_MAP_LEN; i++)
    {
        if(brtMap[i].brt_mask == speed)
        {
            return brtMap[i].brt;
        }
    }

    return 0xFFFFFFFF;
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

/**** public function *****/
/*
 * demo function for get device param from cmd
 */
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

/*
 * open the device with:
 * 1. baudrate setting
 * 2. icf setting
 * 3. flowcontrol setting
 */
CommErrorCode_e comm_Open_Port(portObj_t *portObj)
{
    struct termios  setting;
    CommErrorCode_e res = COMM_PORT_OK;

    portObj->hd = open(portObj->portName, O_RDWR | O_NOCTTY);
    if(portObj->hd < 0)
    {
        portObj->hd = INVALID_HANDLE;
        return COMM_PORT_OPEN_ERROR;
    }

    memset(&setting, 0x0, sizeof(setting));

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

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }
    setting.c_cflag |= CREAD | CLOCAL | HUPCL;

    /* update termiox NOW */
    if(tcsetattr(portObj->hd, TCSANOW, &setting) != 0)
    {
        close(portObj->hd);
        portObj->hd = INVALID_HANDLE;

        return COMM_PORT_SET_FAIL;
    }

    return res;
}

/*
 * close the device
 */
CommErrorCode_e comm_Close_Port(portObj_t *portObj)
{
    if(portObj->hd != INVALID_HANDLE)
    {
        close(portObj->hd);
        portObj->hd = INVALID_HANDLE;
    }
    return COMM_PORT_OK;
}

/*
 * get device attribute
 */
CommErrorCode_e comm_Get_Port_Atrribute(portObj_t *portObj)
{
    struct termios setting;
    speed_t speed_o, speed_i;

    printf("device: %s\n", portObj->portName);
    printf("hd: %d\n", portObj->hd);

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    /* get parity */
    if(setting.c_cflag & PARENB)
    {
        if(setting.c_cflag & PARODD)
        {
            printf("Odd parity is set.\n");
        }
        else
        {
            printf("Even parity is set.\n");
        }
    }
    else
    {
        printf("None parity is set.\n");
    }

    /* get frame size */
    if(setting.c_cflag & CS7)
    {
        printf("7 data bit\n");
    }
    else if(setting.c_cflag & CS8)
    {
        printf("8 data bit\n");
    }

    /* get baudrate */
    speed_i = cfgetispeed(&setting);
    speed_o = cfgetospeed(&setting);
    printf("out baudrate: %d\n",comm_Brt_Map(speed_o));
    printf("in baudrate: %d\n",comm_Brt_Map(speed_i));

    return COMM_PORT_OK;
}

/*
 * set device baudrate
 * 300 - 921600
 */
CommErrorCode_e comm_Brt_Set(portObj_t *portObj)
{
    struct termios setting;
    unsigned int i;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    for(i=0; i<BRT_MAP_LEN; i++)
    {
        if(portObj->brt == brtMap[i].brt)
        {
            cfsetispeed(&setting, brtMap[i].brt_mask);
            cfsetospeed(&setting, brtMap[i].brt_mask);
            setting.c_cflag |= brtMap[i].brt_mask;
        }
    }

    if(tcsetattr(portObj->hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_FAIL;
    }

    return COMM_PORT_OK;
}

/*
 * set device icf
 * 8N1/7E1/7O1
 */
CommErrorCode_e comm_Icf_Set(portObj_t *portObj)
{
    struct termios setting;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    //printf("icf: %d\n", portObj->icf);
    switch(portObj->icf)
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
        case ICF_7S1:   //fall through
        default:
            setting.c_cflag &= ~PARENB;
            setting.c_cflag &= ~CSTOPB;
            setting.c_cflag &= ~CSIZE;
            setting.c_cflag |= CS8;
            break;
    }

    if(tcsetattr(portObj->hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_ICF_ERROR;
    }

    return COMM_PORT_OK;
}

/*
 * set device flowcontrl
 * RTSCTS/XONXOFF/NONE
 */
CommErrorCode_e comm_Fct_Set(portObj_t *portObj)
{
    struct termios setting;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return COMM_PORT_GET_ERROR;
    }

    switch(portObj->fct)
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

    if(tcsetattr(portObj->hd, TCSANOW, &setting) != 0)
    {
        return COMM_PORT_SET_ICF_ERROR;
    }

    return COMM_PORT_OK;
}


