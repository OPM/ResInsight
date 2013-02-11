## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I/private/joaho/EnKF/devel/EnKF/libecl/include \
	-I/private/joaho/EnKF/devel/EnKF/libconfig/include \
	-I/private/joaho/EnKF/devel/EnKF/libutil/include \
	-I/private/joaho/EnKF/devel/EnKF/librms/include \
	-I/private/joaho/EnKF/devel/EnKF/libsched/include \
	-I/private/joaho/EnKF/devel/EnKF/libjob_queue/include \
	-I/private/joaho/EnKF/devel/EnKF/libanalysis/include \
	-I/private/joaho/EnKF/devel/EnKF/libsample/include \
	-I/private/joaho/EnKF/devel/EnKF/libenkf/include \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = test

test_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	enkf_setup.c enkf_setup.h \
	callbacks.c callbacks.h \
	custom-cell-renderer-progressbar.c custom-cell-renderer-progressbar.h

test_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) \
			 -L/private/joaho/EnKF/devel/EnKF/libenkf/lib -lenkf \
			 -L/private/joaho/EnKF/devel/EnKF/libecl/lib -lecl \
			 -L/private/joaho/EnKF/devel/EnKF/libjob_queue/lib -ljob_queue \
			 -L/private/joaho/EnKF/devel/EnKF/libsched/lib -lsched \
			 -L/private/joaho/EnKF/devel/EnKF/libconfig/lib -lconfig \
			 -L/private/joaho/EnKF/devel/EnKF/libutil/lib -lutil \
			 -L/d/proj/bg/enkf/software/lib \
			 -L/d/proj/bg/enkf/software/include \
			 -L/private/joaho/EnKF/devel/EnKF/librms/lib -lrms -lm -lpthread -lz  -llapack -lblas  -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lglib-2.0
