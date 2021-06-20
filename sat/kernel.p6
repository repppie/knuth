# 7.2.2.2-21 Find kernel of at most size r
use lib '..';
use Graph;

sub MAIN($f, $r) {
	my @v = read-graph($f);

	for @v -> $v {
		my @o;
		@o.push: "{$v.num+1}";
		for $v.arcs -> $a {
			say "~{$v.num+1} ~{$a+1}";
			@o.push: "{$a+1}";
		}
		say ~@o;
	}
	for 1..^(@v.elems - $r) X 1..$r -> ($j, $k) {
		say "~s_{$j}_$k s_{$j+1}_$k";
	}
	for 1..(@v.elems - $r) X 0..$r -> ($j, $k) {
		print "~s_{$j}_$k " unless $k == 0;
		print "s_{$j}_{$k+1} " unless $k == $r;
		say "~{$j+$k}";
	}

}
