# Kernels in a graph
use lib '..';
use Graph;

sub intersperse($e, @l) {
	gather for @l {
		FIRST { .take; next; }
		take $e;
		.take;
	}
}

sub MAIN($f) {
	my @v = read-graph($f);

	my $c;
	for @v -> $v {
		my @o;
		@o.push: "{$v.num+1}";
		my $i;
		for $v.arcs -> $a {
			say "~{$v.num+1} ~{$a+1} |";
			@o.push: "{$a+1}";
			say '&' unless $i++ == 0;
		}
		say "{~@o} {'| ' x (@o.elems-1)} &";
		say '&' unless $c++ == 0;
	}

	say 'c';
}
