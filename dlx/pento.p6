my $o = ((0,0), (0,1), (0,2), (0,3), (0,4));
my $p = ((0,0), (1,0), (1,1), (2,0), (2,1));
my $q = ((0,0), (0,1), (0,2), (0,3), (1,3));
my $r = ((0,1), (0,2), (1,0), (1,1), (2,1));
my $s = ((1,0), (1,1), (1,2), (0,2), (0,3));
my $t = ((0,0), (0,1), (0,2), (1,1), (2,1));
my $u = ((0,0), (1,0), (1,1), (1,2), (0,2));
my $v = ((0,0), (1,0), (2,0), (2,1), (2,2));
my $w = ((0,0), (0,1), (1,1), (1,2), (2,2));
my $x = ((0,1), (1,0), (1,1), (1,2), (2,1));
my $y = ((1,0), (1,1), (1,2), (1,3), (0,2));
my $z = ((0,0), (0,1), (1,1), (2,1), (2,2));
my %p = 'O' => [$o], 'P' => [$p], 'Q' => [$q], 'R' => [$r], 'S' => [$s],
    'T' => [$t], 'U' => [$u], 'V' => [$v], 'W' => [$w], 'X' => [$x],
    'Y' => [$y], 'Z' => [$z];

sub x-below-zero($p) {
	$p.map: -> ($y, $x) { return True if ($x < 0); }
	return False;
}

sub y-below-zero($p) {
	$p.map: -> ($y, $x) { return True if ($y < 0); }
	return False;
}

sub translate($p, $x, $y) {
	return $p.map(-> ($yy, $xx) { ($yy + $y, $xx + $x)});
}

sub rotate(@l, $p) {
	my $o = $p;
	for ^4 {
		my @t = $o.map(-> ($y, $x) { ($x, -$y) });
		@t = translate(@t, 1, 0) while x-below-zero(@t);
		@t = translate(@t, 0, 1) while y-below-zero(@t);
		@t = @t.sort;
		@l.push(@t) unless @t ~~ any @l;
		$o = @t;
	}
}

sub coord($x) {
	when $x < 10 { return chr('0'.ord + $x) }
	when $x < 26 + 10 { return chr('a'.ord + $x - 10) }
	return chr('A'.ord + $x - 26 - 10);
}

sub in-hole($x, $y) {
	return True if ($x == 3 and $y == 3) or ($x == 3 and $y == 4) or
	    ($x == 4 and $y == 3) or ($x == 4 and $y == 4);
	return False;
}

sub MAIN(Int $n, Int $m, :$hole?) {
	for %p<>:k -> $k { %p{$k} = %p{$k}.map({ .sort.List }).Array }
	for %p<>:kv -> $k, @v [$_, *@] {
		rotate(@v, $_);
		rotate(@v, .map(-> ($y, $x) { ($y, -$x) })); # reflect
	}
	my @o = %p.keys.sort;
	for ^$n X ^$m -> ($y, $x) {
		@o.push(coord($y) ~ coord($x)) if not defined $hole or
		    not in-hole($x, $y);
	}
	print @o;
	print "\n";

	for %p.keys.sort -> $k {
		for %p{$k}:v -> @p {
			for ^$n X ^$m -> ($y, $x) {
				my &fits = sub (@p) {
					@p.map: -> ($y, $x) {
					    #say "$k ($x,$y) {in-hole($x,$y)}";
					    return False if
					    $x >= $m or $y >= $n or (defined
					    $hole and in-hole($x, $y)); }
					return True;
				};

				my @pp = translate(@p, $x, $y);
				if &fits(@pp) {
					@o = $k;
					for @pp -> ($y, $x) {
						@o.push(coord($y) ~ coord($x));
					}
					print @o;
					print "\n";
				}
			}
		}
	}
}

