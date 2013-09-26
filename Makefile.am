ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = README.md ChangeLog LICENSE autogen.sh m4 .gitignore packages
SUBDIRS    = src vcproj

if BUILDING_DEB
DEBSOURCEPKG=$(PACKAGE)_$(VERSION).orig.tar.gz
DEBSOURCEDIR=$(PACKAGE)-$(VERSION)
DEBIANDIR=packages/debian
deb: dist
	@$(ECHO) "Building Debian package..."; \
	if $(TEST) ! -d $(DEBIANDIR); then \
		$(ECHO) "The files needed for building a Debian package are not" \
		"included by default in the distribution. To build a package, check" \
		"out the project from source control."; \
		exit 1; \
	fi; \
	$(MV) $(PACKAGE)-$(VERSION).tar.gz $(DEBSOURCEPKG) && \
	$(TAR) --extract --gunzip --file $(DEBSOURCEPKG) && \
	$(MKDIR_P) $(DEBSOURCEDIR)/debian && \
	$(CP) -r $(DEBIANDIR) $(DEBSOURCEDIR) && \
	cd $(DEBSOURCEDIR) && \
	$(DEBUILD) $(DPKG_FLAGS); \
	cd ..;
else
deb:
	@$(ECHO) \
	"To build a Debian package, you must run configure with --enable-deb."; \
	exit 1
endif
clean-local:
	-rm -rf $(DEBSOURCEDIR)

CLEANFILES = *.deb *.debian.tar.gz *.dsc *.changes *.build *.upload $(DEBSOURCEPKG) 

