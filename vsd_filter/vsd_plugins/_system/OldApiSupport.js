"use strict";

SetupOldApi(){
	if( TargetVSDRevision === undefined ){
		Print(
			"このスキンが動作する VSD のリビジョン番号を\n" +
			"TargetVSDRevision に設定する必要があります．\n"
		);
		return;
	}
	
	// APIKey の設定を _user_config.js に分離
	if( TargetVSDRevision >= 779 ) return;
	if( GoogleAPIKey[ 0 ] == '' ){
		Vsd.__DrawGoogleMaps_r779 = Vsd.DrawGoogleMaps;
		Vsd.DrawGoogleMaps = funcion( param ){
			GoogleAPIKey[ 0 ] = param.APIKey;
			
			Vsd.DrawGoogleMaps = Vsd.__DrawGoogleMaps_r779;
			delete Vsd.__DrawGoogleMaps_r779;
		}
	}
	
	// Vsd.Speed -> Log.Speed, Vsd.MaxSpeed -> Log.Max.Speed に変更
	if( TargetVSDRevision >= 769 ) return;
	for( var prop in Log ){
		Vsd.[ "Max" + prop ] = Log.Max[ prop ];
		Vsd.[ "Min" + prop ] = Log.Min[ prop ];
		Vsd.[ prop ] = get function(){ return Log[ prop ]; }
	}
	
	// DrawOpenStreetMap → Vsd.DrawRoadMap に変更
	if( TargetVSDRevision >= 743 ) return;
	Vsd.DrawOpenStreetMap = Vsd.DrawRoadMap;
	
	// DrawMeterScale 廃止
	if( TargetVSDRevision >= 723 ) return;
	Vsd.DrawMeterScale = function(
		x, y, r,
		line1_len, line1_width, line1_color,
		line2_len, line2_width, line2_color, line2_cnt,
		min_angle, max_angle, r_num, max_val, line1_cnt,
		color, font
	){
		return Vsd.DrawRoundMeterScaleSub(
			x, y, r,
			line1_len, line1_width, line1_color, line1_cnt,
			line2_len, line2_width, line2_color, line2_cnt,
			min_angle, max_angle, 0, max_val,
			r_num, color, font
		);
	}
}

SetupOldApi();
SetupOldApi = undefined;
