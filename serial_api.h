#define PORT_PATH_MAX 64

typedef enum
{
    COMM_PORT_OK                = 0,
    COMM_PORT_OPEN_ERROR        = 0x100,
    COMM_PORT_GET_ERROR         = 0x200,
    COMM_PORT_GET_PARM_ERROR    = 0x201,

    COMM_PORT_SET_FAIL          = 0x300,
    COMM_PORT_SET_BRT_ERROR     = 0x301,
    COMM_PORT_SET_ICF_ERROR     = 0x302,
    COMM_PORT_SET_FCT_ERROR     = 0x303,

    COMM_PORT_CLOSE_ERROR       = 0x400,

    COMM_PORT_MAP_ERROR         = 0x500,
    COMM_PORT_MAP_FCT_ERROR     = 0x501,
    COMM_PORT_MAP_ICF_ERROR     = 0x502,

    COMM_PORT_UNKNOWN_ERROR     = 0xFFFFFFFF,
}CommErrorCode_e;

typedef enum
{
    ICF_8N1 = 0,
    ICF_7E1,
    ICF_7O1,
    ICF_7S1,

    ICF_UNKNOWN = 0xFFFFFFFF,
}CommIcf_e;

typedef enum
{
    FCT_RTS_CTS = 0,
    FCT_XON_XOFF,
    FCT_NO,

    FCT_UNKNOWN = 0xFFFFFFFF,
}CommFct_e;

typedef struct
{
    int          hd;    /* device handle */
    char         portName[64];
    unsigned int brt;   /* baudrate */
    CommIcf_e    icf;   /* char framing */
    CommFct_e    fct;   /* flow control */
}portObj_t;

CommErrorCode_e comm_Get_Opt(portObj_t *portObj, int argc, char **argv);
CommErrorCode_e comm_Open_Port(portObj_t portObj);
CommErrorCode_e comm_Close_Port(portObj_t portObj);
