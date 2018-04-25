my $w = shift ;
my $h = shift ;
print "x = $w, y = $h, rule = B3/S23\n" ;
for ($i=0; $i<$h; $i++) {
   for ($j=0; $j<$w; $j++) {
      if (rand() < 0.3) {
         print "o" ;
      } else {
         print "b" ;
      }
   }
   print "\$\n" ;
}
