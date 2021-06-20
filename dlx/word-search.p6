my @words = <ONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN ELEVEN TWELVE>;
my ($N, $M);

sub w($w, $i, $j, $di, $dj) {
	my $n = $w.chars;
	return gather for ^$n -> $x {
	    #take "x{$i+$di*$x},{$j+$dj*$x}";
	    take "{$i+$di*$x},{$j+$dj*$x}:{$w.substr($x,1)}";
	} if (0 <= $i+$di*$n <= $N) and (0 <= $j+$dj*$n <= $M);
	return Nil;
}


sub MAIN($n? = 6, $m? = 6) {
	$N = $n;
	$M = $m;
	my @o;
	@o.push: $_ for @words;
	#@o.push: '.1 .2';
	#for ^$n X ^$m -> ($i, $j) {
		#@o.push: "x$i,$j";
	#}
	@o.push: '|';
	for ^$n X ^$m -> ($i, $j) {
		@o.push: "$i,$j";
	}
	say "{@o}";

	for @words -> $w {
		for ^$n X ^$m -> ($i, $j) {
			my $y;
			say "$w $y" with $y = w($w, $i, $j, 0, 1);
			say "$w $y" with $y = w($w.flip, $i, $j, 0, 1);
			say "$w $y" with $y = w($w, $i, $j, 1, 0);
			say "$w $y" with $y = w($w.flip, $i, $j, 1, 0);
			say "$w $y" with $y = w($w, $i, $j, 1, 1);
			say "$w $y" with $y = w($w.flip, $i, $j, 1, 1);
			say "$w $y" with $y = w($w, $i, $j, -1, 1);
			say "$w $y" with $y = w($w.flip, $i, $j, -1, 1);
		}
	}
	#for ^$n X ^$m -> ($i, $j) {
		#say ".$_ x$i,$j" for 1..2; 
	#}
}
