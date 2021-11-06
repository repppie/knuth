sub MAIN($n) {
	say "\"$_\",A{$_*2}" for ^$n;
	my $c = 1;
	for ^$n -> $i {
		say "V{($i-1)%$n},A$c";
		say "V{($i+1)%$n},0";
		$c += 2;
	}
}

