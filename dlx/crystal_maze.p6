# 7.2.2.3-22 (alternate XCC solution)
my @l = ((1,6), (1,7), (1,8), (2,3), (2,7), (2,8), (3,2), (3,5), (3,6), (3,8),
    (4,6), (5,3), (6,1), (6,3), (6,4), (6,7), (7,1), (7,2), (7,6),
    (8,1), (8,2), (8,3));

sub infix:<A>($s, $i) {
	return $s.substr($i, 1);
}

say "AB BC CD DE EF FG GH | $_" given gather { take "x$_" for 1..8 };
for <AB BC CD DE EF FG GH> -> $x {
	say "$x x{.[0]}:{$x A 0} x{.[1]}:{$x A 1}" for @l;
}
