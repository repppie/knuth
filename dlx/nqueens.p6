#!perl6

sub MAIN(Int $n, Bool :$old?) {
	my @a;
	for 1..$n {
		@a.push('r' ~ $_ );
		@a.push('c' ~ $_ );
	}
	@a.push('|') unless $old;
	@a.push('a' ~ $_) for 1..($n * 2);
	@a.push('b' ~ $_) for -($n-1)..($n-1);
	@a.print;
	print "\n";
	for 1..$n X 1..$n -> ($i, $j) {
		say "r$i c$j a{$i+$j} b{$i - $j}";
	}
	if $old {
		say "a$_" for 1..($n * 2);
		say "b$_" for -($n-1)..($n-1);
	}
}
