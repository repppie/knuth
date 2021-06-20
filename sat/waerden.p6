sub MAIN($j, $k, $n) {
	my @oo;
	for 1..$n -> $d {
		for 1..$n -> $i {
			last if $i + $d * ($j-1) > $n;
			my @o;
			@o.push: $i+$_*$d for 0..($j-1);
			@oo.push: "{@o} 0";
		}
		for 1..$n -> $i {
			last if $i + $d * ($k-1) > $n;
			my @o;
			@o.push: -($i+$_*$d) for 0..($k-1);
			@oo.push: "{@o} 0";
		}
	}
	say "p cnf $n {@oo.elems}";
	.say for @oo;
}
