#!/usr/bin/perl -w

#
# Creates an overview miniature map of a full tileset.
#
# Requires the perl GD library 2.0+
#
# Example use:
# - First, use the editor's "Autogenerate thumbnails" function to generate thumbnails in
#   all directories you want to create maps for.
#
# - Then, run the script: 
#   ./tileset_stitcher.pl ../ /relic/castle/castle_0006 test.png small

use GD;

die "Usage: $0 <path-to-map-directory> <one-tile-in-the-set> <output-file.png> [big | small]\n" unless scalar(@ARGV) >= 3;

my $mapdir = $ARGV[0];
my $basemap_path = $ARGV[1];
my $outfile = $ARGV[2];
my $hires = 1;
$hires = 0 if defined $ARGV[3] && $ARGV[3] eq 'small';

my %maps = ();
my %tilesets = ();

# Scan all subdirs except utils
scan_maps($mapdir, $mapdir, \%maps, ["^/utils/*", "^/rendered_overviews/*", "^/unofficial/*", "^/_old/*", "^/reuseables/*"]);
validate_linking(\%maps);

# Find the base map
$basemap = $maps{$basemap_path};
die "Couldn't find map with path $basemap_path.\n" unless defined $basemap;

# Single out the tileset and calculate relative map positions
$tileset = label_tileset($basemap, \%maps, "foo", 0, 0, 0,0);

# Figure out tileset size
my ($xmin, $ymin, $xmax, $ymax) = ($basemap->{mapx}, $basemap->{mapy}, $basemap->{mapx}, $basemap->{mapy});
foreach my $map (@$tileset)
{
    $xmin = $map->{mapx} if $map->{mapx} < $xmin;
    $ymin = $map->{mapy} if $map->{mapy} < $ymin;
    $xmax = $map->{mapx} if $map->{mapx} > $xmax;
    $ymax = $map->{mapy} if $map->{mapy} > $ymax;
}

my $setwidth  = $xmax - $xmin + 1;
my $setheight = $ymax - $ymin + 1;

print "Tileset is $setwidth x $setheight maps\n";

# ISO map size:
my ($iso_xlen, $iso_ylen) = $hires ? (144, 72) : (48,23);

# Origin pixel (pixel position of map @0,0 (in northwest corner))
my $origin_x = $setheight * int($iso_xlen / 2);
my $origin_y = 0;

#
# Calculate map tile positions on map (and get max/min coordinates)
#

my ($isoxmin, $isoymin, $isoxmax, $isoymax) = (100000, 1000000, 0, 0);
foreach my $map (@$tileset)
{
    ($map->{isox}, $map->{isoy}) = tile_position($map->{mapx} - $xmin, $map->{mapy} - $ymin, $iso_xlen, $iso_ylen, $origin_x, $origin_y);
    $isoxmin = $map->{isox} if $map->{isox} < $isoxmin;
    $isoymin = $map->{isoy} if $map->{isoy} < $isoymin;
    $isoxmax = $map->{isox} if $map->{isox} > $isoxmax;
    $isoymax = $map->{isoy} if $map->{isoy} > $isoymax;
}

my $isowidth = $isoxmax - $isoxmin + $iso_xlen;
my $isoheight = $isoymax - $isoymin + $iso_ylen;

# create a new image buffer
my $image = new GD::Image($isowidth, $isoheight, 1);
$image->alphaBlending(1);
my $white = $image->colorAllocate(255,255,255);

# Insert map thumbnails
foreach my $map (@$tileset)
{
    my $thumb_file = thumbnail_file($map, $hires);
    if(! -f $thumb_file)
    {
        print STDERR "Missing thumbnail: ($thumb_file)\n";
        next;
    }
    my $thumb = GD::Image->newFromPng("$thumb_file", 1);

    $image->copy($thumb, $map->{isox} - $isoxmin, $map->{isoy} - $isoymin,
            0,0,$thumb->width, $thumb->height);
    if($hires)
    {
        $image->string(gdTinyFont, 
                $map->{isox} - $isoxmin + $iso_xlen/2 - gdTinyFont->width * length($map->{filename}) / 2,  
                $map->{isoy} - $isoymin + $iso_ylen/2 - gdTinyFont->height / 2, 
                $map->{filename}, $white);
    }
}

open OUT, ">$outfile" or die "Couldn't open $outfile: $!\n";
binmode OUT;
print OUT $image->jpeg;
close OUT;

sub tile_position
{
    my ($mapx, $mapy, $iso_xlen, $iso_ylen, $origin_x, $origin_y) = @_;
    my $x = $origin_x - ($mapy + 1) * int($iso_xlen / 2) + $mapx * int($iso_xlen / 2);
    my $y = $origin_y +  $mapy      * int($iso_ylen / 2) + $mapx * int($iso_ylen / 2);
    return ($x,$y);
}

sub thumbnail_file
{
    my ($map, $hires) = @_;
    $map->{fullpath} =~ /^(.*)\/([^\/]*)$/;
    return $hires ? "$1/.dedit/$2.preview" : "$1/.dedit/$2.icon";
}

#
# The following functions were directly copied from tileset_updater.pl
# Perhaps a common library would be good?
#

# Recurse over a tileset graph and label each tile with coordinates and
# the tileset label
sub label_tileset
{
    my ($map, $maps, $label, $x, $y, $mapx, $mapy) = @_;
    
    $map->{'tileset'} = $label;
    $map->{'x'} = $x;
    $map->{'y'} = $y;
    $map->{'mapx'} = $mapx;
    $map->{'mapy'} = $mapy;
    
    my @set = ($map);
    my @dy=(0, -1,-1,0,1,1,1,0,-1);
    my @dx=(0, 0,1,1,1,0,-1,-1,-1);
    
    for (my $i=1; $i<=8; $i++)
    {
        my $target = $map->{$i};
        if (defined $target && !defined $maps->{$target}->{'tileset'}) 
        {
            my ($x2, $y2) = ($x, $y);
            if ($dx[$i] == -1) { $x2 -= $maps->{$target}->{'width'} }
            elsif ($dx[$i] == 1) { $x2 += $map->{'width'} }
            if ($dy[$i] == -1) { $y2 -= $maps->{$target}->{'height'} }
            elsif ($dy[$i] == 1) { $y2 += $map->{'height'} }
            push @set, @{label_tileset($maps->{$target}, $maps, $label, $x2, $y2, $mapx + $dx[$i], $mapy + $dy[$i])};
        }
    }

    return \@set;
}

# Validate and clean up tile linking if possible
sub validate_linking
{
    my ($maps) = @_;

    my @revdir = (0, 5, 6, 7, 8, 1, 2, 3, 4);
    
    my $errors = 0;
    foreach my $path (keys %$maps) {
#        print "$path\n";
        for (my $i=1; $i<=8; $i++)
        {
            my $target = $maps->{$path}->{$i};
            if (defined $target) 
            {
#                print "  $i: $target\n";
                if(! defined $maps->{$target} ) {
                    print STDERR "WARNING: $target doesn't exist (linked from $path)\n";
                    # Remove the link for now
                    undef $maps->{$path}->{$i};
                } else {
                    if (! defined $maps->{$target}->{$revdir[$i]})
                    {
                        print STDERR "WARNING: $target doesn't link back to $path\n";
                         # We force the linkback here...
                        $maps->{$target}->{$revdir[$i]} = $path;
                    } elsif($maps->{$target}->{$revdir[$i]} ne $path) 
                    {
                        print STDERR "ERROR: $target links back to $maps->{$target}->{$revdir[$i]} instead of $path\n";
                        $errors++;
                    }
                }
            }
        }
    }

    return $errors;
#    die "Aborting due to map linking errors\n" if $errors;
}

# Convert relative paths to absolute
sub normalize_path
{
    my ($path, $basedir) = @_;

    # Absolute paths are easy
    return $path if $path =~ /^\//;
    
    # Relative paths requires a little work
    $path = "$basedir/$path";
    # Handle parent directory references "/../"
    while($path =~ /(.*)\/[^\/]+\/\.\.\/(.*)/) {
        $path = "$1/$2";
    }
    # Handle current directory references "/./"
    while($path =~ /(.*)\/\.\/(.*)/) {
        $path = "$1/$2";
    }
    # Handle double slashes "//"
    while ($path =~ /\/\//) {
        $path =~ s/\/\//\//g;
    }

    return $path;
}

# Recursively scan a directory for map files and store info about the 
# maps in $hash
sub scan_maps
{
    my ($dir, $mapdir, $hash, $ignore) = @_;

    my @conv = (0, 1, 3, 5, 7, 2, 4, 6, 8 );

    opendir (DIR, $dir)  || die "Couldn't read directory $dir: $!\n";
    my @contents = readdir DIR;
    closedir DIR;
    
    foreach my $entry (@contents) {
        # Skip some obvious non-map files and dirs
        next if $entry =~ /(^CVS$)|(^\..*)|(.*\.txt$)|(.*\.art$)|(.*\.tl$)|(.*\.lua$)|(^README$)/;
        my $fullpath = "$dir/$entry";
        my $path = substr($fullpath, length($mapdir));
        
        # Check skip list
        my $skip = 0;
        foreach (@$ignore) {$skip=1 if $path =~ /$_/;};
        next if $skip;

        # Recurse into directories
        scan_maps($fullpath, $mapdir, $hash, $ignore) if -d $fullpath;

        # Scan files for map header and contents
        if (-f $fullpath) {
            open (FILE, $fullpath) || die "Couldn't read file $fullpath: $!\n";
            
            my $map = undef;
            
            while($line = <FILE>) {
                chomp $line;
                if (! defined $map)
                {
                    last if $line ne 'arch map';
                    $map = { 
                        'fullpath' => $fullpath,
                        'path' => $path,
                        'directory' => substr($path, 0, rindex($path, "/")),
                        'filename' => substr($path, rindex($path, "/") + 1)
                    };
                }

                if ($line =~ /tile_path_(\d) (.*)/)
                {
                    # We convert from mapfile tile_path numbering to normal directions
                    $map->{$conv[$1]} = normalize_path($2, $map->{'directory'});
                } elsif ($line =~ /^width (\d+)$/) {
                    $map->{'width'} = $1;
                } elsif ($line =~ /^height (\d+)$/) {
                    $map->{'height'} = $1;
                } elsif ($line eq 'end') {
                    last;
                }
            }
            $hash->{$map->{'path'}} = $map if defined $map;
            
            close FILE;
        }
    }
}
