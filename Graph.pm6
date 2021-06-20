unit module Graph;

class Vertex {
	my $.cnt = 0;
	has Str $.name;
	has Int $.num;
	has $.arcs;
	method TWEAK() {
		$!num = $.cnt++;
	}
	method add-arc(Int $to) {
		$!arcs.push: $to;
	}
}

sub read-graph($f --> Array) is export {
	my @l = $f.IO.lines;
	my (@a, @v);
	for ^@l.elems {
		@v.push: Vertex.new(name => ~$0) => $1.Num when @l[$_] ~~
		    /^ '"' (<alnum>+) '",A' (<digit>+)/;
		@a.push: $0.Int => ($<arc> ?? $<arc>.Int !! Nil) when @l[$_] ~~
		    /^'V' (<digit>+) ',' [0 || ['A' $<arc> = [<digit>+]]]/;
	}
	my @vv;
	for @v -> (:key($v), :value($a)) {
		my $aa = $a;
		while defined $aa {
			$v.add-arc: @a[$aa].key;
			$aa = @a[$aa].value;
		}
		@vv.push: $v;
	}
	return @vv;
}

