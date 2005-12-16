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
 * speechdspeaker.h: Definition of the SpeechdSynthesisDriver
 *                            object-- a GNOME Speech driver for the Speechd
 *                            Speech Synthesis System (implementation in
 *                            speechdsynthesisdriver.c)
 *
 */

#ifndef __SPEECHD_SPEAKER_H_
#define __SPEECHD_SPEAKER_H_

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>

#define SPEECHD_SPEAKER_TYPE        (speechd_speaker_get_type ())
#define SPEECHD_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), SPEECHD_SPEAKER_TYPE, SpeechdSpeaker))
#define SPEECHD_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), SPEECHD_SPEAKER_TYPE, SpeechdSpeakerClass))
#define IS_SPEECHDSPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), SPEECHD_SPEAKER_TYPE))
#define IS_SPEECHD_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SPEECHD_SPEAKER_TYPE))

typedef struct {
	Speaker parent;

        int conn;

	gchar *voice;        
} SpeechdSpeaker;

typedef struct {
	SpeakerClass parent_class;
} SpeechdSpeakerClass;

GType
speechd_speaker_get_type   (void);

SpeechdSpeaker *
speechd_speaker_new (GObject *driver,
		      const GNOME_Speech_VoiceInfo *spec);
 
#endif /* __SPEECHD_SPEAKER_H_ */
