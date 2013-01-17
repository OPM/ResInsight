include global_config
SDP_ROOT          = $(shell get_sdp_root.py)
#################### PLPLOT LINKING #############################
WITH_TRUETYPE = 1 # Change this (0-1) when compiling with or without truetype.
#################################################################
COMPILE_INCLUDE   = -I$(LIBPLOT_HOME)/src -I$(EXTERNAL_INC_PATH) -I$(LIBUTIL_HOME)/include -I$(LIBECL_HOME)/include 
INSTALL_INC_PATH  = $(LIBPLOT_HOME)/include
INSTALL_LIB_PATH  = $(LIBPLOT_HOME)/lib
#################################################################
ARFLAGS = -csr
#################################################################
OBJECTS        = plot.o plot_dataset.o  plot_range.o plplot_driver.o plot_driver.o text_driver.o 
INC_FILES      = plot.h plot_dataset.h  plot_range.h plot_const.h plot_driver.h  plplot_driver.h text_driver.h
LIB            = libplot.a 
TARGET_INCLUDE = -I$(LIBPLOT_HOME)/include -I$(EXTERNAL_INC_PATH) -I$(LIBUTIL_HOME)/include -I$(LIBECL_HOME)/include



all: $(LIB) 
	install -d $(INSTALL_INC_PATH)
	install -d $(INSTALL_LIB_PATH)
	install $(INC_FILES) $(INSTALL_INC_PATH)
	install $(LIB) $(INSTALL_LIB_PATH)

SDP_INSTALL: all
	install $(LIB) $(SDP_ROOT)/lib
	install $(INC_FILES) $(SDP_ROOT)/include

clean:
	rm -f *.o *~ *.x
	rm -f $(LIB)
	rm -f $(INSTALL_LIB_PATH)/$(LIB)
	rm -f $(INSTALL_INC_PATH)/*.h


distclean:
	rm -f *.o *~ *.x
	rm -f $(LIB)
	rm -f $(INSTALL_LIB_PATH)/$(LIB)
	rm -f $(INSTALL_INC_PATH)/*.h

rebuild: clean all

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECTS)

%.o : %.c
	$(CC) -c $(CFLAGS) -DWITH_TRUETYPE=$(WITH_TRUETYPE)  $(COMPILE_INCLUDE) $< -o $@

%.o : %.cc
	$(CPP) -c $(CPPFLAGS) $(COMPILE_INCLUDE) $< -o $@

%.o : %.cxx
	$(CPP) -c $(CPPFLAGS) $(COMPILE_INCLUDE) $< -o $@

testplot_cxx : testplot_cxx.o $(LIB)
	$(CPP) -o testplot_cxx testplot_cxx.o -L. $(LIB_PATH) $(LIB_LINK)

new: 
	../../Scripts/cdep.py all_include.h 0

include dependencies
