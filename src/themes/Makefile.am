THEMES= \
	Atlanta_ctk \
	ClearlooksRe_ctk \
	Dopple_ctk \
	Dopple-Left_ctk \
	DustBlue_ctk \
	Esco_ctk \
	eOS_ctk \
	Gorilla_ctk \
	Motif_ctk \
	Raleigh_ctk \
	Spidey_ctk \
	Spidey-Left_ctk \
	Splint_ctk \
	Splint_ctk-Left_ctk \
	WinMe_ctk

THEME_DIR=$(datadir)/themes
THEME_SUBDIR=metacity-1

EXTRA_DIST= \
	meson.build \
	Atlanta_ctk/meson.build \
	ClearlooksRe_ctk/meson.build \
	Dopple_ctk/meson.build \
	Dopple-Left_ctk/meson.build \
	DustBlue_ctk/meson.build \
	eOS_ctk/meson.build \
	Esco_ctk/meson.build \
	Gorilla_ctk/meson.build \
	Motif_ctk/meson.build \
	Raleigh_ctk/meson.build \
	Spidey_ctk/meson.build \
	Spidey-Left_ctk/meson.build \
	Splint_ctk/meson.build \
	Splint_ctk-Left_ctk/meson.build \
	WinMe_ctk/meson.build

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(THEME_DIR); \
	for THEME in $(THEMES); do \
		echo '-- Installing theme '$$THEME; \
		$(mkinstalldirs) $(DESTDIR)$(THEME_DIR)/$$THEME; \
		$(mkinstalldirs) $(DESTDIR)$(THEME_DIR)/$$THEME/$(THEME_SUBDIR); \
		(installfiles=`find $(srcdir)/$$THEME -name "*.png" -o -name "*.xml"`; \
		 for i in $$installfiles; do \
			echo '-- Installing '$$i ; \
		    	$(INSTALL_DATA) $$i $(DESTDIR)$(THEME_DIR)/$$THEME/$(THEME_SUBDIR) ; \
	  	 done) \
	done

uninstall-local:
	for THEME in $(THEMES); do \
		echo '-- Uninstalling theme '$$THEME; \
		(uninstallfiles=`find $(srcdir)/$$THEME -name "*.png" -o -name "*.xml"`; \
		 for i in $$uninstallfiles; do \
					i=`basename $$i`; \
			echo '-- Removing '$$i ; \
				rm -f $(DESTDIR)$(THEME_DIR)/$$THEME/$(THEME_SUBDIR)/$$i ; \
		done); \
		rmdir $(DESTDIR)$(THEME_DIR)/$$THEME/$(THEME_SUBDIR) || :; \
		rmdir $(DESTDIR)$(THEME_DIR)/$$THEME || :; \
	done
	-rmdir $(DESTDIR)$(THEME_DIR)

dist-hook:
	mkdir $(distdir)/themes; \
	for THEME in $(THEMES); do \
		echo '-- Disting theme '$$THEME; \
		mkdir $(distdir)/$$THEME; \
		(installfiles=`find $(srcdir)/$$THEME -name "*.png" -o -name "*.xml"`; \
		 for i in $$installfiles; do \
			echo '-- Disting '$$i ; \
				cp $$i $(distdir)/$$THEME; \
		done) \
	done

-include $(top_srcdir)/git.mk
