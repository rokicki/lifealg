$max = shift || 30 ;
print "algo,pat,tim,gen,lgen\n" ;
for $f (glob "lifealgo-*-2-T$max*.log") {
   $f =~ /lifealgo-(.*)-(.*)-2-T(\d+)/ or die "Can't parse" ;
   next if $3 != $max ;
   $pat = $1 ;
   $algo = $2 ;
   open F, $f or die "Can't open" ;
   while (<F>) {
      next if /Init/ || /Final/ ;
      chomp ;
      @f = split " ", $_ ;
      $tim = $f[0] ;
      $gen = $f[1] ;
      $pop = $f[2] ;
      last if $gen < 0 || $gen > 1080000000 ;
      $lgen = int(log($gen+2)/log(2) + 0.5) ;
      $key = "$algo $pat " . (100 + $lgen) ;
      if (defined($seen{$key})) {
         $tim = $seen{$key} if $tim > $seen{$key} ;
      } else {
         $seen{$key} = $tim ;
      }
      $data{$key} = "$algo,$pat,$tim,$gen,$lgen\n" ;
   }
   close F ;
}
print $data{$_} for sort keys %data ;
