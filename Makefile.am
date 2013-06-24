ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = README.md ChangeLog LICENSE autogen.sh m4 .gitignore
SUBDIRS    = src vcproj

if BUILDING_DEB
DEBSOURCEPKG=$(PACKAGE)_$(VERSION).orig.tar.gz
DEBSOURCEDIR=$(PACKAGE)-$(VERSION)
deb: dist
	@$(ECHO) "Building Debian package..."; \
	if $(TEST) ! -d debian; then \
		$(ECHO) "The files needed for building a Debian package are not" \
		"included by default in the distribution. To build a package, check" \
		"out the project from source control."; \
		exit 1; \
	fi; \
	$(MV) $(PACKAGE)-$(VERSION).tar.gz $(DEBSOURCEPKG) && \
	$(TAR) --extract --gunzip --file $(DEBSOURCEPKG) && \
	$(MKDIR_P) $(DEBSOURCEDIR)/debian && \
	$(CP) -r debian/ $(DEBSOURCEDIR) && \
	cd $(DEBSOURCEDIR) && \
	$(DEBUILD) $(DPKG_FLAGS); \
	cd ..;
else
deb:
	@$(ECHO) \
	"To build a Debian package, you must run configure with --enable-deb."; \
	exit 1
endif
CLEANFILES = *.deb *.debian.tar.gz *.dsc *.changes *.build $(DEBSOURCEPKG) $(DEBSOURCEDIR)

