# Graph coloring

use lib '..';
use Graph;

sub MAIN($f, $d, Int :$max?, :$radio?, :$p?) {
	my @v = read-graph($f);
	my @near with $radio;

	# Radio coloring (7.2.2.2-36)
	with $radio {
		for @v -> $v {
			for $v.arcs -> $a {
				@near[$v.num].push: $_ unless $_ == $v.num for
				    @v[$a].arcs;
				@near[$v.num].push: $a;
			}
			@near[$v.num].=unique;
		}
	}

	(note "{.num+1} -> {.name}" for @v) with $p;
	
	for @v -> $v {
		my @o;
		@o.push: "{$v.num+1}_$_" for ^$d;
		say ~@o;

		for $v.arcs -> $a {
			with $radio {
				say "~{$v.num+1}_$_ ~{$a+1}_{$_+1}" for ^($d-1);
			}
			next if $v.num > $a;
			say "~{$v.num+1}_$_ ~{$a+1}_$_" for ^$d;
		}
		with $radio {
			for |@near[$v.num] -> $a {
				say "~{$v.num+1}_$_ ~{$a+1}_$_" for ^$d;
			}
		}
	}

	# Color 0 is used at most $max times (7.2.2.2-17)
	with $max {
		for 1..^(@v.elems - $max) X 1..$max -> ($j, $k) {
			say "~s_{$j}_$k s_{$j+1}_$k";
		}
		for 1..(@v.elems - $max) X 0..$max -> ($j, $k) {
			print "~s_{$j}_$k " unless $k == 0;
			print "s_{$j}_{$k+1} " unless $k == $max;
			say "~{$j+$k}_0";
		}
	}
}
