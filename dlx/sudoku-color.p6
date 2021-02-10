#!perl6

# 7.2.2.1-58

sub f($j, $p, $q) {
	return ($j == $p ?? 8 !! $j == $q ?? 9 !! 1 + $j - ($j > $p) -
	    ($j > $q));
}

sub MAIN(Int $n) {
	my @a;
	for (0..8) X (0..8) -> ($i, $j) {
		@a.push('p' ~ $i ~ $j);
		@a.push('r' ~ $i ~ $j+1);
		@a.push('c' ~ $i ~ $j+1);
		@a.push('b' ~ $i ~ $j+1);
	}
	@a.push('*');
	@a.push('|');
	for 1..$n -> $i {
		@a.push('a' ~ $i ~ $_) for 0..18;
		@a.push('d' ~ $i ~ $_) for -8..8;
	}
	@a.push('*' ~ $_) for 0..8;
	@a.print;
	print "\n";

	for (0..8) X (0..8) -> ($p, $q) {
		my @o;
	        next when $q <= $p or $p + $q >= 9;
	        @o.push("*");
                @o.push("*$_:{f($_, $p, $q)}") for 0..8;
                say @o.Str;
	}

	for (0..8) X (0..8) X (1..9) -> ($i, $j, $k) {
		my $x = 3 * floor($i / 3) + floor($j / 3);

		print "p$i$j r$i$k c$j$k b$x$k";
		print " *$j:$k" if $i == 0;
		if $k <= $n  {
			say " a$k" ~ $i + $j ~ " d$k" ~ $i - $j;
		} else {
			print "\n";
		}
	}
}
