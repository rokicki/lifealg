my @pat = qw(QGC1 breeder caterpillar jagged lidka mcc r spiral) ;
my @algo = qw(qlife ulifelib list8x8 tree lifelib hlife) ;
for ($inc=4096; ; $inc *= 2) {
   for $pat (@pat) {
      for $algo (@algo) {
         my $cmd = "./lifealgo -T 30 -v -v -i $inc -f $pat.rle $algo" ;
         print "- $cmd\n" ;
         system("$cmd > lifealgo-$pat-$algo-i$inc-T30.log") ;
      }
   }
}
