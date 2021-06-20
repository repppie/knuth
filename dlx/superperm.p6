# Find superpermutation on n of length l
# (The Haruhi problem: Find smallest l)

# Generate all permutations in lexicographic order
sub gen-permutations($n) {
	my @o;
	my $a = (1..$n).Array;
	loop {
		@o.push: $a.clone;
		my $j = $n - 2;
		if $a[$j] >= $a[$j+1] {
			$j-- until $j < 0 || $a[$j] < $a[$j+1];
		}
		last if $j == -1;
		my $l = $n - 1;
		if $a[$j] >= $a[$l] {
			$l-- until $a[$j] < $a[$l];
		}
		($a[$j], $a[$l]) = ($a[$l], $a[$j]);
		my $k = $j + 1;
		$l = $n - 1;
		while $k < $l {
			($a[$k], $a[$l]) = ($a[$l], $a[$k]);
			$k++;
			$l--;
		}
	}
	return @o;
}

sub MAIN($n, $l) {
	my @o;
	my @p = gen-permutations($n);
	@o.push: "$_" for ^@p.elems;
	@o.push: '|';
	@o.push: "x$_" for ^$l;
	say ~@o;

	for ^@p.elems X ^($l-$n+1) -> ($pp, $ll) {
		my @o;
		@o.push: $pp;
		@o.push: "x{$ll+$_}:{@p[$pp][$_]}" for ^$n;
		say ~@o;
	}
}
