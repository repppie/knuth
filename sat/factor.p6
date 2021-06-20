# Factoring integers with Dadda multiplier

enum Ops <And Or Xor>;
class Op {
	has Ops $.op is required;
	has $.dst is rw;
	has $.l is required;
	has $.r is required;
}

sub MAIN($m, $n, $z) {
	my (@bin, @ops);
	my ($int, $carry) = (1, 1);

	die "m > n" if $m > $n;
	die "z too big" if $z >= 2**($m+$n);

	for 1..$m X 1..$n -> ($i, $j) {
		@bin[$i + $j].push: Op.new(op=>And, l=>"x$i", r=>"y$j");
	}

	my $did;
	repeat {
		$did = False;
		for 2..^@bin.elems -> $i {
			when @bin[$i].elems == 1 {
				$did = True;
				my $z = @bin[$i].pop;
				$z.dst = "#z{$i-1}";		
				@ops.push: $z;
			}
			when @bin[$i].elems == 2 {
				# Half adder
				$did = True;
				my ($a1, $a2) = (@bin[$i].pop, @bin[$i].pop);
				$a1.dst = "#a{$int++}" unless so $a1.dst;
				$a2.dst = "#a{$int++}" unless so $a2.dst;
				my $z = Op.new(op=>Xor, l=>$a1.dst,
				    r=>$a2.dst, dst=>"#z{$i-1}");
				@ops.append: $a1, $a2, $z;
				@bin[$i+1].push: Op.new(op=>And, l=>$a1.dst,
				    r=>$a2.dst, dst=>"#c{$carry++}");
			}
			when @bin[$i].elems >= 3 {
				# Full adder
				$did = True;
				my ($a1, $a2, $a3) = (@bin[$i].pop,
				    @bin[$i].pop, @bin[$i].pop);
				$a1.dst = "#a{$int++}" unless so $a1.dst;
				$a2.dst = "#a{$int++}" unless so $a2.dst;
				$a3.dst = "#a{$int++}" unless so $a3.dst;
				my $p = Op.new(op=>Xor, l=>$a1.dst, r=>$a2.dst,
				    dst=>"#a{$int++}");
				my $q = Op.new(op=>Xor, l=>$p.dst, r=>$a3.dst);
				my $r = Op.new(op=>And, l=>$p.dst, r=>$a3.dst,
				    dst=>"#a{$int++}");
				my $s = Op.new(op=>And, l=>$a1.dst, r=>$a2.dst,
				    dst=>"#a{$int++}");
				my $t = Op.new(op=>Or, l=>$r.dst, r=>$s.dst,
				    dst=>"#c{$carry++}");
				@ops.append: $a1, $a2, $a3, $p, $r, $s;
				@bin[$i].push: $q;
				@bin[$i+1].push: $t;
			}
		}
	} while $did;

	#note @ops.list.grep({ .op ~~ And }).elems;
	#note @ops.list.grep({ .op ~~ Or }).elems;
	#note @ops.list.grep({ .op ~~ Xor }).elems;
	#.note for @ops;

	my @out;
	for @ops -> $o {
		# Tseytin encoding
		given $o.op {
			when And {
				@out.push: "{$o.l} ~{$o.dst}";
				@out.push: "{$o.r} ~{$o.dst}";
				@out.push: "~{$o.l} ~{$o.r} {$o.dst}";
			}
			when Or {
				@out.push: "~{$o.l} {$o.dst}";
				@out.push: "~{$o.r} {$o.dst}";
				@out.push: "{$o.l} {$o.r} ~{$o.dst}";
			}
			when Xor {
				@out.push: "~{$o.l} {$o.r} {$o.dst}";
				@out.push: "{$o.l} ~{$o.r} {$o.dst}";
				@out.push: "{$o.l} {$o.r} ~{$o.dst}";
				@out.push: "~{$o.l} ~{$o.r} ~{$o.dst}";
			}
			default {
				die ~$_;
			}
		}
	}

	@out.push: "{(1 +< $_) +& $z ?? '' !! '~'}#z{$_+1}" for ^($m+$n);
	.say for @out;
}
