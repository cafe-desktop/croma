icondir=$(pkgdatadir)/icons
icon_DATA=croma-window-demo.png

AM_CPPFLAGS =					\
	@CROMA_WINDOW_DEMO_CFLAGS@		\
	@CROMA_MESSAGE_CFLAGS@			\
	-DCROMA_ICON_DIR=\"$(pkgdatadir)/icons\" \
	-DCROMA_LOCALEDIR=\"$(prefix)/@DATADIRNAME@/locale\" \
	$(WARN_CFLAGS)

croma_message_SOURCES= 				\
	croma-message.c

croma_window_demo_SOURCES=				\
	croma-window-demo.c

bin_PROGRAMS=croma-message croma-window-demo

croma_message_LDADD= @CROMA_MESSAGE_LIBS@
croma_window_demo_LDADD= @CROMA_WINDOW_DEMO_LIBS@

EXTRA_DIST= \
	meson.build \
	$(icon_DATA)


-include $(top_srcdir)/git.mk
