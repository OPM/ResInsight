include global_config
all:
	$(MAKE) -s -C $(LIBUTIL_HOME)/src  rebuild
	$(MAKE) -s -C $(LIBPLOT_HOME)/src  rebuild	
	$(MAKE) -s -C $(LIBCONFIG_HOME)/src  rebuild
	$(MAKE) -s -C $(LIBECL_HOME)/src  rebuild
	$(MAKE) -s -C $(LIBRMS_HOME)/src  rebuild
	$(MAKE) -s -C $(LIBSCHED_HOME)/src  rebuild	
	$(MAKE) -s -C $(LIBJOB_QUEUE_HOME)/src  rebuild	
	$(MAKE) -s -C $(LIBENKF_HOME)/src  rebuild	

SDP:	all
	$(MAKE) -s -C $(LIBUTIL_HOME)/src  SDP_INSTALL
	$(MAKE) -s -C $(LIBCONFIG_HOME)/src  SDP_INSTALL
	$(MAKE) -s -C $(LIBECL_HOME)/src  SDP_INSTALL
	$(MAKE) -s -C $(LIBRMS_HOME)/src  SDP_INSTALL
	$(MAKE) -s -C $(LIBPLOT_HOME)/src  SDP_INSTALL	
	$(MAKE) -s -C $(LIBSCHED_HOME)/src  SDP_INSTALL	
	$(MAKE) -s -C $(LIBJOB_QUEUE_HOME)/src  SDP_INSTALL	
	$(MAKE) -s -C $(LIBENKF_HOME)/src  SDP_INSTALL	

