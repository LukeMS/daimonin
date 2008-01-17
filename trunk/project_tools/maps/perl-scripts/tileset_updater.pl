#!/usr/bin/perl -w

use Data::Dumper;

# Analyzes all maps in the map directory and calculates 
# and udpates the maps' tileset information

my $LF = "\012"; # We must write Unix newlines in map files...

my $modify_files = 1;

if (scalar(@ARGV) && $ARGV[0] eq '-n') {
    $modify_files = 0;
    shift @ARGV;
}

my $mapdir = $ARGV[0] || die "Usage: $0 [-n] <path-to-map-directory>\n -n: Don't modify any files.\n";

my %maps = ();          # path -> $map
my %tilesets = ();      # id   -> array($map, ...)
my %old_tilesets = ();  # id   -> array($map, ...)

# Scan all subdirs with some exceptions
scan_maps($mapdir, $mapdir, \%maps, [
    "^/scripts/", "^/lua/", "^/utils/*",         # Not map directories
    "^/unofficial/*", "^/_old/", "^/reuseables/*" # Not on main server
    ]);
validate_linking(\%maps);
relabel_old_tilesets(\%maps, \%tilesets);
find_tilesets(\%maps, \%tilesets);
update_map_files(\%maps, \%tilesets, $modify_files);

# Scan nonpub last to avoid any changes there having effect on main maps
#$nrof_public_tilesets = scalar keys %tilesets;
#%maps = ();
#%tilesets = ();
#scan_maps("$mapdir/nonpub", $mapdir, \%maps, []);
#validate_linking(\%maps);
#find_tilesets(\%maps, \%tilesets, $nrof_public_tilesets + 1);
#update_map_files(\%maps, \%tilesets, $modify_files);

# Go through all map files and update them with tileset_id, tileset_x and tileset_y
sub update_map_files
{
    my ($maps, $tilesets, $modify) = @_;
    
    foreach my $tileset (keys %$tilesets)
    {
        print "Tileset $tileset (".(scalar @{$tilesets->{$tileset}})." tiles):\n";
        foreach my $map (@{$tilesets->{$tileset}})
        {
            print "  $map->{path} ($map->{x}, $map->{y})";
            if(defined($map->{old_tileset}) && $map->{old_tileset} != $map->{tileset})
            { print " (changed from tileset $map->{old_tileset})"; } 
            elsif(! defined($map->{old_tileset}))
            { print " (no previous tileset id)"; }
            elsif($map->{old_x} ne $map->{x} || $map->{old_y} ne $map->{y})
            { print " (changed from ($map->{old_x}, $map->{old_y}))"; } 
                
            print "\n";

            next unless $modify;
            
            open (FILE_IN, $map->{'fullpath'}) || die "Couldn't read file $map->{fullpath}: $!\n";
            open (FILE_OUT, ">$map->{'fullpath'}.new") || die "Couldn't create file $map->{fullpath}.new: $!\n";
            binmode FILE_OUT; 

            my $msg = 0;
            while($line = <FILE_IN>) {
                chomp $line;
                $msg = 1 if $line eq "msg";
                $msg = 0 if $line eq "endmsg";
                last if $line eq "end" && !$msg;
                next if $line =~ /^(tileset_id)|(tileset_x)|(tileset_y)/ && !$msg;
                print FILE_OUT $line, $LF;
            }

            print FILE_OUT "tileset_id $tileset$LF";
            print FILE_OUT "tileset_x $map->{x}$LF";
            print FILE_OUT "tileset_y $map->{y}$LF";
            print FILE_OUT $line, $LF;
            
            my @rest = <FILE_IN>;
            print FILE_OUT @rest;
            
            close FILE_OUT;
            close FILE_IN;

            rename "$map->{'fullpath'}.new", $map->{'fullpath'};
        }
    }
}

sub relabel_old_tilesets
{
    my ($maps, $tilesets) = @_;
        
    foreach my $path (keys %$maps) {
        next if defined $maps->{$path}->{'tileset'};      # Skip already marked maps
        next if !defined $maps->{$path}->{'old_tileset'}; # Skip maps with no previous tileset ID
        next if defined $tilesets->{$maps->{$path}->{'old_tileset'}};  # Skip tilesets with already marked maps
        
        $tilesets->{$maps->{$path}->{'old_tileset'}} = label_tileset($maps->{$path}, $maps, 
            $maps->{$path}->{'old_tileset'}, $maps->{$path}->{'old_x'}, $maps->{$path}->{'old_y'});
    }
}

# Traverse the graph and label all unique unconnected subgraphs
sub find_tilesets
{
    my ($maps, $tilesets) = @_;
        
    my $label = 1;
    foreach my $path (keys %$maps) {
        next if defined $maps->{$path}->{'tileset'}; # Skip already marked map
        $label++ while defined $tilesets->{$label};  # Find next free label
        $tilesets->{$label} = label_tileset($maps->{$path}, $maps, $label, 0, 0);
    }
}

# Recurse over a tileset graph and label each tile with coordinates and
# the tileset label
sub label_tileset
{
    my ($map, $maps, $label, $x, $y) = @_;
    
    $map->{'tileset'} = $label;
    $map->{'x'} = $x;
    $map->{'y'} = $y;
    
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
            push @set, @{label_tileset($maps->{$target}, $maps, $label, $x2, $y2)};
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

    die "Aborting due to map linking errors\n" if $errors;
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
                        'filename' => substr($path, rindex($path, "/"))
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
                } elsif ($line =~ /^tileset_id (\d+)$/) {
                    $map->{'old_tileset'} = $1;
                } elsif ($line =~ /^tileset_x (-?\d+)$/) {
                    $map->{'old_x'} = $1;
                } elsif ($line =~ /^tileset_y (-?\d+)$/) {
                    $map->{'old_y'} = $1;
                } elsif ($line eq 'end') {
                    last;
                }
            }
            $hash->{$map->{'path'}} = $map if defined $map;
            
            close FILE;
        }
    }
}
