# Makefile to generate build tools
#
# Standard usage:
#        make -f build.mk
#
# To prepare a self-contained distribution:
#        make -f build.mk dist
#
#
# Written by Sascha Schumann
#
# $Id$ 


SUBDIRS = libzend TSRM

STAMP = buildmk.stamp

ALWAYS = generated_lists

.PHONY: $(ALWAYS)

all: $(STAMP) $(ALWAYS)
	@$(MAKE) -s -f build2.mk

generated_lists:
	@echo makefile_am_files = Makefile.am libzend/Makefile.am \
		TSRM/Makefile.am `find ext sapi regex pear -name Makefile.am` > $@
	@echo config_h_files = libzend/acconfig.h TSRM/acconfig.h \
		ext/*/config.h.stub sapi/*/config.h.stub >> $@
	@echo config_m4_files = libzend/libzend.m4 TSRM/tsrm.m4 \
		libzend/acinclude.m4 ext/*/config.m4 sapi/*/config.m4 >> $@

$(STAMP): buildcheck.sh
	@./buildcheck.sh && touch $(STAMP)

dist:
	@rm -f $(SUBDIRS) 2>/dev/null || true
	@for i in $(SUBDIRS); do \
		test -d $$i || (test -d ../$$i && cp -rp ../$$i $$i); \
	done
	@find . -type l -exec rm {} \;
	$(MAKE) AMFLAGS=--copy -f build.mk

cvsclean:
	@for i in `find . -follow -name .cvsignore`; do \
		(cd `dirname $$i` && rm -rf `cat .cvsignore`); \
	done
	@rm -f $(SUBDIRS) 2>/dev/null || true
