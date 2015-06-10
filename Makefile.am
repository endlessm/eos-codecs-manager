dist_bin_SCRIPTS = eos-relocate-codecs

systemdunit_in_file = eos-relocate-codecs.service.in
systemdunit_DATA = $(systemdunit_in_file:.service.in=.service)

$(systemdunit_DATA): $(systemdunit_in_file)
	$(AM_V_GEN)sed -e "s|\@bindir\@|$(bindir)|" $< > $@

# Set systemd unit directory that follows $prefix too for distcheck.
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemdunitdir='$${libdir}/systemd/system' \
	$(NULL)

EXTRA_DIST = \
	$(systemdunit_in_file) \
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
