
/*
 * libspeechd.c - Functions for acces to Speech Dispatcher functions
 *
 * Copyright (C) 2005 Brailcom, o.p.s.
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Library General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this package; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * $Id: libspeechd.c,v 1.1.1.1 2005-12-16 16:29:48 hanke Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#include "libspeechd.h"

#ifdef SPD_DEBUG
#define DBG(arg...) \
{ \
    fprintf(stderr," SpeechdOutputLibrary: "); \
    fprintf(stderr,arg); \
    fprintf(stderr,"\n"); \
    fflush(stderr); \
}
#else
#define DBG(arg...) 
#endif

/* --------------------- Public functions ------------------------- */

/* Opens a new Speech Dispatcher connection.
 * Returns socket file descriptor of the created connection
 * or -1 if no connection was opened. */
int 
spd_open(const char* client_name, const char* connection_name, const char* user_name)
{
  struct sockaddr_in address;
  int connection;
  char *set_client_name;
  char* conn_name;
  char* usr_name;
  char *env_port;
  int port;
  int ret;
  char tcp_no_delay = 1;

  if (client_name == NULL) return -1;

  usr_name = strdup(user_name);
  
  if(connection_name == NULL)
      conn_name = strdup("main");
  else
      conn_name = strdup(connection_name);

  env_port = getenv("SPEECHD_PORT");
  if (env_port != NULL)
      port = strtol(env_port, NULL, 10);
  else
      port = SPEECHD_DEFAULT_PORT;
  
  /* Prepare a new socket */
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(port);
  address.sin_family = AF_INET;
  connection = socket(AF_INET, SOCK_STREAM, 0);

  /* Connect to server */
  ret = connect(connection, (struct sockaddr *)&address, sizeof(address));
  if (ret == -1){
      DBG("Error: Can't connect to server: %s", strerror(errno));
      return 0;
  }

  setsockopt(connection, IPPROTO_TCP, TCP_NODELAY, &tcp_no_delay, sizeof(int));
 
  set_client_name = g_strdup_printf("SET SELF CLIENT_NAME \"%s:%s:%s\"", usr_name,
          client_name, conn_name);

  ret = spd_execute_command(connection, set_client_name);

  spd__free(usr_name);
  spd__free(conn_name);
  spd__free(set_client_name);

  return connection;
}


/* Close a Speech Dispatcher connection */
void
spd_close(int connection)
{
    spd_execute_command(connection, "QUIT");
    
    /* close the socket */
    close(connection);
}

/* Say TEXT with priority PRIORITY.
 * Returns 0 on success, -1 otherwise. */                            
int
spd_say(int connection, SPDPriority priority, const char* text)
{
    static char command[16];
    char *etext;
    int ret;

    if (text == NULL) return -1;

    /* Set priority */
    ret = spd__set_priority(connection, priority);
    if(ret) return -1;
    
    /* Check if there is no escape sequence in the text */
    etext = spd__escape_dot(text);
    if (etext == NULL) etext = (char*) text;
  
    /* Start the data flow */
    sprintf(command, "SPEAK");
    ret = spd_execute_command(connection, command);
    if(ret){     
        DBG("Error: Can't start data flow!");
        return -1;
    }
  
    /* Send data */
    spd_send_data(connection, etext, SPD_NO_REPLY);

    /* Terminate data flow */
    ret = spd_execute_command(connection, "\r\n.");
    if(ret){
        DBG("Can't terminate data flow");
        return -1; 
    }

    return 0;
}

int
spd_stop(int connection)
{
  return spd_execute_command(connection, "STOP SELF");
}

int
spd_cancel(int connection)
{
  return spd_execute_command(connection, "CANCEL SELF");
}


int
spd_pause(int connection)
{
  return spd_execute_command(connection, "PAUSE SELF");
}

int
spd_resume(int connection)
{
  return spd_execute_command(connection, "RESUME SELF");
}

int
spd_sound_icon(int connection, SPDPriority priority, const char *icon_name)
{
    char *command;
    int ret;

    if (icon_name == NULL) return -1;

    ret = spd__set_priority(connection, priority);
    if (ret) return -1;

    command = g_strdup_printf("SOUND_ICON %s", icon_name);
    ret = spd_execute_command(connection, command);
    spd__free (command);
    if (ret) return -1;
    
    return 0;
}

#define SPD_SET_COMMAND_INT(param, ssip_name, condition) \
    int \
    spd_set_ ## param (int connection, signed int val) \
    { \
        static char command[64]; \
        if ((!condition)) return -1; \
        sprintf(command, "SET SELF " #ssip_name " %d", val); \
        return spd_execute_command(connection, command); \
    } \

#define SPD_SET_COMMAND_STR(param, ssip_name) \
    int \
    spd_set_ ## param (int connection, const char *str) \
    { \
        char *command; \
        int ret; \
        if (str == NULL) return -1; \
        command = g_strdup_printf("SET SELF " #param " %s", str); \
        ret = spd_execute_command(connection, command); \
        spd__free(command); \
        return ret; \
    } \

SPD_SET_COMMAND_INT(rate, RATE, ((val >= -100) && (val <= +100)) );
SPD_SET_COMMAND_INT(pitch, PITCH, ((val >= -100) && (val <= +100)) );
SPD_SET_COMMAND_INT(volume, VOLUME, ((val >= -100) && (val <= +100)) );

SPD_SET_COMMAND_STR(voice_type, VOICE);

#undef SPD_SET_COMMAND_INT
#undef SPD_SET_COMMAND_STR

int
spd_execute_command(int connection, char* command)
{
    char *buf;
    char *ret;
    int r;
    
    buf = g_strdup_printf("%s\r\n", command);
    ret = spd_send_data(connection, buf, SPD_WAIT_REPLY);
    spd__free(buf);
    
    r = spd__ret_ok(ret);
    spd__free(ret);

    if (!r) return -1;
    else return 0;
}

#define SPD_MAX_REPLY_LENGTH 1024
char*
spd_send_data(int fd, const char *message, int wfr)
{
    char *reply;
    int bytes;

    reply = malloc(sizeof(char) * SPD_MAX_REPLY_LENGTH);
   
    /* write message to the socket */
    write(fd, message, strlen(message));
    DBG(">> : |%s|", message);

    /* read reply to the buffer */
    if (wfr){
        bytes = read(fd, reply, SPD_MAX_REPLY_LENGTH);
        if (bytes == -1){
            DBG("Error: Can't read reply, broken socket.");
            return NULL;
        }
        /* print server reply to as a string */
        reply[bytes] = 0; 
        DBG("<< : |%s|\n", reply);
    }else{
        DBG("<< : no reply expected");
        return "NO REPLY";
    } 

    return reply;
}


/* --------------------- Internal functions ------------------------- */

int
spd__set_priority(int connection, SPDPriority priority)
{
    static char p_name[16];
    static char command[64];

    switch(priority){
    case SPD_IMPORTANT: strcpy(p_name, "IMPORTANT"); break;
    case SPD_MESSAGE: strcpy(p_name, "MESSAGE"); break;
    case SPD_TEXT: strcpy(p_name, "TEXT"); break;
    case SPD_NOTIFICATION: strcpy(p_name, "NOTIFICATION"); break;
    case SPD_PROGRESS: strcpy(p_name, "PROGRESS"); break;
    default: 
        DBG("Error: Can't set priority! Incorrect value.");
        return -1;
    }
		 
    sprintf(command, "SET SELF PRIORITY %s", p_name);
    return spd_execute_command(connection, command);
}

int
spd__ret_ok(char *reply)
{
	int err;

	err = spd__get_err_code(reply);
		
	if ((err>=100) && (err<300)) return 1;
	if (err>=300) return 0;

	SPD_FATAL("Internal error during communication.");
}

int
spd__get_err_code(char *reply)
{
    char err_code[4];
    int err;
	
    if (reply == NULL) return -1;
    DBG("spd_send_data:	reply: %s\n", reply);

    err_code[0] = reply[0];	err_code[1] = reply[1];
    err_code[2] = reply[2];	err_code[3] = '\0';

    DBG("spd__ret_ok: err_code:	|%s|\n", err_code);
   
    if(spd__isanum(err_code)){
        err = atoi(err_code);
    }else{
        DBG("spd__ret_ok: not a number\n");
        return -1;	
    }

    return err;
}

/* spd__isanum() tests if the given string is a number,
 *  returns 1 if yes, 0 otherwise. */
int
spd__isanum(char *str)
{
    int i;
    if (str == NULL) return 0;
    for(i=0;i<=strlen(str)-1;i++){
        if (!isdigit(str[i]))   return 0;
    }
    return 1;
}

void
spd__free(void *ptr)
{
    if (ptr != NULL){
        free(ptr);
        ptr = NULL;
    }
}

char*
spd__escape_dot(const char *text)
{
    char *seq;
    GString *ntext;
    char *p;
    char *otext;
    char *ret = NULL;
    int len;

    if (text == NULL) return NULL;

    DBG("Incomming text to escaping: |%s|", text);

    p = (char*) text;
    otext = p;

    ntext = g_string_new("");

    len = strlen(text);
    if (len == 1){
        if (!strcmp(text, ".")){
            g_string_append(ntext, "..");
            p += 1;
        }
    }
    else if (len >= 3){
        if ((p[0] == '.') && (p[1] == '\r') && (p[2] == '\n')){
            g_string_append(ntext, "..\r\n");
            p += 3;
        }
    }

    //    DBG("Altering text (I): |%s|", ntext->str);

    while ((seq = strstr(p, "\r\n.\r\n"))){
        assert(seq>p);
        g_string_append_len(ntext, p, seq-p);
        g_string_append(ntext, "\r\n..\r\n");
        p = seq+5;
    }

    //   DBG("Altering text (II): |%s|", ntext->str);    

    len = strlen(p);
    if (len >= 3){
        if ((p[len-3] == '\r') && (p[len-2] == '\n')
            && (p[len-1] == '.')){
            g_string_append(ntext, p);
            g_string_append(ntext, ".");
            p += len;
            //  DBG("Altering text (II-b): |%s|", ntext->str);    
        }
    }

    if (p == otext){
        DBG("No escaping needed.");
        g_string_free(ntext, 1);
        return NULL;
    }else{
        g_string_append(ntext, p);
        ret = ntext->str;
        g_string_free(ntext, 0);
    }

    DBG("Altered text after escaping: |%s|", ret);

    return ret;
}

