# 7.2.2.1-87
sub MAIN(Str $file, :$d?) {
	my @words = $file.IO.lines;
	my $n = @words[0].chars;
	my @o;

	for ^$n {
		@o.push('h' ~ $_);
		@o.push('v' ~ $_);
	}
	with $d {
		@o.push('d1');
		@o.push('d2');
	}
	@o.push('|');
	for ^$n X ^$n -> ($i, $j) {
		@o.push('x' ~ $i ~ $j);
	}
	@o.push($_) for @words;
	print "{@o}\n";
	for @words -> $w {
		for ^$n -> $i {
			@o = Empty;
			@o.push('h' ~ $i);
			for ^$n -> $j {
				@o.push('x' ~ $i ~ $j ~ ':' ~ $w.substr($j, 1));
			}
			print "{@o} $w\n";
			with $d {
				@o = Empty;
				@o.push('d1');
				@o.push('x' ~ $_ ~ $_ ~ ':' ~ $w.substr($_, 1))
				    for ^$n;
				print "{@o} $w\n";
				@o = Empty;
				@o.push('d2');
				@o.push('x' ~ $n - $_ - 1 ~ $_ ~ ':' ~
				    $w.substr($_, 1)) for ^$n;
				print "{@o} $w\n";
			}
			@o = Empty;
			@o.push('v' ~ $i);
			for ^$n -> $j {
				@o.push('x' ~ $j ~ $i ~ ':' ~ $w.substr($j, 1));
			}
			print "{@o} $w\n";
		}
	}
}
