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
    SportIcf_e   icf;
    const char  *s_opt;
}icfMap_t;

typedef struct
{
    SportFct_e   ftc;
    const char  *s_opt;
}fctMap_t;

typedef struct
{
    unsigned int    brt;
    speed_t         brt_mask;
}brtMap_t;

/***** local function declare *****/
static SportErrorCode_e SPORT_Fct_Map(SportObj_t *portObj, char *opt);
static SportErrorCode_e SPORT_Icf_Map(SportObj_t *portObj, char *opt);
static unsigned int SPORT_Brt_Map(speed_t speed);

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
static unsigned int SPORT_Brt_Map(speed_t speed)
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

static SportErrorCode_e SPORT_Fct_Map(SportObj_t *portObj, char *opt)
{
    unsigned int i;

    for(i=0; i<FCT_MAP_LEN; i++)
    {
        if(strncmp(opt, fctMap[i].s_opt, strlen(fctMap[i].s_opt)) == 0)
        {
            portObj->fct = fctMap[i].ftc;
            return SPORT_OK;
        }
    }

    return SPORT_MAP_FCT_ERROR;
}

static SportErrorCode_e SPORT_Icf_Map(SportObj_t *portObj, char *opt)
{
    unsigned int i;

    for(i=0; i<ICF_MAP_LEN; i++)
    {
        if(strncmp(opt, icfMap[i].s_opt, strlen(icfMap[i].s_opt)) == 0)
        {
            portObj->icf = icfMap[i].icf;
            return SPORT_OK;
        }
    }

    return SPORT_MAP_ICF_ERROR;
}

/**** public function *****/
/*
 * demo function for get device param from cmd
 */
SportErrorCode_e SPORT_Get_Opt(SportObj_t *portObj, int argc, char **argv)
{
    while(1)
    {
        int c = 0 ;
        int optIndex = 0;
        SportErrorCode_e res;

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
                    return SPORT_GET_PARM_ERROR;
                }
                break;
            case 'b':
                portObj->brt = (unsigned int)atoi(optarg);
                break;
            case 'f':
                res = SPORT_Fct_Map(portObj, optarg);
                if(res != SPORT_OK)
                {
                    return res;
                }
                break;
            case 'i':
                res = SPORT_Icf_Map(portObj, optarg);
                if(res != SPORT_OK)
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

        return SPORT_GET_PARM_ERROR;
    }

    return SPORT_OK;
}

/*
 * open the device with:
 * 1. baudrate setting
 * 2. icf setting
 * 3. flowcontrol setting
 */
SportErrorCode_e SPORT_Open_Port(SportObj_t *portObj)
{
    struct termios  setting;
    SportErrorCode_e res = SPORT_OK;

    //portObj->hd = open(portObj->portName, O_RDWR | O_NOCTTY | O_NDELAY);
    portObj->hd = open(portObj->portName, O_RDWR | O_NOCTTY);
    if(portObj->hd < 0)
    {
        portObj->hd = INVALID_HANDLE;
        return SPORT_OPEN_ERROR;
    }

    //fcntl(portObj->hd, F_SETFL, FNDELAY);
    //fcntl(portObj->hd, F_SETFL, 0);

    memset(&setting, 0x0, sizeof(setting));

    /* set baud rate */
    res = SPORT_Brt_Set(portObj);
    if(res != SPORT_OK)
    {
        return res;
    }

    /* set icf */
    res = SPORT_Icf_Set(portObj);
    if(res != SPORT_OK)
    {
        return res;
    }

    /* set flow control */
    res = SPORT_Fct_Set(portObj);
    if(res != SPORT_OK)
    {
        return res;
    }

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return SPORT_GET_ERROR;
    }
    setting.c_cflag |= CREAD | CLOCAL;
#if 1
    setting.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
#else
    setting.c_iflag |= (ICANON | ECHO | ECHOE |ICRNL);
#endif
    //setting.c_cc[VTIME] = 0;
    //setting.c_cc[VMIN] = 10;

    /* update termiox NOW */
    tcflush(portObj->hd, TCIFLUSH);
    if(tcsetattr(portObj->hd, TCSANOW, &setting) != 0)
    {
        close(portObj->hd);
        portObj->hd = INVALID_HANDLE;

        return SPORT_SET_FAIL;
    }

    return res;
}

/*
 * close the device
 */
SportErrorCode_e SPORT_Close_Port(SportObj_t *portObj)
{
    if(portObj->hd != INVALID_HANDLE)
    {
        close(portObj->hd);
        portObj->hd = INVALID_HANDLE;
    }
    return SPORT_OK;
}

/*
 * get device attribute
 */
SportErrorCode_e SPORT_Get_Port_Atrribute(SportObj_t *portObj)
{
    struct termios setting;
#ifdef _DEBUG_
    speed_t speed_o, speed_i;

    printf("device: %s\n", portObj->portName);
    printf("hd: %d\n", portObj->hd);
#endif

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return SPORT_GET_ERROR;
    }

#ifdef _DEBUG_
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
    printf("out baudrate: %d\n",SPORT_Brt_Map(speed_o));
    printf("in baudrate: %d\n",SPORT_Brt_Map(speed_i));
#endif

    return SPORT_OK;
}

/*
 * set device baudrate
 * 300 - 921600
 */
SportErrorCode_e SPORT_Brt_Set(SportObj_t *portObj)
{
    struct termios setting;
    unsigned int i;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return SPORT_GET_ERROR;
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
        return SPORT_SET_FAIL;
    }

    return SPORT_OK;
}

/*
 * set device icf
 * 8N1/7E1/7O1
 */
SportErrorCode_e SPORT_Icf_Set(SportObj_t *portObj)
{
    struct termios setting;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return SPORT_GET_ERROR;
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
        return SPORT_SET_ICF_ERROR;
    }

    return SPORT_OK;
}

/*
 * set device flowcontrl
 * RTSCTS/XONXOFF/NONE
 */
SportErrorCode_e SPORT_Fct_Set(SportObj_t *portObj)
{
    struct termios setting;

    if(tcgetattr(portObj->hd, &setting) != 0)
    {
        return SPORT_GET_ERROR;
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
        return SPORT_SET_ICF_ERROR;
    }

    return SPORT_OK;
}

/*
 * write data to port
 */
SportErrorCode_e SPORT_Write(SportObj_t *portObj, char * data)
{
    int nbytes;

    nbytes = write(portObj->hd, data, strlen(data));

    if(nbytes > 0)
    {
#ifdef _DEBUG_
        printf("%d byte(s) written to %s\n", nbytes, portObj->portName);
#endif
        return SPORT_OK;
    }
    else
    {
        return SPORT_WRITE_ERROR;
    }
}

/*
 * read 1 byte
 */
static int read_one_byte(SportObj_t *portObj, char *data)
{
    return  read(portObj->hd, data, 1);
}

/*
 * read data from port
 */
SportErrorCode_e SPORT_Read(SportObj_t *portObj)
{
    static const char *res[] = { "OK", "ERROR", "CONNECT" };
    char buff[256];
    unsigned int i;
    unsigned int flag = 1;

    usleep(50000);
    while(flag)
    {
        int nbytes;
        memset(buff, 0x0, sizeof(buff));
        nbytes = read(portObj->hd, buff, 256);

        if(nbytes > 0)
        {
            buff[nbytes] = 0;
            printf("%s\n", (char *)buff);

            for(i=0; i<(sizeof(res)/sizeof(res[0])); i++)
            {
                if(strstr(buff, res[i]) != NULL)
                {
                    flag = 0;
                }
            }
        }
    }

    return SPORT_OK;
}

