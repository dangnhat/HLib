/*
 * sim900_cmds.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: nvhien1992
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern "C" {
#include "msg.h"
}

#include "sim900_cmds.h"
#include "mbdemo_glb.h"

char phone_number[15];

const char call_cmd_usage[] = "Usage:\n"
        "call phone-number, make a voice call to phone-number.\n"
        "-h, get this help.\n";

static bool numbers_only(const char *s);

/**
 *
 */
void send_sms_demo(int argc, char** argv)
{
    printf("<3 not implemented <3\n");
}

/**
 *
 */
void make_voicecall_demo(int argc, char** argv)
{
    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    if (argc > 2) {
        printf("ERR: too many argument. Try -h to get help.\n");
        return;
    }

    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(call_cmd_usage);
                return;
            }
        } else {
            if (strlen(argv[count]) > 11 || strlen(argv[count]) < 10
                    || !numbers_only(argv[count])) {
                printf("Invalid phone number\n");
                return;
            }
            strcpy(phone_number, argv[count]);

            //send phone number to sim900 thread
            msg_t msg;
            msg.type = SHELL_ID;
            msg.content.ptr = phone_number;
            msg_send(&msg, mbdemo_ns::thread_pid[0], false);
        }
    }
}

static bool numbers_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0)
            return false;
    }

    return true;
}
