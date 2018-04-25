my $w = shift ;
my $h = shift ;
my $hor = "3b3o3b3o4b" ;
my $ver = "bo4bobo4bo2b" ;
my $blan = "16b" ;
my @gen = ( $blan, $hor, $blan, $ver, $ver, $ver, $hor, $blan,
            $hor, $ver, $ver, $ver, $blan, $hor, $blan, $blan,
            $blan, $blan, $blan, $blan, $blan, $blan, $blan, $blan, $blan ) ;
my $wx = $w % 16 ;
my $hx = $h % 16 ;
my $wn = int($w/16) ;
my $hn = int($h/16) ;
$yb = 0 ;
$ty = 0 ;
for ($yy=0; $yy<$h; $yy++) {
   if (int($yy*$hn/$h) > $ty) {
      $yb = $yy ;
      $ty++ ;
   }
   my $s = $blan ;
   $s = $gen[$yy-$yb] if $yy-$yb < @gen ;
   $xb = 0 ;
   $tx = 0 ;
   print "$s" ;
   for ($xx=16; $xx<$w; ) {
      if (int($xx*$wn/$w) > $tx) {
         $xb = $xx ;
         $tx++ ;
         print "$s" ;
         $xx += 16 ;
      } else {
         print "b" ;
         $xx++ ;
      }
   }
   print "\$\n" ;
}
