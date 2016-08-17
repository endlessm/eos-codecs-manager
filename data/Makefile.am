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

EXTRA_DIST = \
	$(systemdunit_in_files) \
	$(NULL)

CLEANFILES = \
	$(systemdunit_DATA) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN)

-include $(top_srcdir)/git.mk
