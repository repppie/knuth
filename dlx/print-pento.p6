sub coord($x) {
	when $x ~~ /<[0..9]>/ { return $x.Int }
	when $x ~~ /<[a..z]>/ { return $x.ord - 'a'.ord + 10 }
	when $x ~~ /<[A..Z]>/ { return $x.ord - 'A'.ord + 10 + 26 }
	die "unexpected coord $x";
}

sub color($p) {
	return "\e[{41 + $_}m" ~ ' ' ~ "\e[0m" when $p ge 'O' and $p le 'U'
	    given $p.ord - 'O'.ord;
	return "\e[{41 + $_}m" ~ ' ' ~ "\e[0m" given $p.ord - 'U'.ord;
}

sub MAIN(:$c?) {
	my ($n, $m) = (0, 0);
	for lines() {
		$n = $m = 0;
		when /\'/ {
			my regex R { (<[O..Z]>) ' '
			    (<[0..9 a..z]><[0..9 a..z]>
			    [\s?]) ** 5 };
			my @b;
			my @a = .split("'").grep({$_ ne "" and $_ ne " "}).sort
			    .map({ $_ ~~ &R;
			    ($/[0].Str, $/[1].map({.Str.words}).map: *.comb) });
			for @a -> ($p, @c) {
				$n = coord(.[0])+1 if coord(.[0])+1 > $n for @c;
				$m = coord(.[1])+1 if coord(.[1])+1 > $m for @c;
			}
			for @a -> ($p, @c) {
				@b[coord(.[0]) * $m + coord(.[1])] = $p for @c;
			}
			for ^$n X ^$m -> ($i, $j) {
				given @b[$i * $m + $j] {
					when not defined $_ { print ' ' }
					when defined $c { print color($_) }
					print $_;
				}
				print "\n" if $j == $m - 1;
			}
			print "\n";
		}
	}
}
