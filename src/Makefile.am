lib_LTLIBRARIES = libcroma-private.la

SUBDIRS=wm-tester tools themes

AM_CPPFLAGS = \
	-I$(srcdir)/include \
	-DCROMA_LIBEXECDIR=\"$(libexecdir)\" \
	-DHOST_ALIAS=\"@HOST_ALIAS@\" \
	-DCROMA_LOCALEDIR=\""$(datadir)/locale"\" \
	-DCROMA_PKGDATADIR=\"$(pkgdatadir)\" \
	-DCROMA_DATADIR=\"$(datadir)\" \
	-DG_LOG_DOMAIN=\"croma\" \
	-DSN_API_NOT_YET_FROZEN=1 \
	@CROMA_CFLAGS@ \
	$(WARN_CFLAGS)

croma_SOURCES= 				\
	core/main.c				\
	include/main.h

# by setting libcroma_private_la_CFLAGS, the files shared with
# croma proper will be compiled with different names.
libcroma_private_la_CFLAGS =
libcroma_private_la_SOURCES=			\
	core/async-getprop.c \
	core/async-getprop.h \
	core/atomnames.h \
	core/bell.c \
	core/bell.h \
	core/boxes.c \
	include/boxes.h \
	compositor/compositor.c \
	compositor/compositor-private.h \
	compositor/compositor-xrender.c \
	compositor/compositor-xrender.h \
	include/compositor.h \
	core/constraints.c \
	core/constraints.h \
	core/core.c \
	core/delete.c \
	core/display.c \
	core/display-private.h \
	include/display.h \
	ui/draw-workspace.c \
	ui/draw-workspace.h \
	core/edge-resistance.c \
	core/edge-resistance.h \
	core/effects.c \
	core/effects.h \
	core/errors.c \
	include/errors.h \
	core/eventqueue.c \
	core/eventqueue.h \
	core/frame.c \
	core/frame-private.h \
	include/frame.h \
	ui/gradient.c \
	ui/gradient.h \
	core/group-private.h \
	core/group-props.c \
	core/group-props.h \
	core/group.c \
	core/group.h \
	core/iconcache.c \
	core/iconcache.h \
	core/keybindings.c \
	core/keybindings.h \
	core/main.c \
	include/main.h \
	core/croma-Xatomtype.h \
	core/place.c \
	core/place.h \
	core/prefs.c \
	include/prefs.h \
	core/screen.c \
	core/screen-private.h \
	include/screen.h \
	include/types.h \
	core/session.c \
	core/session.h \
	core/stack.c \
	core/stack.h \
	core/util.c \
	include/util.h \
	core/window-props.c \
	core/window-props.h \
	core/window.c \
	core/window-private.h \
	include/window.h \
	core/workspace.c \
	core/workspace.h \
	core/xprops.c \
	include/xprops.h \
	include/common.h \
	include/core.h \
	include/ui.h \
	ui/fixedtip.c \
	ui/fixedtip.h \
	ui/frames.c \
	ui/frames.h \
	ui/menu.c \
	ui/menu.h \
	ui/metaaccellabel.c \
	ui/metaaccellabel.h \
	ui/preview-widget.c \
	ui/preview-widget.h \
	ui/resizepopup.c \
	include/resizepopup.h \
	ui/tabpopup.c \
	include/tabpopup.h \
	ui/tile-preview.c \
	include/tile-preview.h \
	ui/theme-parser.c \
	ui/theme-parser.h \
	ui/theme.c \
	ui/theme.h \
	ui/ui.c \
	include/all-keybindings.h

libcroma_private_la_LDFLAGS = -no-undefined -version-info $(LT_VERSION_INFO)
libcroma_private_la_LIBADD = @CROMA_LIBS@

libcromaincludedir = $(includedir)/croma-1/croma-private

libcromainclude_HEADERS = \
	include/boxes.h \
	ui/gradient.h \
	include/util.h \
	include/common.h \
	ui/preview-widget.h \
	ui/theme-parser.h \
	ui/theme.h

croma_theme_viewer_SOURCES= \
	ui/theme-viewer.c

bin_PROGRAMS=croma croma-theme-viewer

croma_LDADD=@CROMA_LIBS@ libcroma-private.la
croma_theme_viewer_LDADD= @CROMA_LIBS@ libcroma-private.la

testboxes_SOURCES=include/util.h core/util.c include/boxes.h core/boxes.c core/testboxes.c
testgradient_SOURCES=ui/gradient.h ui/gradient.c ui/testgradient.c
testasyncgetprop_SOURCES=core/async-getprop.h core/async-getprop.c core/testasyncgetprop.c

noinst_PROGRAMS=testboxes testgradient testasyncgetprop

testboxes_LDADD= @CROMA_LIBS@
testgradient_LDADD= @CROMA_LIBS@
testasyncgetprop_LDADD= @CROMA_LIBS@

%.desktop: %.desktop.in
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

desktopfilesdir=$(datadir)/applications
desktopfiles_in_files=croma.desktop.in
desktopfiles_files=$(desktopfiles_in_files:.desktop.in=.desktop)
desktopfiles_DATA = $(desktopfiles_files)

wmpropertiesdir=$(datadir)/cafe/wm-properties
wmproperties_in_files=croma-wm.desktop.in
wmproperties_files=$(wmproperties_in_files:.desktop.in=.desktop)
wmproperties_DATA = $(wmproperties_files)

gsettings_SCHEMAS = org.cafe.croma.gschema.xml
@GSETTINGS_RULES@

xmldir = @MATE_KEYBINDINGS_KEYSDIR@
xml_in_files = \
	50-croma-desktop-key.xml.in \
	50-croma-window-key.xml.in \
	50-croma-global-key.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

%.xml: %.xml.in
	$(AM_V_GEN) GETTEXTDATADIR=$(top_srcdir) $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

BUILT_SOURCES =
CLEANFILES = \
	croma.desktop \
	croma-wm.desktop \
	50-croma-desktop-key.xml \
	50-croma-global-key.xml \
	50-croma-window-key.xml

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = libcroma-private.pc

EXTRA_DIST= \
	$(desktopfiles_files) \
	$(wmproperties_files) \
	$(IMAGES) \
	$(gsettings_SCHEMAS) \
	$(desktopfiles_in_files) \
	$(wmproperties_in_files) \
	$(xml_in_files) \
	libcroma-private.pc.in \
	meson.build


-include $(top_srcdir)/git.mk
