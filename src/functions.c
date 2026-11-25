#include "parse_cmd.h"
#ifndef NULL
#define NULL ((void*)0)
#endif
int any_test(int argc,char *argv[],void *cfg);
int rename_action(int argc, char *argv[],void *cfg);
const WB_CMD wb_cmds[] = {
    {"rename", rename_action},
    {"test",any_test},
    {NULL, NULL}
};