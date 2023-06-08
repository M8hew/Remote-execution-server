#define log_filename "log"

enum {
    SPAWN = 1,

    SERV_LOG = 0,
    SERV_START = 0,
    SERV_STOP = 1,

    SES_CREATE = 1,
    SES_END = 4,
    SES_ERROR = 5,
};