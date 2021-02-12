# 7.2.2.1-90
sub MAIN(Str $file, Int $p? = 4, :$r?) {
	my @words = $file.IO.lines;
	my $n = @words[0].chars;
	my @o;

	for ^$p {
		@o.push('h' ~ $_);
		@o.push('v' ~ $_);
	}
	@o.push('|');
	for ^$p X 1..$n -> ($i, $j) {
		@o.push("x$i,$j");
	}
	@o.push($_) for @words;
	print "{@o}\n";
	for @words -> $w {
		for ^$p -> $i {
			@o = Empty;
			@o.push('h' ~ $i);
			for 1...$n -> $j {
				@o.push('x' ~ $i ~ ',' ~ $j ~ ':' ~
				    $w.substr($j-1, 1));
			}
			print "{@o} $w\n";
			@o = Empty;
			@o.push('v' ~ $i);
			for 1...$n -> $j {
				@o.push('x' ~ ($i + $j) % $p ~ ',' ~ $j ~ ':' ~
				    $w.substr($j-1, 1)) without $r;
				@o.push('x' ~ ($i + $j-1)%$p ~ ',' ~ $j ~ ':' ~
				    $w.substr($n-$j, 1)) with $r;
			}
			print "{@o} $w\n";
		}
	}
}
