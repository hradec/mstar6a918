#!/usr/bin/perl

use warnings;
use strict;
use File::Basename;

use FindBin;

my $config = "$FindBin::Bin/utils.pl";
my $module = @ARGV ? +shift : 'mali';

unless (my $return = do $config)
{
	warn "couldn't parse $config $@" if $@;
	warn "couldn't do $config $!" unless defined $return;
	warn "couldn't run $config" unless $return;
}

my $src = '$(src)/$(MALI_TOP_DIR)$(MALI_VERSION)/linux/src/devicedrv';

while (defined(my $line = <STDIN>))
{
	if (is_comment($line))
	{
		print $line;
		next;
	}

	next if has_ignore_mali_variables($line);

	if ($line =~ m{^\s*include\s+.+?/build_system/project/}s)
	{
		print "include \$(src)/\$(MALI_TOP_DIR)project.mak\n";
		next;
	}

	$line =~ s{\$\(src\)}{$src/$module}g;

	replace_prefix_mali_variables(\$line);
	print $line;
}
