sub prefix:<^1>(Int $a) {
	return 1..$a;
}

sub MAIN(Int $n) {
	my $N = $n * ($n + 1) / 2;
	my @o;

	@o.push: "$_:$_|#$_" for ^1$n;
	for ^$N X ^$N -> ($i, $j) {
		@o.push: "$i,$j";
	}
	say "{@o}";

	for ^1$n -> $k {
		for 0..($N-$k) X 0..($N-$k) -> ($i, $j) {
			@o = "#$k";
			for ^$k X ^$k -> ($I, $J) {
				@o.push: "{$i+$I},{$j+$J}";
			}
			say "{@o}";
		}
	}
}
