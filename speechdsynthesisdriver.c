/*
 *
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2002 Sun Microsystems.
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
 * speechdsynthesisdriver.c: Implementation of the SpeechDispatcherSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <libbonobo.h>
#include <netdb.h>
#include "speechdsynthesisdriver.h"
#include "speechdspeaker.h"
#include "libspeechd.h"

/* Put a message into the logfile (stderr) */
#define DBG(arg...) \
{ \
    fprintf(stderr," SpeechdSynthesisDriver: "); \
    fprintf(stderr,arg); \
    fprintf(stderr,"\n"); \
    fflush(stderr); \
}


static GObjectClass 	*parent_class;
static gboolean 	speechd_server_got_voices = FALSE;
static GSList		*driver_list = NULL;
static GSList		*voices_list = NULL;

static void 	speechd_add_driver	    	(SpeechdSynthesisDriver *d);
static void 	speechd_remove_driver 		(SpeechdSynthesisDriver *d);
static gboolean speechd_is_any_driver_alive 	(void);


static void 
speechd_add_driver (SpeechdSynthesisDriver *d)
{
	driver_list = g_slist_append (driver_list, d);
}


static void 
speechd_remove_driver (SpeechdSynthesisDriver *d)
{
	driver_list = g_slist_remove (driver_list, d);
}



static gboolean 
speechd_is_any_driver_alive ()
{
	return driver_list != NULL;    
}


static SpeechdSynthesisDriver *
speechd_synthesis_driver_from_servant (PortableServer_Servant *servant)
{
	return SPEECHD_SYNTHESIS_DRIVER(bonobo_object_from_servant(servant));
}


static CORBA_string
speechd__get_driverName (PortableServer_Servant servant,
			  CORBA_Environment * ev)
{
	return CORBA_string_dup ("Speech Dispatcher GNOME Speech Driver");  
}




static CORBA_string
speechd__get_synthesizerName (PortableServer_Servant servant,
			       CORBA_Environment * ev)
{
	return CORBA_string_dup ("Speech Dispatcher");
}



static CORBA_string
speechd__get_driverVersion (PortableServer_Servant aservant,
			     CORBA_Environment * ev)
{
	return CORBA_string_dup ("0.1");
}



static CORBA_string
speechd__get_synthesizerVersion (PortableServer_Servant servant,
				  CORBA_Environment * ev)
{
	SpeechdSynthesisDriver *d = speechd_synthesis_driver_from_servant (servant);
	return CORBA_string_dup (d->version);
}

static void
voice_list_free (GSList *l)
{
	g_assert (l);
	GSList *tmp = l;
	
	while (tmp)
	{
    	    CORBA_free (tmp->data);
	    tmp = tmp->next;
	}
	g_slist_free (l);
}



static GSList *
get_voice_list_with_spec (GSList *l,
			  const GNOME_Speech_VoiceInfo *info)
{
	GSList *tmp, *new;
	GNOME_Speech_VoiceInfo *i, *new_info;

	new = NULL;
	tmp = l;
	while (tmp) {
		i = (GNOME_Speech_VoiceInfo *) tmp->data;
		if ((strlen(info->name) ? strcmp (i->name, info->name) : 1) && 
		    (strlen(info->language) ? strcmp (i->language, info->language) : 1) &&
		    (info->gender == i->gender)) {
			new_info = GNOME_Speech_VoiceInfo__alloc ();
            		new_info->language = CORBA_string_dup(info->language);
            		new_info->name = CORBA_string_dup (info->name);
            		new_info->gender = info->gender;
			new = g_slist_prepend (new, new_info);
		}
		tmp = tmp->next;
	}
	if (new == NULL) return NULL;

	new = g_slist_reverse (new);
	return new;
}


static GNOME_Speech_VoiceInfoList *
voice_info_list_from_voice_list (GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc ();
  
	if (!l) {
		rv->_length = rv->_maximum = 0;
		return rv ;
	}

	rv->_length = rv->_maximum = g_slist_length (l);
	rv->_buffer = GNOME_Speech_VoiceInfoList_allocbuf (rv->_length);

	while (l) {
		GNOME_Speech_VoiceInfo *info =
			(GNOME_Speech_VoiceInfo *) l->data;
		rv->_buffer[i].name = CORBA_string_dup (info->name);
		rv->_buffer[i].gender = info->gender;
		rv->_buffer[i].language = CORBA_string_dup(info->language);
		i++;
		l = l->next;
	}
	return rv;
}



static GNOME_Speech_VoiceInfoList *
speechd_getVoices (PortableServer_Servant servant,
		    const GNOME_Speech_VoiceInfo *voice_spec,
		    CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	DBG("speechd_getVoices");

	l = get_voice_list_with_spec (voices_list, voice_spec);
	if (l == NULL) return NULL;

	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_VoiceInfoList *
speechd_getAllVoices (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;

	DBG("speechd_getAllVoices");

	rv = voice_info_list_from_voice_list (voices_list);
	return rv;
}



static GNOME_Speech_Speaker
speechd_createSpeaker (PortableServer_Servant servant,
			const GNOME_Speech_VoiceInfo *voice_spec,
			CORBA_Environment *ev)
{
	SpeechdSynthesisDriver *d = speechd_synthesis_driver_from_servant (servant);
	SpeechdSpeaker *s;
	GNOME_Speech_VoiceInfo *info;
	GSList *l;

	DBG("speechd_createSpeaker");
	
	l = get_voice_list_with_spec (voices_list, voice_spec);
	
	if (l)
		info = l->data;
	else
		info = NULL;

	s = speechd_speaker_new (G_OBJECT(d), info);
	/* FIXME: shound't we free l here? */
	return CORBA_Object_duplicate(bonobo_object_corba_objref (BONOBO_OBJECT(s)), ev);
}


#define ADD_VOICE(voice_name, voicegen) \
	new_info = GNOME_Speech_VoiceInfo__alloc (); \
	new_info->name = CORBA_string_dup (voice_name); \
	new_info->language = CORBA_string_dup ("any"); \
	new_info->gender = GNOME_Speech_gender_##voicegen; \
	voices_list = g_slist_append (voices_list, new_info);

static CORBA_boolean
speechd_driverInit (PortableServer_Servant servant,
		     CORBA_Environment *ev)
{
        GNOME_Speech_VoiceInfo *new_info;
	
        SpeechdSynthesisDriver *d = speechd_synthesis_driver_from_servant (servant);

	DBG("speechd_driverInit");

	if (d->initialized) return TRUE;

	d->version = strdup("unknown");
	if (!d->version) return FALSE;

	DBG("adding voices to list");
	ADD_VOICE("MALE1", male);
	ADD_VOICE("MALE2", male);
	ADD_VOICE("MALE3", male);
	ADD_VOICE("FEMALE1", female);
	ADD_VOICE("FEMALE2", female);
	ADD_VOICE("FEMALE3", female);
	ADD_VOICE("CHILD_MALE", male);
	ADD_VOICE("CHILD_FEMALE", female);

	speechd_server_got_voices = TRUE;	
	d->initialized = TRUE;

	DBG("driver initialization finished");
	return TRUE;
}


static CORBA_boolean
speechd_isInitialized (PortableServer_Servant servant,
			CORBA_Environment *ev)
{
	SpeechdSynthesisDriver *d = speechd_synthesis_driver_from_servant (servant);

	return d->initialized;
}


static void
speechd_synthesis_driver_init (SpeechdSynthesisDriver *d)
{
        d->version = NULL;
        d->last_speaker = NULL;
        d->initialized = FALSE;
        d->sock = -1;
	d->pipe = -1;
        d->channel_sock = NULL;
        d->channel_pipe = NULL;	
        d->crt_clbs = NULL;
        d->is_shutting_up = FALSE;
	d->is_speaking = FALSE;
        d->is_querying = FALSE;
        d->list = NULL;
        d->crt_id = 0;
        d->queue_length = 0;	
}


static void
speechd_synthesis_driver_finalize (GObject *obj)
{
        SpeechdSynthesisDriver *d = SPEECHD_SYNTHESIS_DRIVER (obj);
    	
	if (parent_class->finalize)
    	    parent_class->finalize (obj);
		
	speechd_remove_driver (d);
	
        if (!speechd_is_any_driver_alive ())
	    bonobo_main_quit ();
}


static void
speechd_synthesis_driver_class_init (SpeechdSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = speechd_synthesis_driver_finalize;
	
	/* Initialize epv table */

	klass->epv._get_driverName = speechd__get_driverName;
	klass->epv._get_driverVersion = speechd__get_driverVersion;
	klass->epv._get_synthesizerName = speechd__get_synthesizerName;
	klass->epv._get_synthesizerVersion = speechd__get_synthesizerVersion;
	klass->epv.getVoices = speechd_getVoices;
	klass->epv.getAllVoices = speechd_getAllVoices;
	klass->epv.createSpeaker = speechd_createSpeaker;
	klass->epv.driverInit = speechd_driverInit;
	klass->epv.isInitialized = speechd_isInitialized;
}


BONOBO_TYPE_FUNC_FULL (SpeechdSynthesisDriver,
		       GNOME_Speech_SynthesisDriver,
		       bonobo_object_get_type (),
		       speechd_synthesis_driver);


SpeechdSynthesisDriver * 
speechd_synthesis_driver_new (void)
{
        SpeechdSynthesisDriver *driver;
	
	driver = g_object_new (SPEECHD_SYNTHESIS_DRIVER_TYPE, NULL);
	speechd_add_driver (driver);
    
	return driver;
}


int
main (int  argc,
      char **argv)
{
        SpeechdSynthesisDriver *driver;
	char 	*obj_id;
	int 	ret;

        driver_list = NULL;

	if (!bonobo_init (&argc, argv))
        {
    	    g_error ("Could not initialize Bonobo Activation / Bonobo");
	}

        obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Speech_Dispatcher:proto0.3";

	driver = speechd_synthesis_driver_new ();

	if (!driver)
    	    g_error ("Error creating speech synthesis driver object.\n");

        ret = bonobo_activation_active_server_register (obj_id,
					                bonobo_object_corba_objref (bonobo_object (driver)));

        if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
	    g_error ("Error registering speech synthesis driver.\n");
	else
	    bonobo_main ();

        g_assert (driver_list == NULL);
	bonobo_debug_shutdown ();	

        return 0;
}
