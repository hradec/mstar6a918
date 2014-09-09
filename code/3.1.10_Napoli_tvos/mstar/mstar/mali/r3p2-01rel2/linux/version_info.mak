#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
# (C) COPYRIGHT 2005, 2007-2013 ARM Limited
# ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#

# -*- mode: makefile -*-

# Used as default strings in case of customer delivery (without Subversion), i.e.: MALI_RELEASE_NAME = "Linux-r1p0"
ifndef ARM_INTERNAL_BUILD
ifeq ($(findstring -windowsce-, -$(VARIANT)-),)
MALI_RELEASE_NAME=Linux-r3p2-01rel2
else
MALI_RELEASE_NAME=WindowsCE-r3p0-00dev0
endif
else
MALI_RELEASE_NAME=internal
endif

BUILD_DATE:=$(shell date)

MALI_BUILD_TYPE=$(shell echo $(CONFIG) | tr a-z A-Z)
MALI_ARCHITECTURE="${MALI_ARCH}"
MALI_TRACING="${USE_TRACING}"
MALI_THREADING="${USE_THREADS}"
MALI_TARGET_CORES="${CORES}"
MALI_INITIAL_CPPFLAGS="${INITIAL_CPPFLAGS}"
MALI_CUSTOMER="${CUSTOMER}"
MALI_HOSTLIB="${HOSTLIB}"
MALI_VARIANT="${VARIANT}"
MALI_TOPLEVEL_REPO_URL=$(shell cd $(DRIVER_DIR); (svn info || git svn info || echo 'URL: $(MALI_RELEASE_NAME)') 2>/dev/null | grep '^URL: ' | cut -d: -f2- | cut -b2-)

MALI_GEOMETRY_BACKEND="unknown"
MALI_TARGET_CORE_REVISION="unknown"

# Mali200, Mali400 specifics
ifneq ($(call is-feature-enabled,mali200)$(call is-feature-enabled,mali300)$(call is-feature-enabled,mali400)$(call is-feature-enabled,mali450),)
MALI_GEOMETRY_BACKEND=$(MALI200_GP_BACKEND)
ifneq ($(call is-feature-enabled,mali450),)
MALI_USING_STRING="USING_MALI450=$(USING_MALI450)"
MALI_TARGET_CORE_REVISION="${MALI450_HWVER}"
else
ifneq ($(call is-feature-enabled,mali400),)
MALI_USING_STRING="USING_MALI400=$(USING_MALI400)"
MALI_TARGET_CORE_REVISION="${MALI400_HWVER}"
else
MALI_USING_STRING="USING_MALI200=$(USING_MALI200)"
MALI_TARGET_CORE_REVISION="${MALI200_HWVER}"
endif
endif
endif

TARGET_TOOLCHAIN_VERSION=$(shell ${TARGET_VERSION})
HOST_TOOLCHAIN_VERSION=$(shell ${HOST_VERSION})

## NOTE: The scripts in internal/regression_server/, and the regression server itself require that:
## 1) all the information be present for exactly reproducing the build
## 2) ': BUILD=' begins the version string, which can be obtained by running the 'strings' tool
##
## Should more information be placed in these strings, then the scripts in internal/regression_server may need updating.

VERSION_BUILD_STRINGS = BUILD=${MALI_BUILD_TYPE}
VERSION_BUILD_STRINGS += ARCH=${MALI_ARCHITECTURE}
VERSION_BUILD_STRINGS += PLATFORM=${TARGET_PLATFORM}
VERSION_BUILD_STRINGS += TRACE=${MALI_TRACING}
VERSION_BUILD_STRINGS += THREAD=${MALI_THREADING}
VERSION_BUILD_STRINGS += GEOM=$(MALI_GEOMETRY_BACKEND)
VERSION_BUILD_STRINGS += CORES=${MALI_TARGET_CORES}
VERSION_BUILD_STRINGS += $(MALI_USING_STRING)
VERSION_BUILD_STRINGS += TARGET_CORE_REVISION=${MALI_TARGET_CORE_REVISION}
VERSION_BUILD_STRINGS += TOPLEVEL_REPO_URL=${MALI_TOPLEVEL_REPO_URL}

# Embedded shell-commands, expanded during rule invokation and therefore
# producing per-directory data
VERSION_BUILD_STRINGS += REVISION="`((svnversion | grep -qv exported && echo -n 'Revision: ' && svnversion) || git svn info | sed -e 's/$$$$/M/' | grep '^Revision: ' || echo ${MALI_RELEASE_NAME}) 2>/dev/null | sed -e 's/^Revision: //'`"
VERSION_BUILD_STRINGS += CHANGED_REVISION="`(svn info || git svn info || echo 'Last Changed Rev: $(MALI_RELEASE_NAME)') 2>/dev/null | grep '^Last Changed Rev: ' | cut -d: -f2- | cut -b2-`"
VERSION_BUILD_STRINGS += REPO_URL="`(svn info || git svn info || echo 'URL: $(MALI_RELEASE_NAME)') 2>/dev/null | grep '^URL: ' | cut -d: -f2- | cut -b2-`"
VERSION_BUILD_STRINGS += BUILD_DATE=${BUILD_DATE}
VERSION_BUILD_STRINGS += CHANGE_DATE="`(svn info || git svn info || echo 'Last Changed Date: $(MALI_RELEASE_NAME)') 2>/dev/null | grep '^Last Changed Date: ' | cut -d: -f2- | cut -b2-`"

VERSION_BUILD_STRINGS += TARGET_TOOLCHAIN=${TARGET_TOOLCHAIN}
VERSION_BUILD_STRINGS += HOST_TOOLCHAIN=${HOST_TOOLCHAIN}
VERSION_BUILD_STRINGS += TARGET_TOOLCHAIN_VERSION=${TARGET_TOOLCHAIN_VERSION}
VERSION_BUILD_STRINGS += HOST_TOOLCHAIN_VERSION=${HOST_TOOLCHAIN_VERSION}
VERSION_BUILD_STRINGS += TARGET_SYSTEM=${TARGET_SYSTEM}
VERSION_BUILD_STRINGS += HOST_SYSTEM=${HOST_SYSTEM}
VERSION_BUILD_STRINGS += CPPFLAGS=${MALI_INITIAL_CPPFLAGS}
VERSION_BUILD_STRINGS += CUSTOMER=${MALI_CUSTOMER}
VERSION_BUILD_STRINGS += VARIANT=${MALI_VARIANT}
VERSION_BUILD_STRINGS += HOSTLIB=${MALI_HOSTLIB}
VERSION_BUILD_STRINGS += INSTRUMENTED=${MALI_INSTRUMENTED}
VERSION_BUILD_STRINGS += USING_MRI=${MALI_USING_MRI}
VERSION_BUILD_STRINGS += MALI_TEST_API=${MALI_TEST_API}
VERSION_BUILD_STRINGS += UDD_OS=${UDD_OS}

# MStar: version info for SCM
LIBCODE = MALI
LIBVER  = r3p2-01rel2
LIBCL   = C$Change: 637964 $

SCM_VERSION_INFO = __asm__(\".section .mmodule_version\");
SCM_VERSION_INFO += __asm__(\".string \\\"LIBCODE: ${LIBCODE}\\\\nLIBVER: ${LIBVER}\\\\n${LIBCL}\\\"\");

define make-build-info-file
$(call source-dir-to-binary-dir,$(TARGET_SYSTEM),$1): $(call clean-path,$(filter-out $1,$4));
	$(Q)echo "${SCM_VERSION_INFO} char* __$2_build_info(void) { return \"$2: ${VERSION_BUILD_STRINGS} \";}" > $$@
endef

