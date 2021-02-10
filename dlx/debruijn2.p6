# 7.2.2.1-94
sub infix:<+B>($n, $b) {
	return ($n +> $b) +& 1;
}

sub MAIN($n? = 16) {
	my @a;
	for ^$n {
		@a.push("$_");
		#@a.push("p$_");
	}
	@a.push("|");
	for ^$n {
		@a.push("b$_");
	}
	say ~@a;

	for ^$n X ^$n -> ($num, $k) {
		say "$num " ~ #"p$k " ~
		    "b$k:{$num +B 0} b{($k+1)%$n}:{$num +B 1}" ~
		    " b{($k+3)%$n}:{$num +B 2} b{($k+4)%$n}:{$num +B 3}";
	}
}
