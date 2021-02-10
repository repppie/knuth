# For 7.2.2-75
sub MAIN(Int $n) {
	my (@t, @v);

	
	for ^$n X ^$n -> ($i,$j) {
		if $i < $n - 1 {
			@v[$i][$j].push(($i+1) * $n + $j);
			@v[$i+1][$j].push($i * $n + $j);
		}
		if $j < $n - 1 {
			@v[$i][$j].push($i * $n + $j+1);
			@v[$i][$j+1].push($i * $n + $j);
		}
	}
	my $m = 0;
	my @a;
	for ^$n X ^$n -> ($i, $j) {
		@a.push($m);
		$m += @v[$i][$j].elems;
	}
	for @a.kv -> $i, $v {
		say "\"$i\",A$v";
	}
	$m = 0;
	for ^$n X ^$n -> ($i, $j) {
		@a[$i * $n + $j] = $m;
		for ^@v[$i][$j].elems {
			print "V{@v[$i][$j][$_]},";
			when $_ < @v[$i][$j].elems - 1 {
				say "A{$m+1}";
			}
			say "0";
			NEXT { $m++ };
		}
	}
}
