#define PORT_PATH_MAX 64

typedef enum
{
    SPORT_OK                = 0,
    SPORT_OPEN_ERROR        = 0x100,
    SPORT_GET_ERROR         = 0x200,
    SPORT_GET_PARM_ERROR    = 0x201,

    SPORT_SET_FAIL          = 0x300,
    SPORT_SET_BRT_ERROR     = 0x301,
    SPORT_SET_ICF_ERROR     = 0x302,
    SPORT_SET_FCT_ERROR     = 0x303,

    SPORT_READ_ERROR        = 0x400,
    SPORT_WRITE_ERROR       = 0x500,
    SPORT_CLOSE_ERROR       = 0x600,

    SPORT_MAP_ERROR         = 0x700,
    SPORT_MAP_FCT_ERROR     = 0x701,
    SPORT_MAP_ICF_ERROR     = 0x702,

    SPORT_UNKNOWN_ERROR     = 0xFFFFFFFF,
}SportErrorCode_e;

typedef enum
{
    ICF_8N1 = 0,
    ICF_7E1,
    ICF_7O1,
    ICF_7S1,

    ICF_UNKNOWN = 0xFFFFFFFF,
}SportIcf_e;

typedef enum
{
    FCT_RTS_CTS = 0,
    FCT_XON_XOFF,
    FCT_NO,

    FCT_UNKNOWN = 0xFFFFFFFF,
}SportFct_e;

typedef struct
{
    int          hd;    /* device handle */
    char         portName[64];
    unsigned int brt;   /* baudrate */
    SportIcf_e    icf;   /* char framing */
    SportFct_e    fct;   /* flow control */
}SportObj_t;

SportErrorCode_e SPORT_Get_Opt(SportObj_t *portObj, int argc, char **argv);
SportErrorCode_e SPORT_Open_Port(SportObj_t *portObj);
SportErrorCode_e SPORT_Brt_Set(SportObj_t *portObj);
SportErrorCode_e SPORT_Icf_Set(SportObj_t *portObj);
SportErrorCode_e SPORT_Fct_Set(SportObj_t *portObj);
SportErrorCode_e SPORT_Close_Port(SportObj_t *portObj);
SportErrorCode_e SPORT_Get_Port_Atrribute(SportObj_t *portObj);
SportErrorCode_e SPORT_Write(SportObj_t *portObj, char *data);
SportErrorCode_e SPORT_Read(SportObj_t *portObj);

