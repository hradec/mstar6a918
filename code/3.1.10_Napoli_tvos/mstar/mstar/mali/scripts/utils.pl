#!/usr/bin/perl

use warnings;
use strict;

my @prefix_mali_variables = qw(
	ARCH
	TARGET_PLATFORM
	MSTAR_PLATFORM
	USING_RIU
	RIU_ADDRESS_TYPE
	GPU_CLOCK
	PHYS_TO_BUS_ADDRESS_ADJUST
	GPU_BASE_ADDRESS
	GPU_HW
	NUM_PP
	CONFIG_CHIP_NAME
	CHECK_CONFIG_CHIP_NAME
	CONFIG_BUILDFLAGS
	USING_FIXED_DEVID
	USING_SYNC
	OSKOS
	CHECK_KERNEL_CONFIG
	RESOURCE_MEMORY
	RESOURCE_OS_MEMORY
	RESOURCE_MEM_VALIDATION
	USING_PMU
	EXTRA_DEFINES
	TIMESTAMP
	OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB
	USING_GPU_UTILIZATION
	PROFILING_SKIP_PP_JOBS
	PROFILING_SKIP_PP_AND_GP_JOBS
	SVN_INFO
	SVN_REV
	VERSION_STRINGS
	CONFIG
	DRIVER_DIR
	MSTAR_PLATFORM_FILE
);

my @ignore_mali_variables = qw(
	USING_UMP
	BUILD
	CROSS_COMPILE
);

my $prefix_mali_variables_pattern = qr/@{[ '\b(?:' . join('|', @prefix_mali_variables) . ')\b' ]}/o;
my $ignore_mali_variables_pattern = qr/@{[ '\b(?:' . join('|', @ignore_mali_variables) . ')\b' ]}/o;

sub is_comment
{
	my $line = shift;
	return scalar ($line =~ /^\s*#|^\s*$/);
}

sub replace_prefix_mali_variables
{
	my $line_ref = shift;
	$$line_ref =~ s/$prefix_mali_variables_pattern\s*[?+:]?=/MALI_$&/g;
	$$line_ref =~ s/\$\(($prefix_mali_variables_pattern)/\$(MALI_$1/g;
}

sub has_ignore_mali_variables
{
	my $line= shift;
	return scalar ($line =~ /$ignore_mali_variables_pattern\s*[?+:]?=/);
}
