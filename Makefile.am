SUBDIRS = src data

dist_pkglibexec_SCRIPTS = \
	eos-codecs-activate \
	eos-codecs-install-key \
	$(NULL)

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
