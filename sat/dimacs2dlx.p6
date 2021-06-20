sub MAIN($f?, :$s?) {
	my $fd;
	with $f {
		$fd = $f.IO;
	} else {
		$fd = $*IN;
	}
	my ($cl, @cl, $lits, @o);
	for $fd.lines -> $l {
		when $l.substr(0, 1) eq 'p' {
			$lits = $l.words[2];
		}
		@cl[$cl++] = $l.words>>.Num;
	}
	@o.push: "c$_" for 1..$cl;
	@o.push: "l$_" for 1..$lits;
	@o.push: '|';
	@o.push: ~$_ for 1..$lits;
	say ~@o;
	for ^$cl -> $c {
		when so $s {
			say "c{$c+1} {abs($_)}:{$_ > 0 ?? 1 !! 0}" unless $_ == 0 for |@cl[$c];
		}
		for 1..^(2**(@cl[$c].elems-1)) -> $n {
			my $nn = 0;
			my @o;
			repeat {
				next unless ($n +> $nn) +& 1;
				@o.push: "{abs($_)}:{$_ > 0 ?? 1 !! 0}" given @cl[$c][$nn];
			} while $nn++ < @cl[$c].elems;
			say "c{$c+1} {@o}";
		}
	}
	say "l$_ $_:0" for 1..$lits;
	say "l$_ $_:1" for 1..$lits;
}
