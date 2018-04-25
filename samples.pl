my @pat = qw(QGC1 breeder caterpillar jagged lidka mcc r spiral bp4kx4k unlim) ;
my @algo = qw(qlife ulifelib list8x8 tree lifelib hlife list16x16) ;
for ($inc=1; ; $inc *= 2) {
   for $pat (@pat) {
      for $algo (@algo) {
         my $cmd = "./lifealgo -T 30 -v -v -i $inc -f $pat.rle $algo" ;
         next if -f "lifealgo-$pat-$algo-i$inc-T30.log" ;
         print "- $cmd\n" ;
         system("$cmd > lifealgo-$pat-$algo-i$inc-T30.log") ;
      }
   }
}
