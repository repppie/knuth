my %n;

sub var($v) {
	my $vv = $v;
	return Nil, False if $v == 0;
	my $neg = $v.substr(0, 1) eq '-';
	$vv = $vv.substr(1) if $neg;
	return Nil, $neg if %n{$vv}.substr(0, 1) eq '#';
	$vv = %n{$vv};
	return $vv, $neg;
}

sub MAIN($f) {
	for $f.IO.lines {
		m/(\S+)\s(\S+)$/;
		%n{~$0} = ~$1;
	}
	my $in-v;
	my @o;
	for $*IN.lines {
		when m/^'v'/ {
			$in-v = True;
			for .words[1..*] -> $w {
				my ($v, $neg) = var($w) with $w;
				if defined $v and not $neg {
					@o.push: "{$neg ?? '~' !! ''}$v";
				}
			}
		}
		if $in-v {
			$in-v = False;
			say ~@o.sort if @o.elems;
			say '';
			@o = Empty;
		}
		.say;
	}
}
