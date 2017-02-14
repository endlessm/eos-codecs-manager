do_subst = sed \
        -e 's|@pkglibexecdir[@]|$(pkglibexecdir)|g'

# Unit services for systemd
systemdunit_in_files = \
	eos-codecs-activate.service.in

%.service : %.service.in
	$(AM_V_GEN) $(do_subst) $< > $@

systemdunit_DATA = $(systemdunit_in_files:.service.in=.service)

# Set systemd unit directory that follows $prefix too for distcheck.
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemdunitdir='$${libdir}/systemd/system' \
	$(NULL)

# Desktop file for the key installer
desktop_in_files = eos-codecs-install-key.desktop.in

%.desktop : %.desktop.in
	$(AM_V_GEN) $(do_subst) $< > $@

desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
desktopdir = $(datadir)/applications/

EXTRA_DIST = \
	$(desktop_in_files) \
	$(systemdunit_in_files) \
	$(NULL)

CLEANFILES = \
	$(desktop_DATA) \
	$(systemdunit_DATA) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN)

-include $(top_srcdir)/git.mk
