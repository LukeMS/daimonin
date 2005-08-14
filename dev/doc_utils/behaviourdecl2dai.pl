#!/usr/bin/perl -w

# Generates xml .dai files from the behaviour declaration file
# server/src/include/behaviourdel.h
#
# Usage: 
#   dev/doc_utils/behaviourdecl2dai.pl server/src/include/behaviourdecl.h doc/src/doc/

use strict;
use Parse::RecDescent;
use XML::Generator ();

# Enable warnings within the Parse::RecDescent module.

$::RD_ERRORS = 1; # Make sure the parser dies when it encounters an error
$::RD_WARN   = 1; # Enable warnings. This will warn on unused rules &c.
$::RD_HINT   = 1; # Give out hints to help fix problems.

# Cleans up a doxygen-like comment block to raw text
sub doc
{
    my ($raw) = @_;

    # Remove comment start and end markers
    $raw =~ s/^\s*\/\*\*\s*//; 
    $raw =~ s/\*\///;

    $raw =~ s/^\s*\*\s*//gm; # Remove whitespace and * at beginnings of lines
    $raw =~ s/\s*$//gm; # Remove whitespace at line ends

    return $raw
}

sub class
{
    my ($name, $behaviours, $doc) = @_;

    if(defined $doc) { $doc = $::xml->p(@{$doc}) } 
        else { $doc = ""; }

    return {'name' => $name, 'xml' => $::xml->section($::xml->title(ucfirst(lc($name))), $doc, @{$behaviours})};
}

sub behaviour
{
    my ($name, $parameters, $doc) = @_;

    $name = lc $name;

    if(defined $doc) { $doc = $::xml->p(@{$doc}) } 
        else { $doc = ""; }

    my $signature = $name;
    my @params = ();        
    if($parameters)
    {
        foreach my $param (@{$parameters}) {
            my ($type) = lc $param->{'type'};          
            $type = "string:integer" if $type eq 'stringint';
            my $p = lc($param->{name})."=$type";
            $p = "{$p}" if grep /MULTI/, @{$param->{'flags'}};
            $p = "[$p]" if grep /OPTIONAL/, @{$param->{'flags'}};
            $signature .= " $p";

            push @params, 
                $::xml->dt($::xml->code(lc $param->{'name'}), 
                        " (",
                        lc(join(", ", $param->{'type'}, @{$param->{'flags'}})), 
                        ")"),
                $::xml->dd(@{$param->{'doc'}});
        }
        @params = $::xml->dl(@params);
    } else {
        @params = $::xml->p("(No parameters)");
    }

    return $::xml->section(
        $::xml->title(ucfirst($name)), 
        $::xml->p("Usage: ", $::xml->code($signature)),
        $doc,
        $::xml->section($::xml->title("Parameters"), @params));
#        $::xml->p($::xml->strong("Parameters:")), @params);
}

#sub parameter
#{
#    my ($name, $type, $flags, $default, $doc) = @_;
#
#    if(defined $doc) { $doc = $::xml->p(@{$doc}) } 
#        else { $doc = ""; }
#
 #   return 
 #       $::xml->section(
 #           $::xml->title("Parameter $name"),
 #           $::xml->dl(
 #               $::xml->dt('Type:'), $::xml->dd("$type"),
 #               $::xml->dt('Flags:'), $::xml->dd(join(", ", @{$flags})),
 #               $::xml->dt('Default:'), $::xml->dd($default)),
 #           $doc
#            );
#}

my $grammar = <<'_EOGRAMMAR_';

start: ccomment(s?) classlist

ccomment: m{/\*[^*].*?\*/}s
bdlcomment: m{/\*\*.*?\*/}s
    { $return = ::doc($item[1]); }

classlist: class(s?)
    { $return = $item[1]; }
class: bdlcomment(?) 'BehaviourClass(' classname ',' behaviourlist ')'
    { $return = ::class($item[3], $item[5], $item[1]); }
classname: /[A-Z_]+/

behaviourlist: behaviour(s) 
    | 'NIL'
    { $return = [$::xml->p("(No behaviours)")]; }
behaviour: bdlcomment(?) 'Behaviour(' behaviourname ',' behaviourfunc ',' parameterlist ')'
    { $return = ::behaviour($item[3], $item[-2], $item[1]) }
behaviourname: /[A-Z_]+/
behaviourfunc: /[a-z_]+/ 

parameterlist: parameter(s) 
    | 'NIL'
    { $return = 0; 1;}
#    { $return = [$::xml->p("(No parameters)")]; }
parameter: bdlcomment(?) 'Parameter(' behaviourname ',' parametername ',' parametertype ',' parameterflags ',' defaultvalue ')'
    { $return = {'name' => $item[5], 'type' => $item[7], 'flags' => $item[9], 'default' => $item[11], 'doc' => $item[1]}; }
parametername: /[A-Z_]+/
parametertype: 'INTEGER' | 'STRINGINT' | 'STRING' 
parameterflags: parameterflag(s /\|/)
parameterflag: 'MULTI' | 'SINGLE' | 'OPTIONAL' | 'MANDATORY'
defaultvalue: /"[^"]*"/ | 'NULL' | /-?[0-9]+/

_EOGRAMMAR_

$::xml = XML::Generator->new(':pretty');

my $parser = Parse::RecDescent->new($grammar);

die "Usage: $0 path-to-behaviourdecl.h destination-path\n" unless scalar @ARGV == 2;

open FILE, "<$ARGV[0]" or die "Couldn't open $ARGV[0]: $!\n";
my $file = do { local $/; <FILE> };

foreach my $class (@{$parser->start($file)})
{
    my $filename = "$ARGV[1]/". lc("behaviourclass_$class->{name}.dai");
    open OUTFILE, ">$filename" or die "Couldn't create $filename: $!\n";


    print OUTFILE '<?xml version="1.0" encoding="iso-8859-1"?>', "\n";
    print OUTFILE '<!DOCTYPE section SYSTEM "../dtd/daidoc.dtd">', "\n";
    print OUTFILE $class->{xml},"\n";
    
    print "Generated $filename\n";
}
