LTLIBRARY_OBJECTS = $(LTLIBRARY_SOURCES:.c=.lo) $(LTLIBRARY_OBJECTS_X)
LTLIBRARY_SHARED_OBJECTS = $(LTLIBRARY_OBJECTS:.lo=.slo)
$(LTLIBRARY_SHARED_NAME): $(LTLIBRARY_SHARED_OBJECTS) $(LTLIBRARY_DEPENDENCIES)
	$(SHARED_LIBTOOL) --mode=link $(COMPILE) $(LDFLAGS) -o $@ -avoid-version -module -rpath $(phplibdir) $(LTLIBRARY_LDFLAGS) $(LTLIBRARY_OBJECTS) $(LTLIBRARY_SHARED_LIBADD)
	$(SHARED_LIBTOOL) --mode=install cp $@ $(phplibdir)

targets = $(LTLIBRARY_SHARED_NAME)
