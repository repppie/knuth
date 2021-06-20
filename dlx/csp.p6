# 7.2.2.3-30
my @o;

my @p = 1, 2, 1, 2, 1;
my @q = 2, 3, 3, 5, 5;
my $t = 30;
my %f = 'A' => |(1), 'B' => |(0,3), 'C' => |(0,1), 'D' => |(2,3),
    'E' => |(1,3,4), 'F' => |(0,1,2), 'G' => |(0,2), 'H' => |(0,1,3,4);

@o.push: $_ for ^$t;
#@o.push: "3:3|A 2:2|B 2:2|C D E F";
@o.push: "7:7|A 2:2|B 5:5|C 4:4|D 4:4|E 2:2|F 4:4|G 2:2|H";
for ^5 -> $k {
	for 0..($t-@q[$k]) -> $j {
		@o.push: "0:{@p[$k]}|u$j,$k";
	}
}

say "{@o}";

my %s;
for 'A'..'H' -> $c {
	%s{$c} = Array.new(Hash) without %s{$c};
	for ^5 -> $k {
		for ^($t-@q[$k]+1) -> $j {
			for $j..($j+@q[$k]-1) -> $i {
				#say "c $c j $j i $i k $k f {$k ~~ any %f{$c}}";
				%s{$c}[$i]{"$j,$k"} = 1 if $k ~~ any %f{$c};
			}
		}
	}
}

for ^$t -> $i {
	for 'A'..'H' -> $c {
		@o = Empty;
		@o.push: "$i";
		@o.push: "$c";
		@o.push: "u$_" for %s{$c}[$i].keys;
		say "{@o}";
	}
}
