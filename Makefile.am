SUBDIRS = src


dist_pkglibexec_SCRIPTS = \
	eos-codecs-activate \
	eos-gstreamer-codecs-update \
	$(NULL)

# Unit services for systemd
systemdunit_in_files = \
	eos-codecs-activate.service.in \
	eos-gstreamer-codecs-update.service.in

do_subst = sed \
        -e 's|@pkglibexecdir[@]|$(pkglibexecdir)|g'

%.service : %.service.in
	$(AM_V_GEN) $(do_subst) $< > $@

systemdunit_DATA = $(systemdunit_in_files:.service.in=.service)

# Set systemd unit directory that follows $prefix too for distcheck.
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemdunitdir='$${libdir}/systemd/system' \
	$(NULL)

# Integration with NetworkManager for downloader scripts
networkscriptdir = $(sysconfdir)/network/if-up.d
dist_networkscript_DATA = eos-gstreamer-codecs-update-nm
install-data-hook:
	chmod +x $(DESTDIR)$(sysconfdir)/network/if-up.d/eos-gstreamer-codecs-update-nm

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
