icondir=$(pkgdatadir)/icons
icon_DATA=croma-window-demo.png

AM_CPPFLAGS =					\
	@MARCO_WINDOW_DEMO_CFLAGS@		\
	@MARCO_MESSAGE_CFLAGS@			\
	-DMARCO_ICON_DIR=\"$(pkgdatadir)/icons\" \
	-DMARCO_LOCALEDIR=\"$(prefix)/@DATADIRNAME@/locale\" \
	$(WARN_CFLAGS)

croma_message_SOURCES= 				\
	croma-message.c

croma_window_demo_SOURCES=				\
	croma-window-demo.c

bin_PROGRAMS=croma-message croma-window-demo

croma_message_LDADD= @MARCO_MESSAGE_LIBS@
croma_window_demo_LDADD= @MARCO_WINDOW_DEMO_LIBS@

EXTRA_DIST= \
	meson.build \
	$(icon_DATA)


-include $(top_srcdir)/git.mk
