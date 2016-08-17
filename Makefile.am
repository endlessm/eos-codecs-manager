SUBDIRS = src data

dist_pkglibexec_SCRIPTS = \
	eos-codecs-activate \
	eos-codecs-install-key \
	eos-gstreamer-codecs-update \
	$(NULL)

# Integration with NetworkManager for downloader scripts
networkscriptdir = $(sysconfdir)/network/if-up.d
dist_networkscript_DATA = eos-gstreamer-codecs-update-nm
install-data-hook:
	chmod +x $(DESTDIR)$(sysconfdir)/network/if-up.d/eos-gstreamer-codecs-update-nm

EXTRA_DIST = \
	debian \
	$(NULL)

CLEANFILES = \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL)

-include $(top_srcdir)/git.mk
