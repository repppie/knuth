sub postfix:<!>($n) {
	return [*] 1 .. $n
}

sub infix:<C>($n, $k) {
	return $n! / ($k! * ($n - $k)!)
}

sub infix:<+B>($n, $b) {
	return ($n +> $b) +& 1;
}

sub MAIN($n, :$p? = 0, :$q? = $n) {
	my $m = do for $p..$q { $n C $_ }.sum;
	my $nums := ((1 +< $p - 1)...(1 +< $q - 1));
	my @o;
	@o.push("a$_ p$_") for $nums;
	@o.push('|');
	@o.push("b$_") for ^$m;
	say ~@o;

	for (1 +< $p - 1)..(1 +< $q - 1) -> $num {
	#for $nums -> $num {
		for ^$m -> $b {
			next if ($num > 0 and $b == 0) or ($num == 0 and $b>0);
			@o = ();
			print "a$num p$b ";
			for ^$n {
				@o.push("b{($b + $_) % $m}:{$num +B $_}");
			}
			say ~@o;
		}
	}
}
