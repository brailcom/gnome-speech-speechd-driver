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
 * speechdsynthesisdriver.h: Definition of the SpeechdSynthesisDriver
 *                            object-- a GNOME Speech driver for the Speechd
 *                            Speech Synthesis System (implementation in
 *                            speechdsynthesisdriver.c)
 *
 */

#ifndef __SPEECHD_SYNTHESIS_DRIVER_H_
#define __SPEECHD_SYNTHESIS_DRIVER_H_

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>
#include "speechdspeaker.h"

#define SPEECHD_SYNTHESIS_DRIVER_TYPE        (speechd_synthesis_driver_get_type ())
#define SPEECHD_SYNTHESIS_DRIVER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), SPEECHD_SYNTHESIS_DRIVER_TYPE, SpeechdSynthesisDriver))
#define SPEECHD_SYNTHESIS_DRIVER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), SPEECHD_SYNTHESIS_DRIVER_TYPE, SpeechdSynthesisDriverClass))
#define IS_SPEECHD_SYNTHESIS_DRIVER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), SPEECHD_SYNTHESIS_DRIVER_TYPE))
#define IS_SPEECHD_SYNTHESIS_DRIVER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SPEECHD_SYNTHESIS_DRIVER_TYPE))

typedef struct {
	BonoboObject parent;

	pid_t pid;
	gchar *version;
	gboolean initialized;
	SpeechdSpeaker *last_speaker;
	int sock;
	int pipe;
	GIOChannel *channel_sock;
	GIOChannel *channel_pipe;
	gboolean is_shutting_up; 
	gboolean is_speaking;
	GSList *list;
	gboolean is_querying; 
	gint crt_id;
	GSList *crt_clbs;
	int queue_length;
} SpeechdSynthesisDriver;

typedef struct {
	BonoboObjectClass parent_class;
	POA_GNOME_Speech_SynthesisDriver__epv epv;
} SpeechdSynthesisDriverClass;

GType
speechd_synthesis_driver_get_type   (void);

SpeechdSynthesisDriver *
speechd_synthesis_driver_new (void);
gint
speechd_synthesis_driver_say (SpeechdSynthesisDriver *d,
			       SpeechdSpeaker *s,
			       gchar *text);
gboolean
speechd_synthesis_driver_stop (SpeechdSynthesisDriver *d);
gboolean
speechd_synthesis_driver_is_speaking (SpeechdSynthesisDriver *d);
void
speechd_synthesis_driver_say_raw (SpeechdSynthesisDriver *d,
				   gchar *text);


  
#endif /* __SPEECHD_SYNTHESIS_DRIVER_H_ */
