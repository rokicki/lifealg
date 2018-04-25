my @pat = qw(bp4kx4k r4kx4k) ;
my @algo = qw(list list2 list3) ;
#for ($inc=4096; ; $inc *= 2) {
{
   for $pat (@pat) {
      for $algo (@algo) {
         my $cmd = "./lifealgo -w 4096 -h 4096 -T 30 -v -v -i 2 -2 -f $pat.rle $algo" ;
         next if -f "lifealgo-$pat-$algo-2-T30.log" ;
         print "- $cmd\n" ;
         system("$cmd > lifealgo-$pat-$algo-2-T30.log") ;
      }
   }
}
