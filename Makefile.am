SUBDIRS = src

dist_bin_SCRIPTS = eos-relocate-codecs

# We install the updated in the pkglibexec dir, since that's
# where other updaters (e.g. chromium plugins) are installed.
dist_pkglibexec_SCRIPTS = eos-gstreamer-codecs-update

# Unit services for systemd
systemdunit_in_files = \
	eos-relocate-codecs.service.in \
	eos-gstreamer-codecs-update.service.in

do_subst = sed \
        -e 's|@bindir[@]|$(bindir)|g' \
        -e 's|@pkglibexecdir[@]|$(pkglibexecdir)|g'

%.service : %.service.in
	$(AM_V_GEN) $(do_subst) $< > $@

systemdunit_DATA = $(systemdunit_in_files:.service.in=.service)

# Set systemd unit directory that follows $prefix too for distcheck.
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemdunitdir='$${libdir}/systemd/system' \
	$(NULL)

# Integration with NetworkManager for downloader scripts
networkscriptdir = $(sysconfdir)/NetworkManager/dispatcher.d
dist_networkscript_DATA = 50-eos-gstreamer-codecs-update-nm
install-data-hook:
	chmod +x $(DESTDIR)$(sysconfdir)/NetworkManager/dispatcher.d/50-eos-gstreamer-codecs-update-nm

EXTRA_DIST = \
	$(systemdunit_in_files) \
	debian \
	$(NULL)

CLEANFILES = \
	$(systemdunit_DATA) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL)

-include $(top_srcdir)/git.mk
