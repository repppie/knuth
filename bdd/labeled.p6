# 7.1.4-78 Number of graphs with 12 labeled vertices with max vertex degree d.

sub popcnt($n) {
	return [+] $n.base(2).comb
}

sub infix:<+B>($n, $b) {
        return ($n +> $b) +& 1;
}


sub MAIN($n, $max) {
	my $pp;
	my @a[$n;$n];
	my $t = 0;
	for ^$n -> $i {
		for $i^..^$n -> $j {
			@a[$i;$j] = $t + 1;
			@a[$j;$i] = $t++ + 1;
		}
	}

	for 1..$n -> $i {
		my $t;
		my $p;
		for ^(2**($n-1)) -> $t {
			if popcnt($t) <= $max {
				my $tt;
				my $off = 0;
				for ^($n-1) {
					$off++ if $off == $i - 1;
					print "~" if $t +B $_ == 0;
					print "{@a[$i-1;$off]} ";
					print "& " unless $tt++ == 0;
					$off++;
				}
				say "|" unless $p++ == 0;
			}
		}
		say "&" unless $pp++ == 0;
	}
	say 'c';
	#say 'p';
}
