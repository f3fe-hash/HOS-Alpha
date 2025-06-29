#ifndef __CMDS_H__
#define __CMDS_H__

#include <getopt.h>

#include "root.h"
#include "lib/macro.h"

// HPM headers
#include "HPM/hpm.h"

// net headers
#include "net/netroot.h"
#include "net/netutils.h"
#include "net/ping.h"
#include "net/NetWatch/netwatch.h"

// Crypto headers
#include "Crypto/crypto.h"

// GTK
#include "gtk/gtk_main.h"

#define CMD_RUN_HELP "etc/help/run.txt"
#define CMD_HPM_HELP "etc/help/hpm.txt"
#define CMD_HTP_HELP "etc/help/htp.txt"
#define CMD_HASH_HELP "etc/help/hash.txt"
#define CMD_PING_HELP "etc/help/ping.txt"
#define CMD_STORE_HELP "etc/help/store.txt"
#define CMD_NETWATCH_HELP "etc/help/netwatch.txt"

void cmd_clear      ();
void cmd_exit       ();
void cmd_version    ();
void cmd_echo       (int argc, char** argv);
void cmd_help       (int argc, char** argv);
void cmd_setport    (int argc, char** argv);
void cmd_netwatc    (int argc, char** argv);
void cmd_ping       (int argc, char** argv);
void cmd_hash       (int argc, char** argv);
void cmd_hpm        (int argc, char** argv);

// Utils
void read_help(const char* file);

#endif