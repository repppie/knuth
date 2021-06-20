sub MAIN(:$r?) {
	for lines() {
		my @s;
		next if not $_ ~~ /\'/;
		for $_ ~~ m:g/\'(<-[']>+)\'/ {
			for $_[0].words.grep({ $_ ~~ /':'/ }) {
			    $_ ~~ m:g/(\d+) ',' (\d+) ':' (.)/;
			    @s[.[0]][.[1]] = .[2] given $/[0];
			}
		}
		for ^6 -> $i {
			for ^6 -> $j {
				print "{@s[$i][$j]} " with @s[$i][$j];
				print ". " without @s[$i][$j];
			}
			say "";
		}
		say "";
	}
}
