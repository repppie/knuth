while (<>) {
	while ($_ =~ /'(h\d+)[^']*([a-z]{5})'/g) {
		print "$1 $2\n";
	}
}
