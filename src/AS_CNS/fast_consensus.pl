#!/usr/bin/env perl
#unitig 513
#len 0
#cns 
#qlt 
#data.unitig_coverage_stat  1.000000
#data.unitig_microhet_prob  1.000000
#data.unitig_status         X
#data.unitig_suggest_repeat F
#data.unitig_suggest_unique F
#data.unitig_force_repeat   F
#data.unitig_force_unique   F
#data.contig_status         U
#data.num_frags             10796
#data.num_unitigs           0
#FRG type R ident    121618 container         0 parent         0 hang      0      0 position      0  22340
#FRG type R ident   1248726 container         0 parent         0 hang      0      0 position  12386    969
#FRG type R ident    228344 container         0 parent   1248726 hang   4108  45245 position  57665   5097
#FRG type R ident   2256789 container    228344 parent    228344 hang     20 -52048 position   5117   5617
#FRG type R ident   1471016 container    228344 parent    228344 hang  16933 -21862 position  22049  35783
#FRG type R ident   1698994 container         0 parent    228344 hang  19250  32220 position  89882  24347
#
#>3023a80f-777e-47d8-a12f-7a2f39d54173.0_9518:F:1640:11198_21344:F.0,330 mate=0,0 lib=mr,1 clr=LATEST,1,32498 deleted=0

my $readseq=$ARGV[0];
my $unitigseq=$ARGV[1];
my $iid=0;
my %seq=();

#read in read sequences
open(FILE,$readseq);
while($line=<FILE>){
  chomp($line);
  if(substr($line,0,1) eq ">"){
    my @f=split(/\s+/,$line);
    my @ff=split(/,/,$f[0]);
    $iid=$ff[-1];
  }else{
    $seq{$iid}=$line;
  }
}

#read in unitig sequences
open(FILE,$unitigseq);
while($line=<FILE>){
  chomp($line);
  if(substr($line,0,1) eq ">"){
     $iid=substr($line,1);
  }else{
     $seq{$iid}=$line;
  }
}

#read in layout file from STDIN
my @layout_lines=();
while($line=<STDIN>){
  chomp($line);
  if(substr($line,0,6) eq "unitig"){
    if($#layout_lines>-1){
      do_consensus(@layout_lines);
    }
    @layout_lines=();
    push(@layout_lines,$line);
  }else{
    push(@layout_lines,$line);
  }
}
if($layout_lines>0){
  do_consensus(@layout_lines);
}

sub do_consensus{
my $consensus="";
my @lines=@_;
my @f=split(/\s+/,$lines[0]);
if(defined($seq{"utg_".$f[1]})){
print ">utg_$f[1]\n",$seq{"utg_".$f[1]},"\n";
}else{
foreach my $l (@lines){
  my @f=split(/\s+/,$l);
  my $beg=0;
  my $end=0;
  my $ori=0;
  if($f[0] eq "FRG" && $f[6] eq "0"){#not contained
    if($f[13]<$f[14]){
      $beg=$f[13];
      $end=$f[14];
      $ori=0;
    }else{
      $beg=$f[14];
      $end=$f[13];
      $ori=1;
    }
    if($end > length($consensus) && not($seq{$f[4]} eq "")){
      $consensus=substr($consensus,0,$beg).($ori==0?$seq{$f[4]}:reverse_complement($seq{$f[4]}));
    }
  }
}
print ">utg_$f[1]\n",$consensus,"\n" if(length($consensus)>50000);
}
}

sub reverse_complement{
  my $str=$_[0];
  $str =~ tr/acgtACGTNn/tgcaTGCANn/;
  $str = reverse ($str);
  return ($str);
}

