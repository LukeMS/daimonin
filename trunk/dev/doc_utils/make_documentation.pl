#! /usr/bin/perl -w

# Comments we search for to know in which general area to search for
# python functions in.
$functionstart = "FUNCTIONSTART";
$functionend = "FUNCTIONEND";

# 1 = required, 2 = optional
# keywords in comments that are not in this list will be thought
# as freetext belonging to the previous section.
%keywords = (
    "Name" => 1,
    "Python" => 1,
    "Info" => 2,
    "Status" => 1,
    "Warning" => 2,
    "Remark" => 2,
    "TODO" => 2
);

# States for main loop parser
$ST_INIT = 1;           # Searching for the relevant functions
$ST_NEWCOMMENT = 2;     # Looking for comment box
$ST_COMMENT_START = 3;  # Found the beginning of a comment box
$ST_COMMENT_END = 4;    # Found the end of a comment box
$ST_BODY_START = 5;     # Found the beginning of a function body
$ST_PARSETUPLE = 6;     # Found the beginning of Py_ParseTuple() call

$state = $ST_INIT;      # Initial state 
$line = 0;              # Initial line number

while($_ = <>) {
    $line ++;
    if($_ =~ /^\/\*.*$functionend.*\*\/$/) { last; }

    if($state == $ST_INIT) {
        # Look for magic keyword
        if($_ =~ /^\/\*.*$functionstart.*\*\/$/) {
            $state = $ST_NEWCOMMENT;
            $_ = <>; # Skip one line...            
        }
    } elsif ($state == $ST_NEWCOMMENT) {
        # Look for comment box
        if($_ =~ /^\/\*\*+\*\/$/) {
            $c_current_keyword = "";
            %c_keywords = ();
            @c_param_names = ();
            
            @b_input_types = ();
            %b_return_types = ();
            $b_name = "";
            
            $state = $ST_COMMENT_START;
        }
    } elsif ($state == $ST_COMMENT_START) {
        if($_ =~ /^\/\*(.*)\*\/$/) {
            # Not a box line? If so, then extract comments
            if(not $1 =~ /^\*+$/) {
                my $text = $1;
                $text =~ s/(^\s*)|(\s*$)//g;   # Trim blanks at both ends
                $text =~ /(\w+)\s*:\s*(.*)$/;  # Fetch keyword and text
                
                # Comment line starts with an accepted keyword?
                if (defined $1 and defined $keywords{$1}) {
                    print STDERR "WARNING: double keyword $1 @ $line\n" if defined $c_keywords{$1};
                    $c_keywords{$1} = $2;
                    $c_current_keyword = $1;
                } else {
                    $c_keywords{$c_current_keyword} .= "\n$text";
                }
            }
        } else {
            $state = $ST_COMMENT_END;
        }
    } 
    
    if ($state == $ST_COMMENT_END) {
        # Find function name and start of function body
        
        if($_ =~ /^static PyObject ?\* ?(.*)?\(/) {
            $b_name = $1;
        } 
        if ($_ =~ /{/) {
            $state = $ST_BODY_START;
        }
    } elsif ($state == $ST_BODY_START) {
        # In the body we look for return and parameter types
        
        # Look for return types
        if ($_ =~ /^\s+return/) {
            if ($_ =~ /return Py_None/) {
                $b_return_types{'None'} = 1;
            } elsif ($_ =~ /return Py_BuildValue\("(.*)"/) {
                my $type = $1;
                if($type =~ /[ild]/) {
                    $b_return_types{'integer'} = 1;
                } elsif($type =~ /[s]/) {
                    $b_return_types{'string'} = 1;
                } else {
                    print STDERR "WARNING: unknown return: $type\n";
                }
            } elsif ($_ =~ /return wrap_(\w+)\(/) {
                $b_return_types{$1} = 1;
            } elsif ($_ =~ /return NULL;/) {
            } else {
                print STDERR "WARNING: unknown return: $_\n";
            }
        }
        
        # Look for parameter types
        my ($typestring, $varstring);
        if ($_ =~ /PyArg_ParseTuple\(/) {
            $state = $ST_PARSETUPLE;
            $b_parsetuple = "";
        }
            
        #End of function ?
        if ($_ =~ /^}/) {
            # Work with fetched info:
            my $pname = "";
                
            # Extract parameter names from calling conventions
            my $python = $c_keywords{'Python'};
            if(! defined $python || $python eq '') {
                print STDERR "WARNING: No 'Python' info for function $b_name @ $line\n";
                # Use the name from the c body as the pyhton name and hope it works
                $pname = $b_name;
                $pname =~ s/^.*_//;
                # Later: Build fake parameter list
            } else {
                my $vars;
                ($pname, $vars) = ($python =~ /^.*?\.(.*?)\(\s*(.*?)\s*\)$/g);

                if(defined $pname and defined $vars) {
                    $vars =~ s/\s//g;
                    @c_param_names = split(",", $vars);

                    if ($#c_param_names != $#b_input_types) {
                        print STDERR "WARNING: Not same number of parameters in 'Python' info as in function $b_name body @ $line\n";
                    }
                } else {
                    print STDERR "WARNING: bad 'Python' info for function $b_name @ $line\n";
                    $pname = '';
                }
            }
            
            # Check names
            if (!defined $c_keywords{'Name'} or $b_name ne $c_keywords{'Name'}) {
                print STDERR "WARNING: 'Name' info not same as name for function $b_name @ $line\n";
            }
            $b_name =~ /^(.*)_(.*)$/;
            if ("$2" ne $pname) {
                print STDERR "WARNING: 'Python' function name not something.$2 @ $line\n";
            }

            # Remove redundant CFPython. prefix from function name if present
            $pname =~ s/^.*?\.//;
            
            # check required keywords
            foreach $key (keys %keywords) {
                if($keywords{$key} == 1 && (! defined $c_keywords{$key} or $c_keywords{$key} eq '')) {
                    print STDERR "WARNING: Required keyword '$key' missing for $b_name @ $line\n";
                }
            }

            #
            # Actual output comes here:
            #
            
            outputplaintext($pname);
            
            $state = $ST_NEWCOMMENT;
        }
        
    }
    
    if($state == $ST_PARSETUPLE) {
        $b_parsetuple .= $_;
        if ($_ =~ /\)\s*\)/) {
            $state = $ST_BODY_START;
        
            if (($typestring, $varstring) = 
                    ($b_parsetuple =~ /PyArg_ParseTuple\(.*?"(.*?)",\s*([^)]*?)\)/)) {
#            print "  Input types: $typestring\n";
#            print "  Input vars: $varstring\n";

                $varstring =~ s/\s//g;
                my @vars = split ',', $varstring;
                my @types = ($typestring =~ /O!?|[ldis|]/g);

                my $varpos = 0;

                my $optional = "";

                foreach $type (@types) {
                    if ($type eq 'O!') {
                        if ($vars[$varpos] eq '&Daimonin_MapType') {
                            push @b_input_types, $optional.'map';
                        } elsif ($vars[$varpos] eq '&Daimonin_ObjectType') {
                            push @b_input_types, $optional.'object';
                        } else {
                            print STDERR "WARNING: unknown python input object: ", $vars[$varpos], "\n";                        
                        }
                        $varpos ++;
                    }

                    $optional = "(optional) " if ($type eq '|');
                    push @b_input_types, $optional."speciall" if ($type eq 'O'); 
                    push @b_input_types, $optional."integer" if ($type =~ /[ldi]/);
                    push @b_input_types, $optional."string" if ($type eq 's'); 
                    $varpos++ if ($type =~ /[ldisO]/);
                }

                # Have to create fake variable names?
                if(! defined $c_keywords{'Python'} || $c_keywords{'Python'} eq '') {
                    for (my $i=0; $i <= $#types; $i++) {
                        push @c_param_names, "param$i";
                    }                    
                    $c_keywords{'Python'} = "$b_name (" . join(", ", @c_param_names) . ")";
                    $c_keywords{'Python'} =~ s/^CF//;
                }
            }
        }
    }
}

sub outputplaintext {
    my ($python_name) = @_;
    
    print "$python_name\n";
    print "-" x 60 , "\n";
    print "  $c_keywords{Python}\n";
    if(defined $c_keywords{'Info'}) {
        $c_keywords{'Info'} =~ s/\n/\n    /g;
        print "    $c_keywords{Info}\n";
    }

    print "  Parameter types:\n";
    for (my $i=0; $i <= $#c_param_names; $i++) {
        print "    $b_input_types[$i] $c_param_names[$i]\n";
    }
    
    print "  Possible return types:\n";
    foreach my $rettype (keys %b_return_types) {
        print "    $rettype\n";
    }

    #Order and definition of which keywords to print next
    my @print_keywords = ( "Warning", "Remark", "Status", "TODO" );

    # Info from comments            
    foreach $key (@print_keywords) {
        if(defined $c_keywords{$key} and $c_keywords{$key} ne '') {
            print "  $key:\n";
            $c_keywords{$key} =~ s/\n/\n    /g;
            print "    $c_keywords{$key}\n";
        } 
    }

    print "\n";
}
