for lines() {
	when /\'/ {
		my $n = 0;
		#for .comb(/'b' (\d+) ':' (\d+)/) .map({ .split(':') })
		    #.map({ ($/[0].Int, $^a.[1].Int) given $^a.[0] ~~
		    #    /'b' (\d+)/ })
		    #.map({ (.[0].substr(1..*).Int, .[1].Int) })
		#for .comb(/'b' (\d+) ':' (\d+)/).map({
		my @a = gather { .split("'", :skip-empty).grep({$_ ne " "})
		    .map({ my @a = .words.grep({.substr(0,1) eq "b"});
		        for @a {
			    	take (.[0].substr(1..*).Int, .[1].Int) given
			    	    .split(':') 
		    	}
		        })
		}
		for @a -> ($i, $b) {
		    #.map({(.[0].substr(1..*).Int, .[1].Int) given .split(':') })
			$n = $n +| (1 +< $i) when $b == 1;
		}
		say $n.fmt("%x");
	}
}

