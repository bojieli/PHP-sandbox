
$(builddir)/ifx.c: $(srcdir)/ifx.ec $(builddir)/libphpifx.a
	(if test -d $(INFORMIXDIR); then \
	   THREADLIB=POSIX $(INFORMIXDIR)/bin/esql -e $(IFX_ESQL_FLAGS) $(srcdir)/ifx.ec; mv ifx.c $@; \
	 else \
	   touch $@; \
	 fi)

$(builddir)/libphpifx.a:
	$(LINK) $(IFX_LIBOBJS)
