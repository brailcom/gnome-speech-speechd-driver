bin_PROGRAMS = speechd-synthesis-driver
serverinfodir = $(libdir)/bonobo/servers
serverinfo_DATA = GNOME_Speech_SynthesisDriver_Speech_Dispatcher.server

INCLUDES = -I$(top_srcdir) \
	$(gnome_speechd_CFLAGS) \
	$(WARN_CFLAGS)

LDADD = $(gnome_speechd_LIBS)

speechd_synthesis_driver_SOURCES = \
	libspeechd.c \
	libspeechd.h \
	speechdsynthesisdriver.c \
	speechdsynthesisdriver.h \
	speechdspeaker.c \
	speechdspeaker.h

EXTRA_DIST = GNOME_Speech_SynthesisDriver_Speech_Dispatcher.server.in $(serverinfo_DATA)
CLEANFILES = $(serverinfo_DATA)
