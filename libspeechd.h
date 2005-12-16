
/*
 * libspeechd.h - functions for acces to Speech Dispatcher functions (header)
 *
 * Copyright (C) 2005 Brailcom, o.p.s.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id: libspeechd.h,v 1.1 2005-12-16 16:29:50 hanke Exp $
 */


#include <stdio.h>

#define SPEECHD_DEFAULT_PORT 6560

typedef enum 
{                  /* Type of voice */
    NO_VOICE = 0,
    MALE1 = 1,
    MALE2 = 2,
    MALE3 = 3,
    FEMALE1 = 4,
    FEMALE2 = 5,
    FEMALE3 = 6,
    CHILD_MALE = 7,
    CHILD_FEMALE = 8
}EVoiceType;

typedef enum
{
    SORT_BY_TIME = 0,
    SORT_BY_ALPHABET = 1
}ESort;

typedef enum
{
    MSGTYPE_TEXT = 0,
    MSGTYPE_SOUND_ICON = 1,
    MSGTYPE_CHAR = 2,
    MSGTYPE_KEY = 3,
    MSGTYPE_SPELL = 99
}EMessageType;

typedef enum
{
    RECOGN_NONE = 0,
    RECOGN_SPELL = 1,
    RECOGN_ICON = 2
}ECapLetRecogn;

typedef enum
{
    PUNCT_NONE = 0,
    PUNCT_ALL = 1,
    PUNCT_SOME = 2
}EPunctMode;

typedef enum
{
    SPELLING_OFF = 0,
    SPELLING_ON = 1
}ESpellMode;


/* Debugging */
FILE* spd_debug;
void SPD_DBG(char *format, ...);

/* Unless there is an fatal error, it doesn't print anything */
#define SPD_FATAL(msg) { printf("Fatal error (libspeechd) [%s:%d]:"msg, __FILE__, __LINE__); exit(EXIT_FAILURE); }

/* Arguments for spd_send_data() */
#define SPD_WAIT_REPLY 1              /* Wait for reply */
#define SPD_NO_REPLY 0               /* No reply requested */


/* --------------------- Public data types ------------------------ */

typedef enum{
    SPD_PUNCT_ALL = 0,
    SPD_PUNCT_NONE = 1,
    SPD_PUNCT_SOME = 2
}SPDPunctuation;

typedef enum{
    SPD_CAP_NONE = 0,
    SPD_CAP_SPELL = 1,
    SPD_CAP_ICON = 2
}SPDCapitalLetters;

typedef enum{
    SPD_SPELL_OFF = 0,
    SPD_SPELL_ON = 1
}SPDSpelling;

typedef enum{
    SPD_MALE1 = 1,
    SPD_MALE2 = 2,
    SPD_MALE3 = 3,
    SPD_FEMALE1 = 4,
    SPD_FEMALE2 = 5,
    SPD_FEMALE3 = 6,
    SPD_CHILD_MALE = 7,
    SPD_CHILD_FEMALE = 8
}SPDVoiceType;

typedef enum{
    SPD_IMPORTANT = 1,
    SPD_MESSAGE = 2,
    SPD_TEXT = 3,
    SPD_NOTIFICATION = 4,
    SPD_PROGRESS = 5
}SPDPriority;

/* -------------- Public functions --------------------------*/

/* Openning and closing Speech Dispatcher connection */
int spd_open(const char* client_name, const char* connection_name, const char *user_name);
void spd_close(int connection);

/* Speaking */
int spd_say(int connection, SPDPriority priority, const char* text);

/* Speech flow */
int spd_stop(int connection);

int spd_cancel(int connection);

int spd_pause(int connection);

int spd_resume(int connection);

/* Sound icons */
int spd_sound_icon(int connection, SPDPriority priority, const char *icon_name);

/* Setting parameters */
int spd_set_voice_type(int connection, const char *voice);

int spd_set_rate(int connection, signed int rate);

int spd_set_pitch(int connection, signed int pitch);

int spd_set_volume(int connection, signed int volume);

/* Direct SSIP communication */
int spd_execute_command(int connection, char* command);
char* spd_send_data(int fd, const char *message, int wfr);

/* --------------  Private functions  ------------------------*/

int spd__set_priority(int connection, SPDPriority priority);

char* spd__escape_dot(const char *otext);

int spd__isanum(char* str);		
void spd__free(void *ptr);
int spd__ret_ok(char *reply);
int spd__get_err_code(char *reply);
