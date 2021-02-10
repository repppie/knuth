#!perl6

# 7.2.2.1-14

sub MAIN($n) {
	my @a;
	for 0..($n - 1) {
		@a.push('S' ~ $_ );
		@a.push('M' ~ $_ );
	}
	@a.print;
	print "\n";
	for 0..($n - 1) -> $i {
		for 0..($n - 1) -> $j {
			if $j != $i % $n and $j != ($i - 1) % $n {
				say "S$i M$j";
			}
		}
	}
}
