# Copyright (C) 1996-2023 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

#
#	Makefile for name service switch.
#
subdir	:= nss

include ../Makeconfig

headers := \
  nss.h \
  # headers

# This is the trivial part which goes into libc itself.
routines = \
  $(addsuffix -lookup,$(databases)) \
  compat-lookup \
  digits_dots \
  getnssent \
  getnssent_r \
  nss_action \
  nss_action_parse \
  nss_database \
  nss_fgetent_r \
  nss_files_data \
  nss_files_fopen \
  nss_files_functions \
  nss_hash \
  nss_module \
  nss_parse_line_result \
  nss_readline \
  nsswitch \
  rewrite_field \
  valid_field \
  valid_list_field \
  # routines

# These are the databases that go through nss dispatch.
# Caution: if you add a database here, you must add its real name
# in databases.def, too.
databases = \
  alias \
  ethers \
  grp \
  hosts \
  netgrp \
  network \
  proto \
  pwd \
  service \
  sgrp \
  spwd \
  # databases

ifneq (,$(filter sunrpc,$(subdirs)))
databases		+= key rpc
have-sunrpc		:= 1
else
have-sunrpc		:= 0
endif
CPPFLAGS-getent.c	= -DHAVE_SUNRPC=$(have-sunrpc)

others                  := getent makedb
install-bin             := getent makedb
makedb-modules = xmalloc hash-string
others-extras		= $(makedb-modules)
extra-objs		+= $(makedb-modules:=.o)

tests-static            = tst-field
tests-internal		= tst-field

tests := \
  bug17079 \
  test-digits-dots \
  test-netdb \
  tst-nss-getpwent \
  tst-nss-hash \
  tst-nss-test1 \
  tst-nss-test2 \
  tst-nss-test4 \
  tst-nss-test5 \
  tst-nss-test_errno \
  # tests

xtests = bug-erange

tests-container := \
  tst-nss-compat1 \
  tst-nss-db-endgrent \
  tst-nss-db-endpwent \
  tst-nss-files-hosts-long \
  tst-nss-files-hosts-v4mapped \
  tst-nss-gai-actions \
  tst-nss-gai-hv2-canonname \
  tst-nss-test3 \
  tst-reload1 \
  tst-reload2 \
  # tests-container

# Tests which need libdl
ifeq (yes,$(build-shared))
tests += tst-nss-files-hosts-erange
tests += tst-nss-files-hosts-multi
tests += tst-nss-files-hosts-getent
tests += tst-nss-files-alias-leak
tests += tst-nss-files-alias-truncated
endif

# If we have a thread library then we can test cancellation against
# some routines like getpwuid_r.
ifeq (yes,$(have-thread-library))
tests += tst-cancel-getpwuid_r
endif

# Specify rules for the nss_* modules.  We have some services.
services		:= files db compat

extra-libs		= $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs) ../locale/programs ../intl


routines += \
  $(addprefix files-, $(filter-out key, $(databases))) \
  files-init \
  files-initgroups \
  # routines

# Build only an empty shared libnss_files.
libnss_files-inhibit-o = $(filter-out .os,$(object-suffixes))

# Pretend that libnss_files.so is a linker script, so that the symbolic link
# is not installed.
install-lib-ldscripts = libnss_files.so
$(inst_libdir)/libnss_files.so:

libnss_db-dbs		:= $(addprefix db-,\
				       $(filter-out hosts network key alias,\
						    $(databases))) \
			   db-initgroups
libnss_db-routines	:= $(libnss_db-dbs) db-open db-init hash-string
generated		+= $(filter-out db-alias.c db-netgrp.c, \
					$(addsuffix .c,$(libnss_db-dbs)))

libnss_compat-routines	:= $(addprefix compat-,grp pwd spwd initgroups) \
			   nisdomain

install-others		+= $(inst_vardbdir)/Makefile

# Build static module into libc if requested
libnss_db-inhibit-o	= $(filter-out .os,$(object-suffixes))
libnss_compat-inhibit-o	= $(filter-out .os,$(object-suffixes))
ifeq ($(build-static-nss),yes)
tests-static		+= tst-nss-static
endif
extra-test-objs		+= nss_test1.os nss_test2.os nss_test_errno.os \
			   nss_test_gai_hv2_canonname.os

ifeq ($(run-built-tests),yes)
ifneq (no,$(PERL))
tests-special += $(objpfx)mtrace-tst-nss-gai-hv2-canonname.out
endif
endif

generated += mtrace-tst-nss-gai-hv2-canonname.out \
		tst-nss-gai-hv2-canonname.mtrace

include ../Rules

ifeq (yes,$(have-selinux))
LDLIBS-makedb		:= -lselinux
endif

libnss-libc = $(common-objpfx)linkobj/libc.so
# Target-specific variable setting to link objects using deprecated
# RPC interfaces with the version of libc.so that makes them available
# for new links:
$(services:%=$(objpfx)libnss_%.so): libc-for-link = $(libnss-libc)

$(libnss_db-dbs:%=$(objpfx)%.c): $(objpfx)db-%.c: nss_files/files-%.c
	@rm -f $@.new
	(echo '#define EXTERN_PARSER';\
	 echo '#define GENERIC "../nss_db/db-XXX.c"';\
	 echo '#include "$<"') > $@.new
	mv -f $@.new $@


$(objpfx)makedb: $(makedb-modules:%=$(objpfx)%.o)

$(inst_vardbdir)/Makefile: db-Makefile $(+force)
	$(do-install)

libnss_test1.so-no-z-defs = 1
libnss_test2.so-no-z-defs = 1

rtld-tests-LDFLAGS += -Wl,--dynamic-list=nss_test.ver

libof-nss_test1 = extramodules
libof-nss_test2 = extramodules
libof-nss_test_errno = extramodules
libof-nss_test_gai_hv2_canonname = extramodules
$(objpfx)/libnss_test1.so: $(objpfx)nss_test1.os $(link-libc-deps)
	$(build-module)
$(objpfx)/libnss_test2.so: $(objpfx)nss_test2.os $(link-libc-deps)
	$(build-module)
$(objpfx)/libnss_test_errno.so: $(objpfx)nss_test_errno.os $(link-libc-deps)
	$(build-module)
$(objpfx)/libnss_test_gai_hv2_canonname.so: \
  $(objpfx)nss_test_gai_hv2_canonname.os $(link-libc-deps)
	$(build-module)
$(objpfx)nss_test2.os : nss_test1.c
# Use the nss_files suffix for these objects as well.
$(objpfx)/libnss_test1.so$(libnss_files.so-version): $(objpfx)/libnss_test1.so
	$(make-link)
$(objpfx)/libnss_test2.so$(libnss_files.so-version): $(objpfx)/libnss_test2.so
	$(make-link)
$(objpfx)/libnss_test_errno.so$(libnss_files.so-version): \
  $(objpfx)/libnss_test_errno.so
	$(make-link)
$(objpfx)/libnss_test_gai_hv2_canonname.so$(libnss_files.so-version): \
  $(objpfx)/libnss_test_gai_hv2_canonname.so
	$(make-link)
$(patsubst %,$(objpfx)%.out,$(tests) $(tests-container)) : \
	$(objpfx)/libnss_test1.so$(libnss_files.so-version) \
	$(objpfx)/libnss_test2.so$(libnss_files.so-version) \
	$(objpfx)/libnss_test_errno.so$(libnss_files.so-version) \
	$(objpfx)/libnss_test_gai_hv2_canonname.so$(libnss_files.so-version)

ifeq (yes,$(have-thread-library))
$(objpfx)tst-cancel-getpwuid_r: $(shared-thread-library)
endif

$(objpfx)tst-nss-files-alias-leak.out: $(objpfx)/libnss_files.so
$(objpfx)tst-nss-files-alias-truncated.out: $(objpfx)/libnss_files.so

tst-nss-gai-hv2-canonname-ENV = \
		MALLOC_TRACE=$(objpfx)tst-nss-gai-hv2-canonname.mtrace \
		LD_PRELOAD=$(common-objpfx)/malloc/libc_malloc_debug.so
$(objpfx)mtrace-tst-nss-gai-hv2-canonname.out: \
  $(objpfx)tst-nss-gai-hv2-canonname.out
	{ test -r $(objpfx)tst-nss-gai-hv2-canonname.mtrace \
	|| ( echo "tst-nss-gai-hv2-canonname.mtrace does not exist"; exit 77; ) \
	&& $(common-objpfx)malloc/mtrace \
	$(objpfx)tst-nss-gai-hv2-canonname.mtrace; } > $@; \
	$(evaluate-test)

# Disable DT_RUNPATH on NSS tests so that the glibc internal NSS
# functions can load testing NSS modules via DT_RPATH.
LDFLAGS-tst-nss-test1 = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test2 = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test3 = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test4 = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test5 = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test_errno = -Wl,--disable-new-dtags
LDFLAGS-tst-nss-test_gai_hv2_canonname = -Wl,--disable-new-dtags
