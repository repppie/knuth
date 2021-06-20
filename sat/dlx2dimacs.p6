sub MAIN($f?) {
	my $fd;
	with $f {
		$fd = $f.IO;
	} else {
		$fd = $*IN;
	}
	my $l = $fd.lines>>.words;
	my (%items, %options);
	my $nitems = 1;
	for $l[0] {
		last if $_ ~~ '|';
		%items{$_} = $nitems++;
	}
	for 1..^$l.elems -> $op {
		%options{$_}.push: $op for $l[$op];
	}

	my @o;
	my %int;
	for %options.kv -> $it, $op {
		@o.push: "$op 0";
		for 0..^$op.elems -> $i {
			for 0..^$i -> $j {
				%int{"-{$op[$i]} -{$op[$j]}"} = 1;
			}
		}
	}
	@o.push("$_ 0") for %int.keys;
	say "p cnf {$l.elems-1} {@o.elems}";
	.say for @o;
}
