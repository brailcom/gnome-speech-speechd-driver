info_TEXINFOS = gnome-speech-speechd-driver.texi

SUFFIXES = .html
html_docs = gnome-speech-speechd-driver.html
HTMLDIR = htmldoc

.texi.html:
	$(MAKEINFO) --html -o $(HTMLDIR) $(info_TEXINFOS) $<

.PHONY: html
html: $(html_docs)

bin_PROGRAMS = speechd-synthesis-driver
serverinfodir = $(libdir)/bonobo/servers
serverinfo_DATA = GNOME_Speech_SynthesisDriver_Speech_Dispatcher.server

INCLUDES = -I$(top_srcdir) \
	$(gnome_speech_libs_CFLAGS) \
	$(WARN_CFLAGS)

LDADD = $(gnome_speech_libs_LIBS)

speechd_synthesis_driver_SOURCES = \
	libspeechd.c \
	libspeechd.h \
	speechdsynthesisdriver.c \
	speechdsynthesisdriver.h \
	speechdspeaker.c \
	speechdspeaker.h

EXTRA_DIST = GNOME_Speech_SynthesisDriver_Speech_Dispatcher.server.in $(serverinfo_DATA) version.sh build.sh BUGS
CLEANFILES = $(serverinfo_DATA)