sub infix:<+B>($n, $b) {
	return ($n +> $b) +& 1;
}

sub G($n) {
	my $f = 0;
	for (1..(2**$n)).rotor(2) -> ($a, $b) {
		say "~$a ~$b & ~$a $b & | $a $b & | {$f == 0 ?? '' !! '&'}";
		$f++;
	}
}

sub GG($n) {
	my @o;
	my $m = 2**$n;
	for 1..$m -> $i {
		@o.push: "$i {$m-$i} {$m-$i+1}{$i==1??'.'!!''}" when
		    $i%2 == 1 && $i != $m-1;
		@o.push: "$i 1 {$m-$i+1}" when $i % 2 == 1 && $i == $m-1;
		@o.push: "$i 0 {$m-$i+1}" when $i % 2 == 0 && $i != $m;
		@o.push: "$i 0 1" when $i % 2 == 0 && $i == $m;
	}
	say "bdd";
	.say for @o.reverse;
}

sub MAIN($n) {
	G(1);
	for 2..$n -> $i {
		say "dup";
		my $j = 1;
		for (1..(2**($i-1))).reverse {
			say "chg 1 {$_} {$_*2-1}" unless $_ == 1;
		}
		for (1..(2**($i-1))).reverse {
			say "chg 0 {$_} {$_*2}";
		}
		say "&";
		G($i);
		say "&";
	}
	say "c";
}
