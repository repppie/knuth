sub MAIN(:$r?) {
	for lines() {
		my @s;
		my ($i,$j) = (0, 1);
		for $_ ~~ m:g/\''h' (\d+) <-[\']>* (\w ** 5)\'/ {
			@s[.[0]] = .[1].uc;
			$i++;
		}
		@s = @s.reverse with $r;
		for @s -> $w {
			print " " x (((so $r) ?? $j++ !! $i--) * 2);
			for ^$w.chars {
				print "{$w.substr($_, 1)} ";
			}
			print "\n";
		}
	}
}
