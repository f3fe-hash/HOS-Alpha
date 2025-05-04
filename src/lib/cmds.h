#ifndef __CMDS_H__
#define __CMDS_H__

#include "../root.h"
#include "macro.h"

// HPM headers
#include "../HPM/hpm.h"
#include "../HPM/hpm_utils.h"

// Networking headers
#include "../Networking/netroot.h"
#include "../Networking/netutils.h"
#include "../Networking/ping.h"
#include "../Networking/NetWatch/netwatch.h"

// Crypto headers
#include "../Crypto/crypto.h"

// GTK
#include "gtk/gtk_main.h"

void cmd_clear();
void cmd_echo(char **tokens);
void cmd_exit();
void cmd_help();
void cmd_version();

// Networking commands
void cmd_netwatch(char **tokens);
void cmd_ping(char **tokens);

// Crypto commands
void cmd_hash(char **tokens);

// Package manager commands
void cmd_hpm(char **tokens);

#endif