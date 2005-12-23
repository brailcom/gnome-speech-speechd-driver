/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2002 Sun Microsystems Inc.
 * Copyright 2005 Brailcom, o.p.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * speechdspeaker.c.c: Implementation of the SpeechdSynthesisDriver
 *                            object-- a GNOME Speech driver for the Speechd
 *                            Speech Synthesis System
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libbonobo.h>
#include "speechdsynthesisdriver.h"
#include "speechdspeaker.h"
#include "libspeechd.h"

/* Put a message into the logfile (stderr currently) */
#define DBG(arg...) \
{ \
    fprintf(stderr," SpeechdSpeaker: "); \
    fprintf(stderr,arg); \
    fprintf(stderr,"\n"); \
    fflush(stderr); \
}

#define RET_STATUS(status) \
     if (status == 0) return TRUE; \
     else return FALSE;

#define GET_SPEAKER() \
	SpeechdSpeaker *speaker = speechd_speaker_from_servant (servant); \

#define GET_DRIVER() \
	Speaker *s = SPEAKER (speaker); \
        SpeechdSynthesisDriver *d = SPEECHD_SYNTHESIS_DRIVER(s->driver);

static GObjectClass *parent_class;


static SpeechdSpeaker *
speechd_speaker_from_servant (PortableServer_Servant *servant)
{
	return SPEECHD_SPEAKER(bonobo_object_from_servant (servant));
}



static CORBA_long
speechd_say (PortableServer_Servant servant,
	      const CORBA_char *text,
	      CORBA_Environment *ev)
{
	static int counter = 1;
	int ret;
    
	GET_SPEAKER();
	GET_DRIVER();

	/* Refresh if needded */
        if (d->last_speaker != speaker || speaker_needs_parameter_refresh (SPEAKER(speaker)))
	    {
		/* if (!d->last_speaker || strcmp (d->last_speaker->voice, s->voice))*/
		spd_set_voice_type(speaker->conn, speaker->voice);
		speaker_refresh_parameters (SPEAKER(s));
		d->last_speaker = speaker;
	    }

	ret = spd_say(speaker->conn, SPD_MESSAGE, text);       
	if (ret < 0) return -1;

	return counter++;
}



static CORBA_boolean
speechd_stop (PortableServer_Servant servant,
	       CORBA_Environment *ev)
{
        int ret;
	GET_SPEAKER();

	ret = spd_cancel(speaker->conn);

	RET_STATUS(ret);
}


static CORBA_boolean 
speechd_isSpeaking (PortableServer_Servant servant, 
 		     CORBA_Environment *ev) 
{ 
	/* Speech Dispatcher is currently not able to provide this
	   capability */

	return FALSE; 
} 


static CORBA_boolean 
speechd_registerSpeechCallback (PortableServer_Servant servant, 
 				 const GNOME_Speech_SpeechCallback callback, 
 				 CORBA_Environment *ev) 
{
	/* Speech Dispatcher is currently not able to provide this
	   capability */

	return FALSE; 
}

static gboolean
festival_set_rate (Speaker *speaker,
		   gdouble new_value)
{
    fprintf(stderr, "festival_set_rate\n");
    fflush(stderr);
    
    return TRUE;
}


#define SPEECHD_SET_NUM(name) \
  static gboolean \
  speechd_set_ ## name (Speaker *speaker, \
     		         gdouble new_value) \
   { \
        DBG("Setting parameter " #name " to value %d", (int) new_value); \
        int ret; \
	SpeechdSpeaker *speechd_speaker = SPEECHD_SPEAKER (speaker); \
	ret = spd_set_ ## name (speechd_speaker->conn, (int) new_value); \
	RET_STATUS(ret); \
   }

SPEECHD_SET_NUM(rate);
SPEECHD_SET_NUM(pitch);
SPEECHD_SET_NUM(volume);

static void
speechd_speaker_init (SpeechdSpeaker *s) 
{
        int ret;
	s->voice = NULL; 
	SPEAKER(s)->clb_list = NULL; 

	s->conn = spd_open("gnomespeech", "main", "unknown");

	ret = spd_set_voice_type(s->conn, s->voice);
}



static void
speechd_speaker_finalize (GObject *obj)
{
	SpeechdSpeaker *s = SPEECHD_SPEAKER (obj);
	Speaker *speaker = SPEAKER (s);

	if (speaker->driver)
		g_object_unref (speaker->driver);

	if (s->voice)
		g_free (s->voice);

	if (parent_class->finalize)
		parent_class->finalize (obj);
}



static void
speechd_speaker_class_init (SpeechdSpeakerClass *klass)
{
	SpeakerClass *class = SPEAKER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = speechd_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = speechd_say;
	class->epv.stop = speechd_stop;
	class->epv.isSpeaking = speechd_isSpeaking; 
	class->epv.registerSpeechCallback = speechd_registerSpeechCallback;
}



BONOBO_TYPE_FUNC (SpeechdSpeaker,
		  speaker_get_type (),
		  speechd_speaker);



#define ADD_PARAM(name, min, def, max) \
    speaker_add_parameter (s, \
	                  #name, \
                           min, \
                           def, \
			   max, \
			   speechd_set_ ## name);

SpeechdSpeaker *
speechd_speaker_new (GObject *driver,
		      const GNOME_Speech_VoiceInfo *info)
{
	SpeechdSpeaker *speaker;
	Speaker *s;

	
	speaker = g_object_new (SPEECHD_SPEAKER_TYPE, NULL);
	s = SPEAKER (speaker);
       	s->driver = g_object_ref (driver);

	DBG("Adding parameters");
	/* Add supported parameters */
	ADD_PARAM(rate, -100, 0, 100);
	ADD_PARAM(pitch, -100, 0, 100);
	ADD_PARAM(volume, -100, 0, 100);

	if (info != NULL && info->name != NULL)	speaker->voice = strdup(info->name);
	

	return speaker;
}
