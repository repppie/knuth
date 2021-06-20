my $vars = 1;
my %v;

sub var($v) {
	my $vv = $v;
	$vv = $v.substr(1) if $v.substr(0, 1) eq '~';
	%v{$vv} = $vars++ unless %v{$vv}:exists;
	return %v{$vv};
}

sub MAIN(:$t?) {
	my ($cl, @o);
	for lines() -> $l {
		my @c = $l.words.map({
			"{$_.substr(0, 1) eq '~' ?? '-' !! ''}{var($_)}";
		});
		@o.push: ~@c ~ ' 0';
		$cl++;
	}
	say "p cnf {$vars-1} $cl";
	.say for @o;

	with $t {
		my $f = $t.IO.open(:w, :create);
		for %v.kv -> $k, $v {
			$f.say: "$v $k";
		}
		$f.close;
	}
}
