include global_config
SDP_ROOT	= $(shell get_sdp_root.py)
#################################################################
COMPILE_INCLUDE   = -I$(LIBRMS_HOME)/src -I$(LIBECL_HOME)/include -I$(LIBUTIL_HOME)/include 
INSTALL_INC_PATH  = $(LIBRMS_HOME)/include
INSTALL_LIB_PATH  = $(LIBRMS_HOME)/lib
#################################################################
OBJECTS       = rms_file.o rms_util.o rms_tag.o rms_type.o rms_tagkey.o rms_stats.o rms_export.o
INC_FILES     = rms_file.h rms_util.h rms_tag.h rms_type.h rms_tagkey.h rms_stats.h rms_export.h
LIB	      = librms.a


LOCAL: LIB 
	install -d $(INSTALL_INC_PATH)
	install -d $(INSTALL_LIB_PATH)
	install $(INC_FILES) $(INSTALL_INC_PATH)
	install $(LIB) $(INSTALL_LIB_PATH)

SDP_INSTALL: LIB BIN
	install $(LIB) $(SDP_ROOT)/lib/lib$(LIB_ROOT).a
	install $(INC_FILES) $(SDP_ROOT)/include

LIB:	$(LIB)

clean:
	rm -f *.o *~ 
	rm -f $(LIB)
	rm -f $(INSTALL_LIB_PATH)/$(LIB)
	rm -f $(INSTALL_INC_PATH)/*.h

rebuild: clean LOCAL

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECTS)


%.o : %.c
	$(CC) -c $(CFLAGS) $(COMPILE_INCLUDE) $(CPPFLAGS) $< -o $@


new: 
	../../Scripts/cdep.py all_include.h 0

include dependencies

##################################################################
## Binaries
#
#BIN_FILES             = rms.x rms_stat.x roff_tags.x rms_extract.x rms_setname.x
#BIN:	                $(BIN_FILES)
#BIN_FILES_SDP_INSTALL = roff_tags.x
#
#rms_test.o    : rms_test.c
#rms_stat.o    : rms_stat.c
#tag_list.o    : tag_list.c
#rms_extract.o : rms_extract.c
#rms_setname.o : rms_setname.c
#
#roff_tags.x:	tag_list.o $(LIB)
#	$(CC) -$(MFLAG) $(LDFLAGS) tag_list.o -o roff_tags.x $(LIB_PATH)  $(LIB_LINK) 	
#
#rms.x:	rms_test.o $(LIB)
#	$(CC) -$(MFLAG) $(LDFLAGS) rms_test.o -o rms.x $(LIB_PATH)  $(LIB_LINK) 	
#
#rms_stat.x: rms_stat.o $(LIB)
#	$(CC) -$(MFLAG) $(LDFLAGS) rms_stat.o -o rms_stat.x $(LIB_PATH)  $(LIB_LINK) 	
#
#rms_extract.x: rms_extract.o $(LIB)
#	$(CC) -$(MFLAG) $(LDFLAGS) rms_extract.o -o rms_extract.x $(LIB_PATH)  $(LIB_LINK) 	
#
#rms_setname.x: rms_setname.o $(LIB)
#	$(CC) -$(MFLAG) $(LDFLAGS) rms_setname.o -o rms_setname.x $(LIB_PATH)  $(LIB_LINK) 	


