use strict;

while (<>) {
	my $n = 0;
	while ($_ =~ /b(\d+):(\d)/g) {
		$n = $n | (1 << $1) if $2 eq "1";
	}
	printf("%x\n", $n) if $n != 0;
}
