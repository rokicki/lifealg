my @pat = qw(QGC1 breeder caterpillar jagged lidka mcc r spiral unlim bp4kx4k r4kx4k) ;
my @algo = qw(qlife ulifelib list8x8 tree lifelib hlife list16x16) ;
#for ($inc=4096; ; $inc *= 2) {
{
   for $pat (@pat) {
      for $algo (@algo) {
         my $cmd = "./lifealgo -T 30 -v -v -i 2 -2 -f $pat.rle $algo" ;
         next if -f "lifealgo-$pat-$algo-2-T30.log" ;
         print "- $cmd\n" ;
         system("$cmd > lifealgo-$pat-$algo-2-T30.log") ;
      }
   }
}
