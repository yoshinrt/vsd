#!/usr/bin/perl -w

$bTomo = 0;

sub ConvCfg {
	local( $CfgFile ) = @_;
	
	$bMark = 0;
	$Marks = '';
	
	$Param{ TRACK_VSt }			= 0;
	$Param{ TRACK_VSt2 }		= 0;
	$Param{ TRACK_VEd }			= 0;
	$Param{ TRACK_VEd2 }		= 0;
	$Param{ TRACK_LSt }			= 0;
	$Param{ TRACK_LSt2 }		= 0;
	$Param{ TRACK_LEd }			= 0;
	$Param{ TRACK_LEd2 }		= 0;
	$Param{ TRACK_LineTrace }	= 0;
	$Param{ TRACK_MapSize }		= 400;
	$Param{ TRACK_MapAngle }	= 0;
	$Param{ TRACK_LSt }			= 0;
	$Param{ TRACK_LSt2 }		= 0;
	$Param{ TRACK_LEd }			= 0;
	$Param{ TRACK_LEd2 }		= 0;
	$Param{ TRACK_GEAR1 }		= 1315;
	$Param{ TRACK_GEAR2 }		= 2030;
	$Param{ TRACK_GEAR3 }		= 2810;
	$Param{ TRACK_GEAR4 }		= 3569;
	$Param{ TRACK_GEAR5 }		= 4544;
	$Param{ TRACK_GEAR6 }		= 5741;
	$Param{ TRACK_PULSE_TACHO }	= 2000;
	$Param{ TRACK_PULSE_SPEED }	= 2548;
	$Param{ TRACK_TACHO }		= 8;
	$Param{ TRACK_SPEED }		= 180;
	$Param{ CHECK_LAP }			= 1;
	$Param{ CHECK_SNAKE }		= 1;
	
	$VidFile = $LogFile = $AvsFile = $CfgFile;
	$LogFile =~ s|.*/||;
	$VidFile =~ s|.*/||;
	
	$AvsFile =~ s/\.cfg$/.avs/;
	$LogFile =~ s/\.cfg$/.log/;
	$VidFile =~ s/\.cfg$/.MP4/;
	
	open( fpIn,  "< $CfgFile" );
	open( fpOut, "> $AvsFile" );
	
	while( <fpIn> ){
		s/[\x0D\x0A]//g;
		
		if( $Marks || $_ eq 'MARK:' ){
			$bMark = 1;
			$Marks .= "$_\n";
		}else{
			$Param{ $1 } = $2 if( /(.+)=(.+)/ );
		}
	}
	
	$Param{ TRACK_VSt } = $Param{ TRACK_VSt } * 100 + $Param{ TRACK_VSt2 };
	$Param{ TRACK_VEd } = $Param{ TRACK_VEd } * 100 + $Param{ TRACK_VEd2 };
	
	if( $bTomo ){
		$Param{ TRACK_LSt } = $Param{ TRACK_LSt } * 100 + int( $Param{ TRACK_LSt2 } / 10 );
		$Param{ TRACK_LEd } = $Param{ TRACK_LEd } * 100 + int( $Param{ TRACK_LEd2 } / 10 );
	}else{
		$Param{ TRACK_LSt } = $Param{ TRACK_LSt } * 100 + $Param{ TRACK_LSt2 };
		$Param{ TRACK_LEd } = $Param{ TRACK_LEd } * 100 + $Param{ TRACK_LEd2 };
	}
	
	if( $Marks ){
		$LogFile = "param_file=\"$AvsFile\"";
	}else{
		$LogFile = "log_file=\"$LogFile\"";
	}
	
	print fpOut<<EOF;
DirectShowSource("$VidFile")
ConvertToYUY2()
VSDFilter( \\
	$LogFile, \\
	video_start=$Param{TRACK_VSt}, \\
	video_end=$Param{TRACK_VEd}, \\
	log_start=$Param{TRACK_LSt}, \\
	log_end=$Param{TRACK_LEd}, \\
EOF
	
	if( !$bTomo ){
		print fpOut<<EOF;
	map_length=$Param{TRACK_LineTrace}, \\
	map_size=$Param{TRACK_MapSize}, \\
	map_angle=$Param{TRACK_MapAngle}, \\
	lap_time=$Param{CHECK_LAP}, \\
	g_snake=$Param{CHECK_SNAKE} \\
)
EOF
	}else{
		print fpOut<<EOF;
	gear1=$Param{TRACK_GEAR1}, \\
	gear2=$Param{TRACK_GEAR2}, \\
	gear3=$Param{TRACK_GEAR3}, \\
	gear4=$Param{TRACK_GEAR4}, \\
	gear5=$Param{TRACK_GEAR5}, \\
	gear6=$Param{TRACK_GEAR6}, \\
	tacho_pulse=$Param{TRACK_PULSE_TACHO}, \\
	speed_pulse=$Param{TRACK_PULSE_SPEED}, \\
	tacho_meter=$Param{TRACK_TACHO}, \\
	speed_meter=$Param{TRACK_SPEED}, \\
	lap_time=$Param{CHECK_LAP} \\
)
EOF
	}
	
	print fpOut "__END__\n$Marks" if( $Marks );
	
	close( fpIn );
	close( fpOut );
}

foreach ( @ARGV ){
	ConvCfg( $_ );
}
