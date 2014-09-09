#!/usr/bin/perl

use warnings;
use strict;

use FindBin;

my $config = "$FindBin::Bin/utils.pl";

unless (my $return = do $config)
{
	warn "couldn't parse $config $@" if $@;
	warn "couldn't do $config $!" unless defined $return;
	warn "couldn't run $config" unless $return;
}

while (defined(my $line = <STDIN>))
{
	if (is_comment($line))
	{
		print $line;
		next;
	}

	next if has_ignore_mali_variables($line);

	if ($line =~ m{^\s*include\s+.+?/build_system/platform/}s)
	{
		print "include \$(src)/\$(MALI_TOP_DIR)platform.mak\n";
		next;
	}

	replace_prefix_mali_variables(\$line);
	print $line;
}
