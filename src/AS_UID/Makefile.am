noinst_LIBRARIES += %D%/libAS_UID.a
%C%_libAS_UID_a_SOURCES = %D%/uidclient.C

bin_PROGRAMS += bin/uidclient
bin_uidclient_SOURCES = %D%/uidclient.C

if SITE_JCVI
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_JTC.C
endif

if SITE_LOCAL
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_LOCAL.C
endif

if SITE_BRI
  %C%_libAS_UID_a_SOURCES += %D%/SYS_UIDclient_SERVER.C %D%/uidserver.C
  bin_PROGRAMS += bin/uidserver
  bin_uidserver_SOURCES = %D%/uidserver.C
endif

libCA_a_SOURCES += $(%C%_libAS_UID_a_SOURCES)
