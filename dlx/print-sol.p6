sub MAIN(:$s?) {
	for lines() {
		when /\'/ {
			my @a = .split("'").sort.map({ $/[0].Str when
			    /r \d (\d)/ });
			for ^@a.elems -> $n {
				print "@a[$n]";
				without $s {
					print " ";
					print "\n" if ($n+1) % 9 == 0; 
				}
			}
			print "\n";
		}
		#default { .say without $s }
	}
}
