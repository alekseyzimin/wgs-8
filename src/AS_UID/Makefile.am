noinst_LIBRARIES += %D%/libAS_UID.a
%C%_libAS_UID_a_SOURCES = %D%/uidclient.C

if SITE_JCVI
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_JTC.C
#  SITEOBJS        = SYS_UIDclient_JTC.o
endif

if SITE_LOCAL
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_LOCAL.C
#  SITEOBJS       += SYS_UIDclient_LOCAL.o
endif

if SITE_BRI
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_SERVER.C %D%/uidserver.C
#  SITEOBJS       += SYS_UIDclient_SERVER.o
#  CXX_PROGS      += uidserver
endif

libCA_a_SOURCES += $(%C%_libAS_UID_a_SOURCES)
