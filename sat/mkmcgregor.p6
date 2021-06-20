# 7.2.2.2-15

sub name($j) {
	return "$j" when $j < 10;
	return "{('a'.ord + $j - 10).chr}" when $j >= 10;
}

sub v($j, $k) {
	return "{name($j)}{name($k)}"
}

class Vertex {
	my $.cnt = 0;
	has $.name;
	has $.num;
	has $.arcs is rw;
	method TWEAK() {
		$!num = $.cnt++;
	}
}

sub add-edge($from, $to) {
	$from.arcs.push: $to.num;
	$to.arcs.push: $from.num;
}

sub MAIN($n) {
	my %v;
	for 0..$n X ^$n -> ($j, $k) {
		%v{"{v($j, $k)}"} = Vertex.new(name => v($j, $k));
	}
	for 0..$n X ^$n -> ($j, $k) {
		when ($j, $k) ~~ (1, 0) {
			add-edge(%v{v($j,$k)}, %v{v($n,$_)}) for
			    floor($n/2)..^$n;
		}
		add-edge(%v{v($j,$k)}, %v{v($j+1,$k+1)}) if $j < $n and
		    $k < $n-1;
		add-edge(%v{v($j,$k)}, %v{v($j+1,$k)}) if $j < $n and $j != $k;
		add-edge(%v{v($j,$k)}, %v{v($j,$k+1)}) if $k < $n-1 and
		    $j != $k+1;
		add-edge(%v{v($j,$k)}, %v{v($n,$n-1)}) if $j == 0;
		add-edge(%v{v($j,$k)}, %v{v($n-$j,0)}) if $k < $n-1 and
		    $j == $k;
		add-edge(%v{v($j,$k)}, %v{v($n+1-$j,0)}) if $j > 0 and $j == $k;
		add-edge(%v{v($j,$k)}, %v{v($n-$j, $n-$j-1)}) if $k == $n-1 and
		    0 < $j < $k;
		add-edge(%v{v($j,$k)}, %v{v($n+1-$j,$n-$j)}) if $k == $n-1 and
		    0 < $j < $n;
	}
	add-edge(%v{v(0,0)}, %v{v(1,0)});
	add-edge(%v{v(0,0)}, %v{v($n,$_)}) for 1..floor($n/2);

	my @a;
	for %v.sort: *.value.num -> (:key($k), :value($v)) {
		say "\"{$v.name}\",A{@a.elems}";
		for ^$v.arcs.elems {
			@a.push: "V{$v.arcs[$_]},A{@a.elems+1}" if $_ <
			    $v.arcs.elems-1;
			@a.push: "V{$v.arcs[$_]},0" if $_ >= $v.arcs.elems-1;
		}
	}
	.say for @a;
}
