#!perl6

sub MAIN($n) {
	(1..$n).print;
	print(' ');
	my @a;
	for 1..($n * 2) {
		@a.push('s' ~ $_);
	}
	@a.print;
	print "\n";
	for 1..$n -> $i {
		for 1..($n * 2) -> $j {
			if $i + $j + 1 <= 2 * $n and not ($i == $n - ($n % 2 ??
			    0 !! 1) and $j > $n / 2) {
				say "$i s$j s{$i + $j + 1}";
			}
		}
	}
}
