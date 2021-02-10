#!perl6

my $s =
    '......3..' ~
    '1..4.....' ~
    '......1.5' ~
    '9........' ~
    '.....26..' ~
    '....53...' ~
    '.5.8.....' ~
    '...9...7.' ~
    '.83....4.';

sub in-row(@a, $i, $k) {
	return True if @a[$i * 9 + $_] == $k for 0..8;
	return False;
}

sub in-col(@a, $i, $k) {
	return True if @a[$i + $_ * 9] == $k for 0..8;
	return False;
}

sub in-box(@a, $b, $k) {
	for (0..2) X (0..2) -> ($i, $j) {
		return True if @a[floor($b / 3) * 9 * 3 +
		    ($b % 3) * 3 + $i * 9 + $j] == $k;
	}
	return False;
}

sub MAIN() {
	my (@a, @s);
	@s = $s.comb.map({ if $_ eq '.' { 0 } else { .Int } });
	for (0..8) X (0..8) -> ($i, $j) {
		@a.push('p' ~ $i ~ $j) unless @s[$i * 9 + $j] != 0;
		@a.push('r' ~ $i ~ $j+1) unless in-row(@s, $i, $j + 1);
		@a.push('c' ~ $i ~ $j+1) unless in-col(@s, $i, $j + 1);
		@a.push('b' ~ $i ~ $j+1) unless in-box(@s, $i, $j + 1);
	}
	@a.print;
	print "\n";

	for (0..8) X (0..8) X (1..9) -> ($i, $j, $k) {
		my $x = 3 * floor($i / 3) + floor($j / 3);

		say "p$i$j r$i$k c$j$k b$x$k" unless
		    @s[$i * 9 + $j] != 0 or
		    in-row(@s, $i, $k) or in-col(@s, $j, $k) or
		    in-box(@s, $x, $k);
	}
}
