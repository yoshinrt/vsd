#!/usr/bin/perl -w

foreach $file ( @ARGV ){
	open( fpIn, "< $file" );
	open( fpOut, "| nkf -w > $file.kml" );
	
	$name = $file;
	$name =~ s/\..*//;
	
	print fpOut << "EOF";
<?xml version='1.0' encoding='UTF-8'?>
<kml xmlns='http://www.opengis.net/kml/2.2'>
	<Document>
		<name>$name</name>
		<description><![CDATA[]]></description>
		<Folder>
			<name>$name</name>
			<Placemark>
				<styleUrl>#line-1267FF-5</styleUrl>
				<name>$name</name>
				<ExtendedData>
				</ExtendedData>
				<LineString>
					<tessellate>1</tessellate>
					<coordinates>
EOF
	
	while( <fpIn> ){
		if( /\$GPGGA/ ){
			@_ = split( /,/, $_ );
			next if( $_[ 2 ] eq '' );
			
			# 0      1      2         3 4          5 6 7  89
			# $GPGGA,074643,3502.2250,N,13400.4518,E,1,04,,12.3,M,,M,,*47
			
			$_[ 4 ] =~ /(\d+)(\d\d\.\d+)/; $Lng = $1 + ( $2 / 60 );
			$_[ 2 ] =~ /(\d+)(\d\d\.\d+)/; $Lat = $1 + ( $2 / 60 );
			print( fpOut "$Lng,$Lat,$_[ 9 ]\n" );
		}
	}
	
	print fpOut << "EOF";
					</coordinates>
				</LineString>
			</Placemark>
		</Folder>
		<Style id='icon-503-DB4436'>
			<IconStyle>
				<color>ff3644DB</color>
				<scale>1.1</scale>
				<Icon>
					<href>http://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
				</Icon>
			</IconStyle>
		</Style>
		<Style id='line-1267FF-5'>
			<LineStyle>
				<color>ffFF6712</color>
				<width>5</width>
			</LineStyle>
		</Style>
	</Document>
</kml>
EOF
}
