print "algo,pat,tim,gen,lgen\n" ;
for $f (glob "lifealgo-*-2-T30.log") {
   $f =~ /lifealgo-(.*)-(.*)-2-T30.log/ or die "Can't parse" ;
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
      last if $gen < 0 || $gen > 1000000000 ;
      $lgen = int(log($gen+2)/log(2) + 0.5) ;
      print "$algo,$pat,$tim,$gen,$lgen\n" ;
   }
   close F ;
}
